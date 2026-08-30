#include "provision_softap.h"

#include "app_config.h"
#include "dns_server.h"
#include "nvs_store.h"

#include "esp_event.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lwip/inet.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG = "provision";

static httpd_handle_t s_server;
static dns_server_handle_t s_dns;
static bool s_active;
static TaskHandle_t s_btn_task;
static char s_portal_uri[32];

bool provision_is_active(void)
{
    return s_active;
}

static const char *HTML_FORM =
    "<!DOCTYPE html><html><head><meta name=viewport content=\"width=device-width,initial-scale=1\">"
    "<title>OTC6 Setup</title></head><body>"
    "<h1>OpenTherm Gateway</h1>"
    "<form method=POST action=/save>"
    "Wi‑Fi SSID<br><input name=wifi_ssid required><br>"
    "Wi‑Fi password<br><input name=wifi_pass type=password><br>"
    "MQTT host<br><input name=mqtt_host required><br>"
    "MQTT port<br><input name=mqtt_port type=number value=1883 required><br>"
    "MQTT username<br><input name=mqtt_user><br>"
    "MQTT password<br><input name=mqtt_pass type=password><br>"
    "MQTT TLS <input name=mqtt_tls type=checkbox value=1><br>"
    "MQTT CA PEM (required if TLS)<br>"
    "<textarea name=mqtt_ca rows=8 cols=40 placeholder=\"-----BEGIN CERTIFICATE-----\"></textarea><br>"
    "CH min °C<br><input name=ch_min type=number step=0.1 value=10 required><br>"
    "CH max °C<br><input name=ch_max type=number step=0.1 value=90 required><br>"
    "<button type=submit>Save</button></form></body></html>";

enum { SAVE_BODY_MAX = 8192 };

static void url_decode(char *s)
{
    char *o = s;
    while (*s) {
        if (*s == '+') {
            *o++ = ' ';
            s++;
        } else if (*s == '%' && s[1] && s[2]) {
            char hex[3] = { s[1], s[2], 0 };
            *o++ = (char)strtol(hex, NULL, 16);
            s += 3;
        } else {
            *o++ = *s++;
        }
    }
    *o = '\0';
}

static bool form_get(const char *body, const char *key, char *out, size_t cap)
{
    char pattern[64];
    snprintf(pattern, sizeof(pattern), "%s=", key);
    const char *p = strstr(body, pattern);
    if (!p) {
        if (out && cap) {
            out[0] = '\0';
        }
        return false;
    }
    p += strlen(pattern);
    size_t i = 0;
    while (*p && *p != '&' && i + 1 < cap) {
        out[i++] = *p++;
    }
    out[i] = '\0';
    url_decode(out);
    return true;
}

static esp_err_t root_get(httpd_req_t *req)
{
    ESP_LOGI(TAG, "GET %s", req->uri);
    httpd_resp_set_type(req, "text/html");
    httpd_resp_set_hdr(req, "Cache-Control", "no-store");
    return httpd_resp_send(req, HTML_FORM, HTTPD_RESP_USE_STRLEN);
}

/* Captive probes hit random paths; redirect so OS shows the portal. */
static esp_err_t http_404_handler(httpd_req_t *req, httpd_err_code_t err)
{
    (void)err;
    ESP_LOGI(TAG, "404 %s → /", req->uri);
    httpd_resp_set_status(req, "303 See Other");
    httpd_resp_set_hdr(req, "Location", "/");
    /* iOS needs a body; empty redirect is not enough */
    return httpd_resp_send(req, "Redirect to captive portal", HTTPD_RESP_USE_STRLEN);
}

