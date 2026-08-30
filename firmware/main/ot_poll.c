#include "ot_poll.h"

#include "app_config.h"
#include "mqtt_ha.h"
#include "nvs_store.h"
#include "ot_catalog.h"
#include "ot_codec.h"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "opentherm.h"

#include <string.h>

static const char *TAG = "ot_poll";

typedef struct {
    uint8_t data_id;
    uint16_t raw;
} write_req_t;

static QueueHandle_t s_write_q;
static TaskHandle_t s_task;
static uint8_t s_master_hb;
static uint8_t s_slave_lb;
static int s_keepalive_fails;
static bool s_boiler_healthy = true;
static bool s_hold_ch;
static float s_hold_ch_c;
static uint32_t s_promote_until[OT_CATALOG_MAX_IDS];
static ot_catalog_t *s_cat_ref;
static bool s_inited;

void ot_poll_set_catalog(struct ot_catalog *cat)
{
    s_cat_ref = cat;
}
static ot_exchange_result_t map_status(open_therm_response_status_t st, open_therm_message_type_t msg)
{
    if (st == OT_STATUS_TIMEOUT) {
        return OT_EXCHANGE_TIMEOUT;
    }
    if (st == OT_STATUS_INVALID) {
        return OT_EXCHANGE_INVALID;
    }
    if (st != OT_STATUS_SUCCESS) {
        return OT_EXCHANGE_ERROR;
    }
    if (msg == OT_UNKNOWN_DATA_ID) {
        return OT_EXCHANGE_UNKNOWN_ID;
    }
    if (msg == OT_DATA_INVALID) {
        return OT_EXCHANGE_INVALID;
    }
    if (msg == OT_READ_ACK || msg == OT_WRITE_ACK) {
        return OT_EXCHANGE_OK;
    }
    return OT_EXCHANGE_ERROR;
}

static void gap_ms(unsigned ms)
{
    vTaskDelay(pdMS_TO_TICKS(ms));
}

esp_err_t ot_poll_init(void)
{
    if (s_inited) {
        return ESP_OK;
    }
    /*
     * Adapter assumptions (WeAct ESP32-C6 Mini):
     * - TTL OpenTherm adapter on GPIO2 (in/IRQ) and GPIO3 (out)
     * - Master role; single boiler slave
     * - sazanof/opentherm framing; C6 HIL must confirm IRQ/framing (T008 gate)
     */
    esp_ot_init(APP_OT_GPIO_IN, APP_OT_GPIO_OUT, false, NULL);
    s_write_q = xQueueCreate(8, sizeof(write_req_t));
    if (!s_write_q) {
        return ESP_ERR_NO_MEM;
    }
    s_master_hb = 0;
    s_inited = true;
    ESP_LOGI(TAG, "OT master init GPIO in=%d out=%d (sazanof)", APP_OT_GPIO_IN, APP_OT_GPIO_OUT);
    return ESP_OK;
}

ot_exchange_result_t ot_poll_exchange(ot_exchange_t *ex)
{
    if (!ex || !s_inited) {
        return OT_EXCHANGE_ERROR;
    }
    open_therm_message_type_t mtype = ex->is_write ? OT_WRITE_DATA : OT_READ_DATA;
    unsigned long request = esp_ot_build_request(mtype, (open_therm_message_id_t)ex->data_id, ex->request_value);
    unsigned long response = esp_ot_send_request(request);
    open_therm_response_status_t st = esp_ot_get_last_response_status();
    open_therm_message_type_t rtype = esp_ot_get_message_type(response);
    ex->response_value = esp_ot_get_uint(response);
    ot_exchange_result_t r = map_status(st, rtype);
    ex->result = r;
    gap_ms(APP_OT_INTER_FRAME_GAP_MS);
    return r;
}

bool ot_poll_enqueue_write(uint8_t data_id, uint16_t raw_value)
{
    if (!s_write_q) {
        return false;
    }
    write_req_t wr = { .data_id = data_id, .raw = raw_value };
    return xQueueSend(s_write_q, &wr, 0) == pdTRUE;
}

void ot_poll_set_master_status_flags(uint8_t master_hb)
{
    s_master_hb = master_hb;
}

uint8_t ot_poll_get_master_status_flags(void)
{
    return s_master_hb;
}

uint8_t ot_poll_get_slave_status_flags(void)
{
    return s_slave_lb;
}

bool ot_poll_boiler_link_healthy(void)
{
    return s_boiler_healthy;
}

void ot_poll_set_hold_ch_setpoint(bool enable, float celsius)
{
    s_hold_ch = enable;
    s_hold_ch_c = celsius;
}

void ot_poll_promote(uint8_t data_id)
{
    if (data_id < OT_CATALOG_MAX_IDS) {
        s_promote_until[data_id] = xTaskGetTickCount() * portTICK_PERIOD_MS + APP_OT_PROMOTE_MS;
    }
}

static void note_keepalive(ot_exchange_result_t r)
{
    if (r == OT_EXCHANGE_OK || r == OT_EXCHANGE_INVALID) {
        s_keepalive_fails = 0;
        if (!s_boiler_healthy) {
            s_boiler_healthy = true;
            mqtt_ha_publish_boiler_link(true);
            ESP_LOGI(TAG, "boiler_link healthy");
        }
    } else {
        s_keepalive_fails++;
        if (s_keepalive_fails >= APP_BOILER_LINK_FAIL_THRESHOLD && s_boiler_healthy) {
            s_boiler_healthy = false;
            mqtt_ha_publish_boiler_link(false);
            ESP_LOGW(TAG, "boiler_link unhealthy after %d fails", s_keepalive_fails);
        }
    }
}

