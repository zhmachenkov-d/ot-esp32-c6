#include "mqtt_ha.h"

#include <stdio.h>
#include <string.h>

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
    (void)topic;
    (void)payload;
    (void)qos;
    (void)retain;
    return ESP_OK;
}
void mqtt_ha_set_message_callback(mqtt_ha_message_cb_t cb, void *ctx)
{
    (void)cb;
    (void)ctx;
}
void mqtt_ha_set_connected_callback(mqtt_ha_connected_cb_t cb, void *ctx)
{
    (void)cb;
    (void)ctx;
}
esp_err_t mqtt_ha_subscribe(const char *topic, int qos)
{
    (void)topic;
    (void)qos;
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