static esp_err_t save_post(httpd_req_t *req)
{
    if (req->content_len <= 0 || req->content_len >= SAVE_BODY_MAX) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "body too large or empty");
        return ESP_FAIL;
    }

    char *body = malloc(SAVE_BODY_MAX);
    if (!body) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "oom");
        return ESP_FAIL;
    }

    int total = 0;
    while (total < (int)req->content_len && total < SAVE_BODY_MAX - 1) {
        int r = httpd_req_recv(req, body + total, (int)req->content_len - total);
        if (r <= 0) {
            free(body);
            httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "recv failed");
            return ESP_FAIL;
        }
        total += r;
    }
    body[total] = '\0';

    /* Large form: allocate off-stack (includes mqtt_ca[4096]) */
    provision_form_t *form = calloc(1, sizeof(*form));
    if (!form) {
        free(body);
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "oom");
        return ESP_FAIL;
    }

    form_get(body, "wifi_ssid", form->wifi_ssid, sizeof(form->wifi_ssid));
    form_get(body, "wifi_pass", form->wifi_password, sizeof(form->wifi_password));
    form_get(body, "mqtt_host", form->mqtt_host, sizeof(form->mqtt_host));
    char tmp[32];
    form_get(body, "mqtt_port", tmp, sizeof(tmp));
    form->mqtt_port = (uint16_t)atoi(tmp);
    if (form->mqtt_port == 0) {
        form->mqtt_port = 1883;
    }
    form_get(body, "mqtt_user", form->mqtt_username, sizeof(form->mqtt_username));
    form_get(body, "mqtt_pass", form->mqtt_password, sizeof(form->mqtt_password));
    form->mqtt_tls = strstr(body, "mqtt_tls=1") != NULL;
    form_get(body, "mqtt_ca", form->mqtt_ca, sizeof(form->mqtt_ca));
    form_get(body, "ch_min", tmp, sizeof(tmp));
    form->ch_min_c = strtof(tmp, NULL);
    form_get(body, "ch_max", tmp, sizeof(tmp));
    form->ch_max_c = strtof(tmp, NULL);
    free(body);

    provision_validate_result_t v = provision_validate(form);
    if (v != PROVISION_OK) {
        free(form);
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "validation failed");
        return ESP_FAIL;
    }

    nvs_gateway_config_t cfg;
    nvs_store_get(&cfg);
    strncpy(cfg.wifi_ssid, form->wifi_ssid, sizeof(cfg.wifi_ssid) - 1);
    strncpy(cfg.wifi_password, form->wifi_password, sizeof(cfg.wifi_password) - 1);
    strncpy(cfg.mqtt_host, form->mqtt_host, sizeof(cfg.mqtt_host) - 1);
    cfg.mqtt_port = form->mqtt_port;
    strncpy(cfg.mqtt_username, form->mqtt_username, sizeof(cfg.mqtt_username) - 1);
    strncpy(cfg.mqtt_password, form->mqtt_password, sizeof(cfg.mqtt_password) - 1);
    cfg.mqtt_tls = form->mqtt_tls;
    cfg.ch_min_c = form->ch_min_c;
    cfg.ch_max_c = form->ch_max_c;
    cfg.has_wifi_credentials = true;
    cfg.has_mqtt_config = true;
    nvs_store_save(&cfg);

    if (form->mqtt_tls) {
        nvs_store_mqtt_ca_save(form->mqtt_ca);
    } else {
        nvs_store_mqtt_ca_clear();
    }
    free(form);

    httpd_resp_sendstr(req, "Saved. Rebooting to STA...");
    ESP_LOGI(TAG, "credentials saved; reboot");
    vTaskDelay(pdMS_TO_TICKS(500));
    esp_restart();
    return ESP_OK;
}

static void start_httpd(void)
{
    /* Captive DNS floods httpd with junk; keep noise down */
    esp_log_level_set("httpd_uri", ESP_LOG_ERROR);
    esp_log_level_set("httpd_txrx", ESP_LOG_ERROR);
    esp_log_level_set("httpd_parse", ESP_LOG_ERROR);

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = 80;
    config.max_open_sockets = 7;
    config.lru_purge_enable = true;
    if (httpd_start(&s_server, &config) != ESP_OK) {
        ESP_LOGE(TAG, "httpd start failed");
        return;
    }
    httpd_uri_t root = { .uri = "/", .method = HTTP_GET, .handler = root_get };
    httpd_uri_t save = { .uri = "/save", .method = HTTP_POST, .handler = save_post };
    httpd_register_uri_handler(s_server, &root);
    httpd_register_uri_handler(s_server, &save);
    httpd_register_err_handler(s_server, HTTPD_404_NOT_FOUND, http_404_handler);
}

static void dhcp_set_captive_portal_uri(void)
{
    esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_AP_DEF");
    if (!netif) {
        return;
    }
    esp_netif_ip_info_t ip_info;
    if (esp_netif_get_ip_info(netif, &ip_info) != ESP_OK) {
        return;
    }
    char ip_addr[16];
    inet_ntoa_r(ip_info.ip.addr, ip_addr, sizeof(ip_addr));
    snprintf(s_portal_uri, sizeof(s_portal_uri), "http://%s", ip_addr);

    esp_netif_dhcps_stop(netif);
    esp_err_t err = esp_netif_dhcps_option(netif, ESP_NETIF_OP_SET, ESP_NETIF_CAPTIVEPORTAL_URI,
                                           s_portal_uri, strlen(s_portal_uri));
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "DHCP captive URI option: %s", esp_err_to_name(err));
    }
    esp_netif_dhcps_start(netif);
    ESP_LOGI(TAG, "portal URL %s", s_portal_uri);
}

