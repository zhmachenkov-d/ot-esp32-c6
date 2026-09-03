#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NVS_DEVICE_ID_MAX       16
#define NVS_SSID_MAX            33
#define NVS_PASS_MAX            65
#define NVS_MQTT_HOST_MAX       128
#define NVS_MQTT_USER_MAX       64
#define NVS_MQTT_PASS_MAX       64
#define NVS_MQTT_CA_PEM_MAX     4096
#define NVS_CATALOG_BLOB_MAX    520
/** SoftAP WPA2-PSK: 16 hex chars + NUL (stable per device; not cleared with credentials). */
#define NVS_SOFTAP_PSK_MAX      17

typedef struct {
    char device_id[NVS_DEVICE_ID_MAX];
    char wifi_ssid[NVS_SSID_MAX];
    char wifi_password[NVS_PASS_MAX];
    char mqtt_host[NVS_MQTT_HOST_MAX];
    uint16_t mqtt_port;
    char mqtt_username[NVS_MQTT_USER_MAX];
    char mqtt_password[NVS_MQTT_PASS_MAX];
    bool mqtt_tls;
    float ch_min_c;
    float ch_max_c;
    bool has_last_accepted_ch;
    float last_accepted_ch_setpoint_c;
    bool has_wifi_credentials;
    bool has_mqtt_config;
} nvs_gateway_config_t;

/** Initialize NVS flash and load gateway config (seeds defaults on miss). */
esp_err_t nvs_store_init(void);

/** Copy current config into *out. */
esp_err_t nvs_store_get(nvs_gateway_config_t *out);

/** Persist full gateway config (credentials, CH bounds, last CH, identity). */
esp_err_t nvs_store_save(const nvs_gateway_config_t *cfg);

/** Clear Wi‑Fi + MQTT credentials (SoftAP re-provision). Keeps CH bounds / identity / SoftAP PSK. */
esp_err_t nvs_store_clear_credentials(void);

/**
 * Load SoftAP WPA2-PSK from NVS, or generate and persist a random 16-hex PSK.
 * Survives credential clear so serial/label/QR stay valid across re-provision.
 */
esp_err_t nvs_store_ensure_softap_psk(char *psk, size_t cap);

/** Save last-accepted CH setpoint (°C). */
esp_err_t nvs_store_set_last_ch_setpoint(float celsius);

/** Load/save opaque catalog blob (versioned by caller). */
esp_err_t nvs_store_catalog_save(const uint8_t *blob, size_t len);
esp_err_t nvs_store_catalog_load(uint8_t *blob, size_t cap, size_t *out_len);

/**
 * MQTT broker CA / server PEM (NUL-terminated string stored as blob).
 * Max NVS_MQTT_CA_PEM_MAX including trailing NUL.
 */
esp_err_t nvs_store_mqtt_ca_save(const char *pem);
esp_err_t nvs_store_mqtt_ca_load(char *buf, size_t cap, size_t *out_len);
esp_err_t nvs_store_mqtt_ca_clear(void);

/** Ensure device_id from MAC if empty. */
esp_err_t nvs_store_ensure_device_id(char *device_id, size_t cap);

#ifdef __cplusplus
}
#endif
