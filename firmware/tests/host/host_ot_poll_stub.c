#include "ot_poll.h"

#include <string.h>

static uint8_t s_master_hb;
static uint8_t s_slave_lb = 0x0A; /* flame+CH for fixtures */
static bool s_healthy = true;
static ot_exchange_result_t s_next = OT_EXCHANGE_OK;
static uint16_t s_next_raw;

void host_ot_poll_stub_set_response(ot_exchange_result_t r, uint16_t raw)
{
    s_next = r;
    s_next_raw = raw;
}

esp_err_t ot_poll_init(void) { return ESP_OK; }
esp_err_t ot_poll_start(void) { return ESP_OK; }
void ot_poll_set_catalog(struct ot_catalog *cat) { (void)cat; }

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
    (void)data_id;
    (void)raw_value;
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
