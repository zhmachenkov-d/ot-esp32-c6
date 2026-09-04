#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*mqtt_ha_message_cb_t)(const char *topic, const char *payload, int len,
                                     bool retain, void *ctx);

/** Invoked on MQTT_EVENT_CONNECTED (after birth/offline publish). */
typedef void (*mqtt_ha_connected_cb_t)(void *ctx);

esp_err_t mqtt_ha_init(const char *device_id,
                       const char *host, uint16_t port,
                       const char *username, const char *password,
                       bool tls, const char *ca_pem);

esp_err_t mqtt_ha_start(void);
void mqtt_ha_stop(void);

bool mqtt_ha_connected(void);

/** Birth / LWT helpers (T011 ownership). */
esp_err_t mqtt_ha_publish_birth_online(void);
esp_err_t mqtt_ha_publish_offline(void);

esp_err_t mqtt_ha_publish(const char *topic, const char *payload, int qos, bool retain);

void mqtt_ha_set_message_callback(mqtt_ha_message_cb_t cb, void *ctx);

void mqtt_ha_set_connected_callback(mqtt_ha_connected_cb_t cb, void *ctx);

esp_err_t mqtt_ha_subscribe(const char *topic, int qos);

/** Publish boiler_link state: "healthy" / "unhealthy". */
esp_err_t mqtt_ha_publish_boiler_link(bool healthy);

/** Build status topic into buf: otc6/<device_id>/status */
void mqtt_ha_status_topic(char *buf, size_t cap, const char *device_id);

/** Build HA update state/command topics. */
void mqtt_ha_update_state_topic(char *buf, size_t cap, const char *device_id);
void mqtt_ha_update_command_topic(char *buf, size_t cap, const char *device_id);

/** Publish retained update entity JSON on state_topic. */
esp_err_t mqtt_ha_publish_update_state(const char *device_id, const char *json);

#ifdef __cplusplus
}
#endif
