#include "app_config.h"
#include "failsafe.h"
#include "mqtt_commands.h"
#include "mqtt_discovery.h"
#include "mqtt_ha.h"
#include "nvs_store.h"
#include "ot_catalog.h"
#include "ot_codec.h"
#include "ot_poll.h"
#include "provision_softap.h"

#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "nvs_flash.h"

#include <string.h>

static const char *TAG = "main";

static nvs_gateway_config_t s_cfg;
static ot_catalog_t s_catalog;
static failsafe_state_t s_failsafe;
static EventGroupHandle_t s_wifi_events;
static bool s_wifi_up;
static bool s_got_ip;

#define WIFI_OK_BIT BIT0

static void wifi_event_handler(void *arg, esp_event_base_t base, int32_t id, void *data)
{
    (void)arg;
    if (base == WIFI_EVENT && id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (base == WIFI_EVENT && id == WIFI_EVENT_STA_DISCONNECTED) {
        s_wifi_up = false;
        s_got_ip = false;
        esp_wifi_connect();
    } else if (base == IP_EVENT && id == IP_EVENT_STA_GOT_IP) {
        s_wifi_up = true;
        s_got_ip = true;
        xEventGroupSetBits(s_wifi_events, WIFI_OK_BIT);
    }
}

static esp_err_t wifi_sta_start(void)
{
    s_wifi_events = xEventGroupCreate();
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, NULL));
    wifi_config_t wcfg = { 0 };
    strncpy((char *)wcfg.sta.ssid, s_cfg.wifi_ssid, sizeof(wcfg.sta.ssid) - 1);
    strncpy((char *)wcfg.sta.password, s_cfg.wifi_password, sizeof(wcfg.sta.password) - 1);
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wcfg));
    ESP_ERROR_CHECK(esp_wifi_start());
    EventBits_t bits = xEventGroupWaitBits(s_wifi_events, WIFI_OK_BIT, false, true, pdMS_TO_TICKS(30000));
    return (bits & WIFI_OK_BIT) ? ESP_OK : ESP_ERR_TIMEOUT;
}

static void failsafe_task(void *arg)
{
    (void)arg;
    bool was_active = false;
    while (1) {
        bool mqtt_up = mqtt_ha_connected();
        uint32_t now = xTaskGetTickCount() * portTICK_PERIOD_MS;
        failsafe_on_link(&s_failsafe, s_wifi_up && s_got_ip, mqtt_up, now);
        bool active = failsafe_is_active(&s_failsafe);
        if (active && !was_active) {
            /* Option A: present offline when active */
            mqtt_ha_publish_offline();
            if (s_cfg.has_last_accepted_ch) {
                ot_poll_set_hold_ch_setpoint(true, s_cfg.last_accepted_ch_setpoint_c);
                failsafe_set_held_ch(&s_failsafe, s_cfg.last_accepted_ch_setpoint_c);
            }
            ESP_LOGW(TAG, "fail-safe ACTIVE");
        } else if (!active && was_active) {
            ot_poll_set_hold_ch_setpoint(false, 0);
            mqtt_ha_publish_birth_online();
            mqtt_discovery_publish_all(s_cfg.device_id, &s_catalog, s_cfg.ch_min_c, s_cfg.ch_max_c);
            mqtt_commands_start_subscriptions();
            ESP_LOGI(TAG, "fail-safe cleared");
        } else if (!active && failsafe_app_availability_online(&s_failsafe) &&
                   s_failsafe.phase == FAILSAFE_ENTRY_TIMER) {
            /* stay online during entry timer — no publish spam */
        }
        was_active = active;
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

static void state_publish_task(void *arg)
{
    (void)arg;
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
        if (!mqtt_ha_connected() || !s_catalog.validated) {
            continue;
        }
        for (int id = 0; id < OT_CATALOG_MAX_IDS; id++) {
            const ot_catalog_entry_t *e = &s_catalog.ids[id];
            if (e->support != OT_SUPPORT_AVAILABLE) {
                continue;
            }
            char state[32];
            if (!e->has_raw) {
                state[0] = '\0';
            } else if (id == 1 || id == 8 || id == 25 || id == 56 || id == 57) {
                ot_codec_format_float(state, sizeof(state), ot_codec_f88_to_float(e->last_raw), true);
            } else {
                ot_codec_format_u16(state, sizeof(state), e->last_raw, true);
            }
            mqtt_discovery_publish_state(s_cfg.device_id, (uint8_t)id, state);
        }
        if (s_catalog.ids[0].support == OT_SUPPORT_AVAILABLE) {
            mqtt_discovery_publish_status_projections(s_cfg.device_id,
                                                      ot_poll_get_master_status_flags(),
                                                      ot_poll_get_slave_status_flags(),
                                                      s_catalog.ids[0].writable);
        }
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "OTC6 gateway %s", APP_FW_VERSION);
    ESP_ERROR_CHECK(nvs_store_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(nvs_store_get(&s_cfg));
    ESP_ERROR_CHECK(provision_button_start());
    ESP_ERROR_CHECK(failsafe_init(&s_failsafe, APP_FAILSAFE_ENTRY_TIMER_MS, APP_LINK_UP_DEBOUNCE_MS));
    mqtt_commands_set_failsafe(&s_failsafe);

    ESP_ERROR_CHECK(ot_poll_init());
    ot_catalog_init(&s_catalog);
    ot_poll_set_catalog(&s_catalog);
    ESP_ERROR_CHECK(ot_poll_start());

    if (!s_cfg.has_wifi_credentials || !s_cfg.has_mqtt_config) {
        ESP_LOGW(TAG, "no credentials — SoftAP provisioning");
        ESP_ERROR_CHECK(provision_softap_start(s_cfg.device_id));
        return;
    }

    if (wifi_sta_start() != ESP_OK) {
        ESP_LOGW(TAG, "STA join failed — staying up for retry/button");
    }

    ESP_ERROR_CHECK(mqtt_ha_init(s_cfg.device_id, s_cfg.mqtt_host, s_cfg.mqtt_port,
                                 s_cfg.mqtt_username, s_cfg.mqtt_password, s_cfg.mqtt_tls));
    ESP_ERROR_CHECK(mqtt_ha_start());

    /* Catalog: load cache then discover/validate */
    if (ot_catalog_load_nvs(&s_catalog) != ESP_OK) {
        ESP_LOGI(TAG, "catalog discovery (cold)");
        ot_catalog_discover(&s_catalog);
    } else {
        ESP_LOGI(TAG, "catalog loaded; re-validate");
        ot_catalog_discover(&s_catalog);
    }

    mqtt_commands_init(s_cfg.device_id, &s_catalog, s_cfg.ch_min_c, s_cfg.ch_max_c);
    /* Wait briefly for MQTT */
    for (int i = 0; i < 50 && !mqtt_ha_connected(); i++) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    if (mqtt_ha_connected()) {
        mqtt_discovery_publish_all(s_cfg.device_id, &s_catalog, s_cfg.ch_min_c, s_cfg.ch_max_c);
        mqtt_discovery_publish_climate(s_cfg.device_id, s_cfg.ch_min_c, s_cfg.ch_max_c);
        mqtt_commands_start_subscriptions();
    }

    xTaskCreate(failsafe_task, "failsafe", 3072, NULL, 4, NULL);
    xTaskCreate(state_publish_task, "ot_state", 3072, NULL, 3, NULL);
    ESP_LOGI(TAG, "operational");
}
