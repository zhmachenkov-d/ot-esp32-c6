#include "ot_poll.h"

#include <string.h>

static uint8_t s_master_hb;
static uint8_t s_slave_lb = 0x0A; /* flame+CH for fixtures */
static bool s_healthy = true;
static ot_exchange_result_t s_next = OT_EXCHANGE_OK;
static uint16_t s_next_raw;
static ot_write_complete_cb_t s_write_cb;
static void *s_write_cb_ctx;
static bool s_enqueue_ok = true;
static ot_exchange_result_t s_write_result = OT_EXCHANGE_OK;
static bool s_auto_complete;

void host_ot_poll_stub_set_response(ot_exchange_result_t r, uint16_t raw)
{
    s_next = r;
    s_next_raw = raw;
}

void host_ot_poll_stub_set_enqueue(bool ok)
{
    s_enqueue_ok = ok;
}

void host_ot_poll_stub_set_write_result(ot_exchange_result_t r)
{
    s_write_result = r;
}

void host_ot_poll_stub_set_auto_complete(bool enable)
{
    s_auto_complete = enable;
}

esp_err_t ot_poll_init(void) { return ESP_OK; }
esp_err_t ot_poll_start(void) { return ESP_OK; }
void ot_poll_set_catalog(struct ot_catalog *cat) { (void)cat; }

void ot_poll_set_write_complete_cb(ot_write_complete_cb_t cb, void *ctx)
{
    s_write_cb = cb;
    s_write_cb_ctx = ctx;
}

ot_exchange_result_t ot_poll_exchange(ot_exchange_t *ex)
{
    if (!ex) {
        return OT_EXCHANGE_ERROR;
    }
    ex->response_value = s_next_raw;
    ex->result = s_next;
    return s_next;
}

bool ot_poll_enqueue_write(uint8_t data_id, uint16_t raw_value)
{
    if (!s_enqueue_ok) {
        return false;
    }
    if (s_auto_complete && s_write_cb) {
        s_write_cb(data_id, raw_value, s_write_result, s_write_cb_ctx);
    }
    return true;
}

void ot_poll_set_master_status_flags(uint8_t master_hb) { s_master_hb = master_hb; }
uint8_t ot_poll_get_master_status_flags(void) { return s_master_hb; }
uint8_t ot_poll_get_slave_status_flags(void) { return s_slave_lb; }
bool ot_poll_boiler_link_healthy(void) { return s_healthy; }
void ot_poll_set_hold_ch_setpoint(bool enable, float celsius)
{
    (void)enable;
    (void)celsius;
}
void ot_poll_promote(uint8_t data_id) { (void)data_id; }
