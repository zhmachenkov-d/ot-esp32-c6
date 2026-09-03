#include "nvs_store.h"

#include "app_config.h"

#include "esp_log.h"
#include "esp_mac.h"
#include "esp_random.h"
#include "nvs.h"
#include "nvs_flash.h"

#include <stdio.h>
#include <string.h>

static const char *TAG = "nvs_store";
static const char *NS = "otc6";

static nvs_gateway_config_t s_cfg;
static bool s_ready;

static void seed_defaults(nvs_gateway_config_t *c)
{
    memset(c, 0, sizeof(*c));
    c->mqtt_port = 1883;
    c->mqtt_tls = false;
    c->ch_min_c = APP_CH_MIN_C_DEFAULT;
    c->ch_max_c = APP_CH_MAX_C_DEFAULT;
}

esp_err_t nvs_store_ensure_device_id(char *device_id, size_t cap)
{
    if (!device_id || cap < 13) {
        return ESP_ERR_INVALID_ARG;
    }
    if (device_id[0] != '\0') {
        return ESP_OK;
    }
    uint8_t mac[6];
    esp_err_t err = esp_read_mac(mac, ESP_MAC_WIFI_STA);
    if (err != ESP_OK) {
        return err;
    }
    snprintf(device_id, cap, "%02x%02x%02x%02x%02x%02x",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return ESP_OK;
}

static esp_err_t load_from_nvs(nvs_gateway_config_t *c)
{
    nvs_handle_t h;
    esp_err_t err = nvs_open(NS, NVS_READONLY, &h);
    if (err != ESP_OK) {
        return err;
    }
    seed_defaults(c);
    size_t len;

    len = sizeof(c->device_id);
    nvs_get_str(h, "device_id", c->device_id, &len);
    len = sizeof(c->wifi_ssid);
    if (nvs_get_str(h, "wifi_ssid", c->wifi_ssid, &len) == ESP_OK && c->wifi_ssid[0]) {
        c->has_wifi_credentials = true;
    }
    len = sizeof(c->wifi_password);
    nvs_get_str(h, "wifi_pass", c->wifi_password, &len);
    len = sizeof(c->mqtt_host);
    if (nvs_get_str(h, "mqtt_host", c->mqtt_host, &len) == ESP_OK && c->mqtt_host[0]) {
        c->has_mqtt_config = true;
    }
    uint16_t port = 1883;
    if (nvs_get_u16(h, "mqtt_port", &port) == ESP_OK) {
        c->mqtt_port = port;
    }
    len = sizeof(c->mqtt_username);
    nvs_get_str(h, "mqtt_user", c->mqtt_username, &len);
    len = sizeof(c->mqtt_password);
    nvs_get_str(h, "mqtt_pass", c->mqtt_password, &len);
    uint8_t tls = 0;
    if (nvs_get_u8(h, "mqtt_tls", &tls) == ESP_OK) {
        c->mqtt_tls = tls != 0;
    }
    int32_t imin = 0, imax = 0;
    if (nvs_get_i32(h, "ch_min_x100", &imin) == ESP_OK) {
        c->ch_min_c = ((float)imin) / 100.0f;
    }
    if (nvs_get_i32(h, "ch_max_x100", &imax) == ESP_OK) {
        c->ch_max_c = ((float)imax) / 100.0f;
    }
    int32_t last = 0;
    if (nvs_get_i32(h, "last_ch_x100", &last) == ESP_OK) {
        c->has_last_accepted_ch = true;
        c->last_accepted_ch_setpoint_c = ((float)last) / 100.0f;
    }
    nvs_close(h);
    nvs_store_ensure_device_id(c->device_id, sizeof(c->device_id));
    return ESP_OK;
}

esp_err_t nvs_store_init(void)
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    if (err != ESP_OK) {
        return err;
    }
    seed_defaults(&s_cfg);
    err = load_from_nvs(&s_cfg);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        nvs_store_ensure_device_id(s_cfg.device_id, sizeof(s_cfg.device_id));
        err = ESP_OK;
    }
    s_ready = (err == ESP_OK);
    ESP_LOGI(TAG, "init ok device_id=%s wifi=%d mqtt=%d",
             s_cfg.device_id, (int)s_cfg.has_wifi_credentials, (int)s_cfg.has_mqtt_config);
    return err;
}

