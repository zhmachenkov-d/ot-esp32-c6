#pragma once
/* Minimal nvs_store.h for host — catalog blob only */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "esp_err.h"

#define NVS_DEVICE_ID_MAX       16
#define NVS_SSID_MAX            33
#define NVS_PASS_MAX            65
#define NVS_MQTT_HOST_MAX       128
#define NVS_MQTT_USER_MAX       64
#define NVS_MQTT_PASS_MAX       64
#define NVS_MQTT_CA_PEM_MAX     4096
#define NVS_CATALOG_BLOB_MAX    512

esp_err_t nvs_store_catalog_save(const uint8_t *blob, size_t len);
esp_err_t nvs_store_catalog_load(uint8_t *blob, size_t cap, size_t *out_len);
esp_err_t nvs_store_mqtt_ca_save(const char *pem);
esp_err_t nvs_store_mqtt_ca_load(char *buf, size_t cap, size_t *out_len);
esp_err_t nvs_store_mqtt_ca_clear(void);
esp_err_t nvs_store_set_last_ch_setpoint(float celsius);
