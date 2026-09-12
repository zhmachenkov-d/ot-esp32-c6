#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "status_led.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Caller-supplied bind flags; no firmware file-static reads. */
typedef struct {
    bool softap_active;
    bool failsafe_active;
    bool provision_boot_run_no_mqtt;
    bool ota_failed;
    bool sta_got_ip;
    bool mqtt_connected;
    bool mqtt_session_ready;
    bool ot_healthy;
} status_led_ladder_input_t;

/** Pre-dim logical RGB + pattern. */
typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    status_led_pattern_t pattern;
} status_led_ladder_result_t;

/**
 * Pure SoftAP → critical → else/highest-wins ladder mapping.
 * Side-effect free; no static rung state.
 */
status_led_ladder_result_t status_led_ladder_eval(const status_led_ladder_input_t *in);

#ifdef __cplusplus
}
#endif