esp_err_t provision_softap_start(const char *device_id)
{
    if (s_active) {
        return ESP_OK;
    }

    char psk[NVS_SOFTAP_PSK_MAX];
    esp_err_t err = nvs_store_ensure_softap_psk(psk, sizeof(psk));
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "SoftAP PSK unavailable: %s", esp_err_to_name(err));
        return err;
    }

    provision_softap_ap_params_t ap_params;
    if (!provision_softap_build_ap_params(device_id, psk, &ap_params)) {
        ESP_LOGE(TAG, "SoftAP AP params invalid (PSK required)");
        return ESP_ERR_INVALID_STATE;
    }
    if (ap_params.authmode != PROVISION_SOFTAP_AUTH_WPA2_PSK) {
        ESP_LOGE(TAG, "refusing open SoftAP");
        return ESP_ERR_INVALID_STATE;
    }

    if (esp_netif_get_handle_from_ifkey("WIFI_AP_DEF") == NULL) {
        esp_netif_create_default_wifi_ap();
    }

    wifi_mode_t cur_mode;
    esp_err_t mode_err = esp_wifi_get_mode(&cur_mode);
    bool wifi_already_init = (mode_err != ESP_ERR_WIFI_NOT_INIT);
    /* If get_mode works, the driver has been started at least once in STA paths;
     * stop before AP switch. Virgin SoftAP: get_mode fails with NOT_INIT. */
    bool wifi_started = wifi_already_init;
    provision_softap_wifi_plan_t plan;
    provision_softap_plan_wifi(wifi_already_init, wifi_started, &plan);

    if (plan.call_wifi_init) {
        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        err = esp_wifi_init(&cfg);
        /* Fresh init must succeed; already-init is handled by skipping call_wifi_init.
         * Tolerate ESP_ERR_WIFI_INIT_STATE / ESP_OK if a race inits underneath us. */
        if (err != ESP_OK && err != ESP_ERR_WIFI_INIT_STATE) {
            ESP_LOGE(TAG, "esp_wifi_init: %s", esp_err_to_name(err));
            return err;
        }
    }

    if (plan.call_wifi_stop) {
        err = esp_wifi_stop();
        if (err != ESP_OK && err != ESP_ERR_WIFI_NOT_STARTED) {
            ESP_LOGW(TAG, "esp_wifi_stop: %s", esp_err_to_name(err));
        }
    }

    err = esp_wifi_set_mode(WIFI_MODE_AP);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_set_mode(AP): %s", esp_err_to_name(err));
        return err;
    }

    wifi_config_t ap = { 0 };
    strncpy((char *)ap.ap.ssid, ap_params.ssid, sizeof(ap.ap.ssid) - 1);
    ap.ap.ssid_len = strlen(ap_params.ssid);
    strncpy((char *)ap.ap.password, ap_params.password, sizeof(ap.ap.password) - 1);
    ap.ap.channel = 1;
    ap.ap.max_connection = 4;
    ap.ap.authmode = WIFI_AUTH_WPA2_PSK;
    err = esp_wifi_set_config(WIFI_IF_AP, &ap);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_set_config(AP): %s", esp_err_to_name(err));
        return err;
    }
    err = esp_wifi_start();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_start: %s", esp_err_to_name(err));
        return err;
    }

    dhcp_set_captive_portal_uri();
    start_httpd();

    /* Contract: DNS catch-all → SoftAP IP so phones open the portal */
    dns_server_config_t dns_cfg = DNS_SERVER_CONFIG_SINGLE("*", "WIFI_AP_DEF");
    s_dns = start_dns_server(&dns_cfg);
    if (!s_dns) {
        ESP_LOGE(TAG, "DNS captive server failed");
    }

    s_active = true;
    /* SoftAP PSK is logged for serial commissioning (label/QR may mirror this). */
    ESP_LOGI(TAG,
             "SoftAP %s WPA2-PSK password=%s — join then open %s/ if portal does not pop up",
             ap_params.ssid, ap_params.password,
             s_portal_uri[0] ? s_portal_uri : "http://192.168.4.1");
    return ESP_OK;
}

void provision_softap_stop(void)
{
    if (s_dns) {
        stop_dns_server(s_dns);
        s_dns = NULL;
    }
    if (s_server) {
        httpd_stop(s_server);
        s_server = NULL;
    }
    s_active = false;
}

static void button_task(void *arg)
{
    (void)arg;
    gpio_config_t io = {
        .pin_bit_mask = 1ULL << APP_SOFTAP_BUTTON_GPIO,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io);
    while (1) {
        if (gpio_get_level(APP_SOFTAP_BUTTON_GPIO) == 0) {
            TickType_t start = xTaskGetTickCount();
            while (gpio_get_level(APP_SOFTAP_BUTTON_GPIO) == 0) {
                if ((xTaskGetTickCount() - start) >= pdMS_TO_TICKS(APP_SOFTAP_LONG_PRESS_MS)) {
                    ESP_LOGW(TAG, "long-press: clear credentials + SoftAP");
                    nvs_store_clear_credentials();
                    esp_restart();
                }
                vTaskDelay(pdMS_TO_TICKS(50));
            }
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

esp_err_t provision_button_start(void)
{
    if (s_btn_task) {
        return ESP_OK;
    }
    BaseType_t ok = xTaskCreate(button_task, "prov_btn", 2048, NULL, 3, &s_btn_task);
    return ok == pdPASS ? ESP_OK : ESP_ERR_NO_MEM;
}
