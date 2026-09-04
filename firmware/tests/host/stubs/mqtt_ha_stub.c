#include "mqtt_ha.h"

#include <stdio.h>
#include <string.h>

#define HOST_MQTT_HA_MAX_PUBLISHES 64
#define HOST_MQTT_HA_TOPIC_CAP 160
#define HOST_MQTT_HA_PAYLOAD_CAP 1024
#define HOST_MQTT_HA_MAX_SUBS 32

static char s_topics[HOST_MQTT_HA_MAX_PUBLISHES][HOST_MQTT_HA_TOPIC_CAP];
static char s_payloads[HOST_MQTT_HA_MAX_PUBLISHES][HOST_MQTT_HA_PAYLOAD_CAP];
static int s_publish_count;

static char s_subs[HOST_MQTT_HA_MAX_SUBS][HOST_MQTT_HA_TOPIC_CAP];
static int s_sub_count;

static mqtt_ha_message_cb_t s_msg_cb;
static void *s_msg_ctx;

void host_mqtt_ha_reset_publishes(void)
{
    s_publish_count = 0;
    memset(s_topics, 0, sizeof(s_topics));
    memset(s_payloads, 0, sizeof(s_payloads));
}

void host_mqtt_ha_reset_subscribes(void)
{
    s_sub_count = 0;
    memset(s_subs, 0, sizeof(s_subs));
}

int host_mqtt_ha_publish_count(void)
{
    return s_publish_count;
}

int host_mqtt_ha_count_topic_substr(const char *substr)
{
    int n = 0;
    if (!substr) {
        return 0;
    }
    for (int i = 0; i < s_publish_count; i++) {
        if (strstr(s_topics[i], substr) != NULL) {
            n++;
        }
    }
    return n;
}

int host_mqtt_ha_count_payload_substr(const char *substr)
{
    int n = 0;
    if (!substr) {
        return 0;
    }
    for (int i = 0; i < s_publish_count; i++) {
        if (strstr(s_payloads[i], substr) != NULL) {
            n++;
        }
    }
    return n;
}

int host_mqtt_ha_count_subscribe_substr(const char *substr)
{
    int n = 0;
    if (!substr) {
        return 0;
    }
    for (int i = 0; i < s_sub_count; i++) {
        if (strstr(s_subs[i], substr) != NULL) {
            n++;
        }
    }
    return n;
}

void host_mqtt_ha_inject_message(const char *topic, const char *payload, bool retain)
{
    if (s_msg_cb) {
        int len = payload ? (int)strlen(payload) : 0;
        s_msg_cb(topic, payload ? payload : "", len, retain, s_msg_ctx);
    }
}

esp_err_t mqtt_ha_init(const char *device_id, const char *host, uint16_t port,
                       const char *username, const char *password, bool tls,
                       const char *ca_pem)
{
    (void)device_id;
    (void)host;
    (void)port;
    (void)username;
    (void)password;
    (void)tls;
    (void)ca_pem;
    return ESP_OK;
}
esp_err_t mqtt_ha_start(void) { return ESP_OK; }
void mqtt_ha_stop(void) {}
bool mqtt_ha_connected(void) { return true; }
esp_err_t mqtt_ha_publish_birth_online(void) { return ESP_OK; }
esp_err_t mqtt_ha_publish_offline(void) { return ESP_OK; }
esp_err_t mqtt_ha_publish(const char *topic, const char *payload, int qos, bool retain)
{
    (void)qos;
    (void)retain;
    if (topic && s_publish_count < HOST_MQTT_HA_MAX_PUBLISHES) {
        snprintf(s_topics[s_publish_count], HOST_MQTT_HA_TOPIC_CAP, "%s", topic);
        snprintf(s_payloads[s_publish_count], HOST_MQTT_HA_PAYLOAD_CAP, "%s",
                 payload ? payload : "");
        s_publish_count++;
    }
    return ESP_OK;
}
void mqtt_ha_set_message_callback(mqtt_ha_message_cb_t cb, void *ctx)
{
    s_msg_cb = cb;
    s_msg_ctx = ctx;
}
void mqtt_ha_set_connected_callback(mqtt_ha_connected_cb_t cb, void *ctx)
{
    (void)cb;
    (void)ctx;
}
esp_err_t mqtt_ha_subscribe(const char *topic, int qos)
{
    (void)qos;
    if (topic && s_sub_count < HOST_MQTT_HA_MAX_SUBS) {
        snprintf(s_subs[s_sub_count], HOST_MQTT_HA_TOPIC_CAP, "%s", topic);
        s_sub_count++;
    }
    return ESP_OK;
}
esp_err_t mqtt_ha_publish_boiler_link(bool healthy)
{
    (void)healthy;
    return ESP_OK;
}
void mqtt_ha_status_topic(char *buf, size_t cap, const char *device_id)
{
    snprintf(buf, cap, "otc6/%s/status", device_id);
}

void mqtt_ha_update_state_topic(char *buf, size_t cap, const char *device_id)
{
    snprintf(buf, cap, "otc6/%s/update/state", device_id);
}

void mqtt_ha_update_command_topic(char *buf, size_t cap, const char *device_id)
{
    snprintf(buf, cap, "otc6/%s/update/set", device_id);
}

esp_err_t mqtt_ha_publish_update_state(const char *device_id, const char *json)
{
    char topic[96];
    mqtt_ha_update_state_topic(topic, sizeof(topic), device_id);
    return mqtt_ha_publish(topic, json, 1, true);
}