static ot_exchange_result_t do_status_keepalive(void)
{
    ot_exchange_t ex = {
        .data_id = 0,
        .request_value = ot_codec_pack_hb_lb(s_master_hb, 0),
        .is_write = false,
    };
    ot_exchange_result_t r = ot_poll_exchange(&ex);
    if (r == OT_EXCHANGE_OK || r == OT_EXCHANGE_INVALID) {
        uint8_t hb, lb;
        ot_codec_unpack_hb_lb(ex.response_value, &hb, &lb);
        s_slave_lb = lb;
    }
    note_keepalive(r);
    return r;
}

static void apply_hold_ch(void)
{
    if (!s_hold_ch) {
        return;
    }
    ot_exchange_t ex = {
        .data_id = 1,
        .request_value = ot_codec_float_to_f88(s_hold_ch_c),
        .is_write = true,
    };
    ot_poll_exchange(&ex);
}

static bool pick_next_read(uint8_t *out_id, uint32_t now_ms, uint8_t *rr)
{
    if (!s_cat_ref) {
        return false;
    }
    /* Promoted */
    for (int i = 0; i < OT_CATALOG_MAX_IDS; i++) {
        if (s_promote_until[i] > now_ms && s_cat_ref->ids[i].support == OT_SUPPORT_AVAILABLE && i != 0) {
            *out_id = (uint8_t)i;
            return true;
        }
    }
    /* Fast RR */
    for (int n = 0; n < OT_CATALOG_MAX_IDS; n++) {
        uint8_t id = (uint8_t)((*rr + n) % OT_CATALOG_MAX_IDS);
        if (id == 0) {
            continue;
        }
        if (s_cat_ref->ids[id].support == OT_SUPPORT_AVAILABLE &&
            s_cat_ref->ids[id].poll_tier == OT_TIER_FAST) {
            *rr = (uint8_t)((id + 1) % OT_CATALOG_MAX_IDS);
            *out_id = id;
            return true;
        }
    }
    /* Slow due — simple: any slow available (budgeted externally) */
    static uint8_t slow_rr;
    for (int n = 0; n < OT_CATALOG_MAX_IDS; n++) {
        uint8_t id = (uint8_t)((slow_rr + n) % OT_CATALOG_MAX_IDS);
        if (id == 0) {
            continue;
        }
        if (s_cat_ref->ids[id].support == OT_SUPPORT_AVAILABLE &&
            s_cat_ref->ids[id].poll_tier == OT_TIER_SLOW) {
            slow_rr = (uint8_t)((id + 1) % OT_CATALOG_MAX_IDS);
            *out_id = id;
            return true;
        }
    }
    return false;
}

static void poll_task(void *arg)
{
    (void)arg;
    uint8_t rr = 1;
    ESP_LOGI(TAG, "poll task started");
    while (1) {
        TickType_t tick_start = xTaskGetTickCount();
        uint32_t now_ms = tick_start * portTICK_PERIOD_MS;

        /* Keepalive first — never dropped */
        do_status_keepalive();
        apply_hold_ch();

        /* One serialized write slot if pending */
        write_req_t wr;
        if (xQueueReceive(s_write_q, &wr, 0) == pdTRUE) {
            if (wr.data_id == 0) {
                /* Should not WRITE-DATA id 0 — flags already in s_master_hb */
                ESP_LOGW(TAG, "ignored WRITE enqueue for ID 0");
            } else {
                ot_exchange_t ex = {
                    .data_id = wr.data_id,
                    .request_value = wr.raw,
                    .is_write = true,
                };
                ot_poll_exchange(&ex);
                ot_poll_promote(wr.data_id);
            }
        }

        /* Time-budgeted reads */
        TickType_t deadline = tick_start + pdMS_TO_TICKS(APP_OT_KEEPALIVE_INTERVAL_MS - 50);
        while (xTaskGetTickCount() < deadline) {
            uint8_t id;
            if (!pick_next_read(&id, now_ms, &rr)) {
                break;
            }
            ot_exchange_t ex = {
                .data_id = id,
                .request_value = 0,
                .is_write = false,
            };
            ot_exchange_result_t r = ot_poll_exchange(&ex);
            if (s_cat_ref && (r == OT_EXCHANGE_OK || r == OT_EXCHANGE_INVALID)) {
                if (r == OT_EXCHANGE_OK) {
                    if (!s_cat_ref->ids[id].has_raw || s_cat_ref->ids[id].last_raw != ex.response_value) {
                        ot_poll_promote(id);
                    }
                    s_cat_ref->ids[id].has_raw = true;
                    s_cat_ref->ids[id].last_raw = ex.response_value;
                }
            }
            if (xTaskGetTickCount() >= deadline) {
                break;
            }
        }

        TickType_t elapsed = xTaskGetTickCount() - tick_start;
        if (elapsed < pdMS_TO_TICKS(APP_OT_KEEPALIVE_INTERVAL_MS)) {
            vTaskDelay(pdMS_TO_TICKS(APP_OT_KEEPALIVE_INTERVAL_MS) - elapsed);
        }
    }
}

esp_err_t ot_poll_start(void)
{
    if (!s_inited) {
        esp_err_t e = ot_poll_init();
        if (e != ESP_OK) {
            return e;
        }
    }
    if (s_task) {
        return ESP_OK;
    }
    BaseType_t ok = xTaskCreate(poll_task, "ot_poll", 4096, NULL, 5, &s_task);
    return ok == pdPASS ? ESP_OK : ESP_ERR_NO_MEM;
}
