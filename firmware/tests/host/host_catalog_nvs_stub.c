#include "nvs_store.h"

#include <string.h>

static uint8_t s_blob[NVS_CATALOG_BLOB_MAX];
static size_t s_blob_len;

esp_err_t nvs_store_catalog_save(const uint8_t *blob, size_t len)
{
    if (!blob || len > sizeof(s_blob)) {
        return ESP_ERR_INVALID_ARG;
    }
    memcpy(s_blob, blob, len);
    s_blob_len = len;
    return ESP_OK;
}

esp_err_t nvs_store_catalog_load(uint8_t *blob, size_t cap, size_t *out_len)
{
    if (!blob || !out_len || s_blob_len == 0) {
        return ESP_ERR_NOT_FOUND;
    }
    if (cap < s_blob_len) {
        return ESP_ERR_INVALID_SIZE;
    }
    memcpy(blob, s_blob, s_blob_len);
    *out_len = s_blob_len;
    return ESP_OK;
}

esp_err_t nvs_store_set_last_ch_setpoint(float celsius)
{
    (void)celsius;
    return ESP_OK;
}
