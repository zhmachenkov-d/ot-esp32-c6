#include "mqtt_ha.h"

#include "app_config.h"
#include "nvs_store.h"

#include "esp_log.h"
#include "mqtt_client.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG = "mqtt_ha";

static esp_mqtt_client_handle_t s_client;
static char s_device_id[16];
static char s_status_topic[64];
static char s_ca_pem[NVS_MQTT_CA_PEM_MAX];
static bool s_connected;
static bool s_force_offline_birth;
static mqtt_ha_message_cb_t s_msg_cb;
static void *s_msg_ctx;
static mqtt_ha_connected_cb_t s_connected_cb;
static void *s_connected_ctx;

void mqtt_ha_status_topic(char *buf, size_t cap, const char *device_id)
{
    snprintf(buf, cap, "%s%s/status", APP_MQTT_TOPIC_ROOT, device_id);
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    (void)handler_args;
    (void)base;
    esp_mqtt_event_handle_t event = event_data;
    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        s_connected = true;
        ESP_LOGI(TAG, "connected");
        if (s_force_offline_birth) {
            mqtt_ha_publish_offline();
        } else {
            mqtt_ha_publish_birth_online();
        }
        if (s_connected_cb) {
            s_connected_cb(s_connected_ctx);
        }
        break;
    case MQTT_EVENT_DISCONNECTED:
        s_connected = false;
        ESP_LOGW(TAG, "disconnected");
        break;
    case MQTT_EVENT_DATA:
        if (s_msg_cb && event->topic && event->data) {
            char topic[128];
            int tlen = event->topic_len < (int)sizeof(topic) - 1 ? event->topic_len : (int)sizeof(topic) - 1;
            memcpy(topic, event->topic, tlen);
            topic[tlen] = '\0';
            char *payload = malloc((size_t)event->data_len + 1);
            if (payload) {
                memcpy(payload, event->data, event->data_len);
                payload[event->data_len] = '\0';
                s_msg_cb(topic, payload, event->data_len, event->retain != 0, s_msg_ctx);
                free(payload);
            }
        }
        break;
    default:
        break;
    }
}

esp_err_t mqtt_ha_init(const char *device_id,
                       const char *host, uint16_t port,
                       const char *username, const char *password,
                       bool tls, const char *ca_pem)
{
    if (!device_id || !host || !host[0]) {
        return ESP_ERR_INVALID_ARG;
    }
    if (tls && (!ca_pem || !ca_pem[0])) {
        return ESP_ERR_INVALID_ARG;
    }
    strncpy(s_device_id, device_id, sizeof(s_device_id) - 1);
    mqtt_ha_status_topic(s_status_topic, sizeof(s_status_topic), device_id);

    char uri[192];
    snprintf(uri, sizeof(uri), "%s://%s:%u", tls ? "mqtts" : "mqtt", host, (unsigned)port);

    esp_mqtt_client_config_t cfg = {
        .broker.address.uri = uri,
        .credentials.username = (username && username[0]) ? username : NULL,
        .credentials.authentication.password = (password && password[0]) ? password : NULL,
        .session.last_will = {
            .topic = s_status_topic,
            .msg = "offline",
            .msg_len = 7,
            .qos = 1,
            .retain = true,
        },
    };
    if (tls) {
        size_t pem_len = strlen(ca_pem);
        if (pem_len + 1 > sizeof(s_ca_pem)) {
            return ESP_ERR_INVALID_ARG;
        }
        memcpy(s_ca_pem, ca_pem, pem_len + 1);
        /* Pin broker CA/server PEM; skip CN so LAN IPs work with self-signed certs */
        cfg.broker.verification.certificate = s_ca_pem;
        cfg.broker.verification.certificate_len = pem_len + 1;
        cfg.broker.verification.skip_cert_common_name_check = true;
    }

    s_client = esp_mqtt_client_init(&cfg);
    if (!s_client) {
        return ESP_ERR_NO_MEM;
    }
    esp_mqtt_client_register_event(s_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    ESP_LOGI(TAG, "init uri=%s lwt=%s tls=%d", uri, s_status_topic, (int)tls);
    return ESP_OK;
}

esp_err_t mqtt_ha_start(void)
{
    if (!s_client) {
        return ESP_ERR_INVALID_STATE;
    }
    return esp_mqtt_client_start(s_client);
}

void mqtt_ha_stop(void)
{
    if (s_client) {
        esp_mqtt_client_stop(s_client);
        esp_mqtt_client_destroy(s_client);
        s_client = NULL;
    }
    s_connected = false;
}

bool mqtt_ha_connected(void)
{
    return s_connected;
}

esp_err_t mqtt_ha_publish_birth_online(void)
{
    s_force_offline_birth = false;
    return mqtt_ha_publish(s_status_topic, "online", 1, true);
}

esp_err_t mqtt_ha_publish_offline(void)
{
    s_force_offline_birth = true;
    return mqtt_ha_publish(s_status_topic, "offline", 1, true);
}

esp_err_t mqtt_ha_publish(const char *topic, const char *payload, int qos, bool retain)
{
    if (!s_client || !topic) {
        return ESP_ERR_INVALID_STATE;
    }
    int msg_id = esp_mqtt_client_publish(s_client, topic, payload ? payload : "",
                                         payload ? (int)strlen(payload) : 0, qos, retain);
    return msg_id >= 0 ? ESP_OK : ESP_FAIL;
}

void mqtt_ha_set_message_callback(mqtt_ha_message_cb_t cb, void *ctx)
{
    s_msg_cb = cb;
    s_msg_ctx = ctx;
}

void mqtt_ha_set_connected_callback(mqtt_ha_connected_cb_t cb, void *ctx)
{
    s_connected_cb = cb;
    s_connected_ctx = ctx;
}

esp_err_t mqtt_ha_subscribe(const char *topic, int qos)
{
    if (!s_client || !topic) {
        return ESP_ERR_INVALID_STATE;
    }
    int id = esp_mqtt_client_subscribe(s_client, topic, qos);
    return id >= 0 ? ESP_OK : ESP_FAIL;
}

esp_err_t mqtt_ha_publish_boiler_link(bool healthy)
{
    char topic[80];
    snprintf(topic, sizeof(topic), "%s%s/boiler_link", APP_MQTT_TOPIC_ROOT, s_device_id);
    return mqtt_ha_publish(topic, healthy ? "healthy" : "unhealthy", 1, true);
}

void mqtt_ha_update_state_topic(char *buf, size_t cap, const char *device_id)
{
    snprintf(buf, cap, "%s%s/update/state", APP_MQTT_TOPIC_ROOT, device_id);
}

void mqtt_ha_update_command_topic(char *buf, size_t cap, const char *device_id)
{
    snprintf(buf, cap, "%s%s/update/set", APP_MQTT_TOPIC_ROOT, device_id);
}

esp_err_t mqtt_ha_publish_update_state(const char *device_id, const char *json)
{
    char topic[96];
    mqtt_ha_update_state_topic(topic, sizeof(topic), device_id);
    return mqtt_ha_publish(topic, json ? json : "", 1, true);
}