esp_err_t nvs_store_get(nvs_gateway_config_t *out)
{
    if (!out || !s_ready) {
        return ESP_ERR_INVALID_STATE;
    }
    *out = s_cfg;
    return ESP_OK;
}

esp_err_t nvs_store_save(const nvs_gateway_config_t *cfg)
{
    if (!cfg) {
        return ESP_ERR_INVALID_ARG;
    }
    nvs_handle_t h;
    esp_err_t err = nvs_open(NS, NVS_READWRITE, &h);
    if (err != ESP_OK) {
        return err;
    }
    nvs_set_str(h, "device_id", cfg->device_id);
    nvs_set_str(h, "wifi_ssid", cfg->wifi_ssid);
    nvs_set_str(h, "wifi_pass", cfg->wifi_password);
    nvs_set_str(h, "mqtt_host", cfg->mqtt_host);
    nvs_set_u16(h, "mqtt_port", cfg->mqtt_port);
    nvs_set_str(h, "mqtt_user", cfg->mqtt_username);
    nvs_set_str(h, "mqtt_pass", cfg->mqtt_password);
    nvs_set_u8(h, "mqtt_tls", cfg->mqtt_tls ? 1 : 0);
    nvs_set_i32(h, "ch_min_x100", (int32_t)(cfg->ch_min_c * 100.0f));
    nvs_set_i32(h, "ch_max_x100", (int32_t)(cfg->ch_max_c * 100.0f));
    if (cfg->has_last_accepted_ch) {
        nvs_set_i32(h, "last_ch_x100", (int32_t)(cfg->last_accepted_ch_setpoint_c * 100.0f));
    }
    err = nvs_commit(h);
    nvs_close(h);
    if (err == ESP_OK) {
        s_cfg = *cfg;
        s_cfg.has_wifi_credentials = cfg->wifi_ssid[0] != '\0';
        s_cfg.has_mqtt_config = cfg->mqtt_host[0] != '\0';
    }
    return err;
}

esp_err_t nvs_store_clear_credentials(void)
{
    nvs_gateway_config_t c = s_cfg;
    c.wifi_ssid[0] = '\0';
    c.wifi_password[0] = '\0';
    c.mqtt_host[0] = '\0';
    c.mqtt_username[0] = '\0';
    c.mqtt_password[0] = '\0';
    c.mqtt_tls = false;
    c.mqtt_port = 1883;
    c.has_wifi_credentials = false;
    c.has_mqtt_config = false;
    esp_err_t err = nvs_store_save(&c);
    if (err != ESP_OK) {
        return err;
    }
    return nvs_store_mqtt_ca_clear();
}

esp_err_t nvs_store_ensure_softap_psk(char *psk, size_t cap)
{
    if (!psk || cap < NVS_SOFTAP_PSK_MAX) {
        return ESP_ERR_INVALID_ARG;
    }
    psk[0] = '\0';

    nvs_handle_t h;
    esp_err_t err = nvs_open(NS, NVS_READONLY, &h);
    if (err == ESP_OK) {
        size_t len = cap;
        err = nvs_get_str(h, "softap_psk", psk, &len);
        nvs_close(h);
        if (err == ESP_OK && strlen(psk) >= 8) {
            return ESP_OK;
        }
        psk[0] = '\0';
    }

    uint8_t rnd[8];
    esp_fill_random(rnd, sizeof(rnd));
    for (size_t i = 0; i < sizeof(rnd); i++) {
        snprintf(psk + (i * 2), 3, "%02x", rnd[i]);
    }

    err = nvs_open(NS, NVS_READWRITE, &h);
    if (err != ESP_OK) {
        return err;
    }
    err = nvs_set_str(h, "softap_psk", psk);
    if (err == ESP_OK) {
        err = nvs_commit(h);
    }
    nvs_close(h);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "generated SoftAP WPA2-PSK (persisted)");
    }
    return err;
}

