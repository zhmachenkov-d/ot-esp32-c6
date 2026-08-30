#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    OT_EXCHANGE_OK = 0,
    OT_EXCHANGE_TIMEOUT,
    OT_EXCHANGE_INVALID,
    OT_EXCHANGE_UNKNOWN_ID,
    OT_EXCHANGE_ERROR,
} ot_exchange_result_t;

typedef struct {
    uint8_t data_id;
    uint16_t request_value;
    uint16_t response_value;
    ot_exchange_result_t result;
    bool is_write;
} ot_exchange_t;

struct ot_catalog;

/** Init OpenTherm master on APP_OT_GPIO_IN/OUT (sazanof or Melnyk-port). */
esp_err_t ot_poll_init(void);

/** Single blocking exchange (read or write). Caller enforces inter-frame gap. */
ot_exchange_result_t ot_poll_exchange(ot_exchange_t *ex);

/** Optional catalog pointer for tiered reads (nullable until discovery). */
void ot_poll_set_catalog(struct ot_catalog *cat);

/** Start keepalive + tiered poll FreeRTOS task. */
esp_err_t ot_poll_start(void);

/** Enqueue a write (serialized; never drops keepalive). Returns false if queue full. */
bool ot_poll_enqueue_write(uint8_t data_id, uint16_t raw_value);

/**
 * ID 0 special: update pending master Status flags applied on next READ-DATA(id=0).
 * Never uses WRITE-DATA(id=0).
 */
void ot_poll_set_master_status_flags(uint8_t master_hb);

uint8_t ot_poll_get_master_status_flags(void);

/** Last slave status LB from keepalive (valid if boiler-link has succeeded). */
uint8_t ot_poll_get_slave_status_flags(void);

bool ot_poll_boiler_link_healthy(void);

/** Hold CH setpoint on the wire (fail-safe). */
void ot_poll_set_hold_ch_setpoint(bool enable, float celsius);

/** Promote Data ID to fast tier after write/change. */
void ot_poll_promote(uint8_t data_id);

#ifdef __cplusplus
}
#endif
