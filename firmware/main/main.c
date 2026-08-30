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
#include "esp_heap_caps.h"
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
static char s_mqtt_ca[NVS_MQTT_CA_PEM_MAX];
static ot_catalog_t s_catalog;
static failsafe_state_t s_failsafe;
static EventGroupHandle_t s_wifi_events;
static bool s_wifi_up;
static bool s_got_ip;
static bool s_mqtt_session_armed;
static bool s_mqtt_session_ready;
static uint32_t s_mqtt_session_since_ms;

#define WIFI_OK_BIT BIT0

static void mqtt_session_arm(void)
{
    /* Every MQTT (re)connect: arm retained gate; subscribe after 2 s debounce (T051/T053). */
    uint32_t now = xTaskGetTickCount() * portTICK_PERIOD_MS;
    mqtt_commands_set_time_ms(now);
    mqtt_commands_begin_post_recovery();
    s_mqtt_session_armed = true;
    s_mqtt_session_ready = false;
    s_mqtt_session_since_ms = now;
}

static void on_mqtt_connected(void *ctx)
{
    (void)ctx;
    mqtt_session_arm();
}

static void mqtt_session_tick(uint32_t now_ms)
{
    mqtt_commands_set_time_ms(now_ms);
    if (!s_mqtt_session_armed || s_mqtt_session_ready) {
        return;
    }
    if (!mqtt_ha_connected() || !(s_wifi_up && s_got_ip)) {
        return;
    }
    if ((now_ms - s_mqtt_session_since_ms) < APP_LINK_UP_DEBOUNCE_MS) {
        return;
    }
    mqtt_discovery_publish_all(s_cfg.device_id, &s_catalog, s_cfg.ch_min_c, s_cfg.ch_max_c);
    mqtt_discovery_publish_climate(s_cfg.device_id, s_cfg.ch_min_c, s_cfg.ch_max_c);
    mqtt_commands_start_subscriptions();
    s_mqtt_session_ready = true;
    ESP_LOGI(TAG, "MQTT session ready (discovery + subscribe after debounce)");
}

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
        mqtt_session_tick(now);
        bool active = failsafe_is_active(&s_failsafe);
        if (active && !was_active) {
            /* Option A: present offline when active — hold live NVS last CH (T047) */
            mqtt_ha_publish_offline();
            nvs_gateway_config_t live;
            if (nvs_store_get(&live) == ESP_OK && live.has_last_accepted_ch) {
                ot_poll_set_hold_ch_setpoint(true, live.last_accepted_ch_setpoint_c);
                failsafe_set_held_ch(&s_failsafe, live.last_accepted_ch_setpoint_c);
            }
            ESP_LOGW(TAG, "fail-safe ACTIVE");
        } else if (!active && was_active) {
            ot_poll_set_hold_ch_setpoint(false, 0);
            mqtt_ha_publish_birth_online();
            /* Retained gate + rediscovery/subscribe happen via mqtt_session_arm on reconnect
             * or re-arm here if MQTT stayed up through fail-safe. */
            if (mqtt_up) {
                mqtt_session_arm();
            }
            ESP_LOGI(TAG, "fail-safe cleared");
        } else if (!active && failsafe_app_availability_online(&s_failsafe) &&
                   s_failsafe.phase == FAILSAFE_ENTRY_TIMER) {
            /* stay online during entry timer — no publish spam */
        }
        if (!mqtt_up) {
            s_mqtt_session_armed = false;
            s_mqtt_session_ready = false;
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
            mqtt_discovery_publish_status_flag_states(s_cfg.device_id,
                                                      ot_poll_get_master_status_flags(),
                                                      ot_poll_get_slave_status_flags());
            mqtt_discovery_publish_climate_mode(s_cfg.device_id,
                                                (ot_poll_get_master_status_flags() & 0x01) != 0);
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

    const char *ca_pem = NULL;
    bool ca_pem_ok = true;
    if (s_cfg.mqtt_tls) {
        size_t ca_len = 0;
        esp_err_t ca_err = nvs_store_mqtt_ca_load(s_mqtt_ca, sizeof(s_mqtt_ca), &ca_len);
        ca_pem_ok = (ca_err == ESP_OK && s_mqtt_ca[0] != '\0');
        if (ca_pem_ok) {
            ca_pem = s_mqtt_ca;
        }
    }

    provision_boot_action_t boot =
        provision_boot_action(true, true, s_cfg.mqtt_tls, ca_pem_ok);
    if (boot == PROVISION_BOOT_RUN_NO_MQTT) {
        /* SoftAP only via GPIO9 long-press once credentials exist (contract). */
        ESP_LOGE(TAG,
                 "MQTT TLS enabled but CA PEM missing — MQTT disabled; "
                 "long-press GPIO9 to re-provision");
    } else {
        ESP_ERROR_CHECK(mqtt_ha_init(s_cfg.device_id, s_cfg.mqtt_host, s_cfg.mqtt_port,
                                     s_cfg.mqtt_username, s_cfg.mqtt_password, s_cfg.mqtt_tls,
                                     ca_pem));
        mqtt_ha_set_connected_callback(on_mqtt_connected, NULL);
        ESP_ERROR_CHECK(mqtt_ha_start());
    }

    /* Catalog: load cache then discover/validate */
    if (ot_catalog_load_nvs(&s_catalog) != ESP_OK) {
        ESP_LOGI(TAG, "catalog discovery (cold)");
        ot_catalog_discover(&s_catalog);
    } else {
        ESP_LOGI(TAG, "catalog loaded; re-validate");
        ot_catalog_discover(&s_catalog);
    }

    mqtt_commands_init(s_cfg.device_id, &s_catalog, s_cfg.ch_min_c, s_cfg.ch_max_c);
    /* Wait briefly for MQTT; discovery/subscribe run after link-up debounce (T051/T053) */
    for (int i = 0; i < 50 && !mqtt_ha_connected(); i++) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    if (mqtt_ha_connected() && !s_mqtt_session_armed) {
        mqtt_session_arm();
    }
    /* Drain debounce so first subscribe happens before we log heap */
    for (int i = 0; i < 15; i++) {
        uint32_t now = xTaskGetTickCount() * portTICK_PERIOD_MS;
        mqtt_session_tick(now);
        if (s_mqtt_session_ready) {
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(200));
    }
    if (mqtt_ha_connected()) {
        size_t free_heap = heap_caps_get_free_size(MALLOC_CAP_DEFAULT);
        ESP_LOGI(TAG, "free heap after OT+MQTT: %u bytes (budget >= 65536)", (unsigned)free_heap);
    }

    xTaskCreate(failsafe_task, "failsafe", 3072, NULL, 4, NULL);
    xTaskCreate(state_publish_task, "ot_state", 3072, NULL, 3, NULL);
    ESP_LOGI(TAG, "operational");
}