esp_err_t nvs_store_mqtt_ca_save(const char *pem)
{
    if (!pem || !pem[0]) {
        return ESP_ERR_INVALID_ARG;
    }
    size_t len = strlen(pem) + 1; /* include NUL for PEM consumers */
    if (len > NVS_MQTT_CA_PEM_MAX) {
        return ESP_ERR_INVALID_ARG;
    }
    nvs_handle_t h;
    esp_err_t err = nvs_open(NS, NVS_READWRITE, &h);
    if (err != ESP_OK) {
        return err;
    }
    err = nvs_set_blob(h, "mqtt_ca", pem, len);
    if (err == ESP_OK) {
        err = nvs_commit(h);
    }
    nvs_close(h);
    return err;
}

esp_err_t nvs_store_mqtt_ca_load(char *buf, size_t cap, size_t *out_len)
{
    if (!buf || cap == 0 || !out_len) {
        return ESP_ERR_INVALID_ARG;
    }
    nvs_handle_t h;
    esp_err_t err = nvs_open(NS, NVS_READONLY, &h);
    if (err != ESP_OK) {
        return err;
    }
    size_t len = cap;
    err = nvs_get_blob(h, "mqtt_ca", buf, &len);
    nvs_close(h);
    if (err == ESP_OK) {
        if (len == 0 || buf[len - 1] != '\0') {
            /* Ensure C string even if older blob lacked NUL */
            if (len >= cap) {
                return ESP_ERR_INVALID_SIZE;
            }
            buf[len] = '\0';
            len++;
        }
        *out_len = len;
    }
    return err;
}

esp_err_t nvs_store_mqtt_ca_clear(void)
{
    nvs_handle_t h;
    esp_err_t err = nvs_open(NS, NVS_READWRITE, &h);
    if (err != ESP_OK) {
        return err;
    }
    err = nvs_erase_key(h, "mqtt_ca");
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        err = ESP_OK;
    }
    if (err == ESP_OK) {
        err = nvs_commit(h);
    }
    nvs_close(h);
    return err;
}

esp_err_t nvs_store_set_last_ch_setpoint(float celsius)
{
    s_cfg.has_last_accepted_ch = true;
    s_cfg.last_accepted_ch_setpoint_c = celsius;
    nvs_handle_t h;
    esp_err_t err = nvs_open(NS, NVS_READWRITE, &h);
    if (err != ESP_OK) {
        return err;
    }
    nvs_set_i32(h, "last_ch_x100", (int32_t)(celsius * 100.0f));
    err = nvs_commit(h);
    nvs_close(h);
    return err;
}

esp_err_t nvs_store_catalog_save(const uint8_t *blob, size_t len)
{
    if (!blob || len == 0 || len > NVS_CATALOG_BLOB_MAX) {
        return ESP_ERR_INVALID_ARG;
    }
    nvs_handle_t h;
    esp_err_t err = nvs_open(NS, NVS_READWRITE, &h);
    if (err != ESP_OK) {
        return err;
    }
    err = nvs_set_blob(h, "catalog", blob, len);
    if (err == ESP_OK) {
        err = nvs_commit(h);
    }
    nvs_close(h);
    return err;
}

esp_err_t nvs_store_catalog_load(uint8_t *blob, size_t cap, size_t *out_len)
{
    if (!blob || !out_len) {
        return ESP_ERR_INVALID_ARG;
    }
    nvs_handle_t h;
    esp_err_t err = nvs_open(NS, NVS_READONLY, &h);
    if (err != ESP_OK) {
        return err;
    }
    size_t len = cap;
    err = nvs_get_blob(h, "catalog", blob, &len);
    nvs_close(h);
    if (err == ESP_OK) {
        *out_len = len;
    }
    return err;
}
