#include "status_led_ladder.h"

static status_led_ladder_result_t result(uint8_t r, uint8_t g, uint8_t b,
                                         status_led_pattern_t pattern)
{
    status_led_ladder_result_t out = {
        .r = r,
        .g = g,
        .b = b,
        .pattern = pattern,
    };
    return out;
}

status_led_ladder_result_t status_led_ladder_eval(const status_led_ladder_input_t *in)
{
    if (in->softap_active) {
        return result(255, 0, 255, STATUS_LED_PATTERN_SLOW); /* magenta */
    }

    if (in->failsafe_active || in->provision_boot_run_no_mqtt || in->ota_failed) {
        return result(255, 0, 0, STATUS_LED_PATTERN_FAST); /* red */
    }

    /* Else/highest-wins: rung 6 → 1 */
    if (in->mqtt_session_ready && in->ot_healthy) {
        return result(0, 200, 0, STATUS_LED_PATTERN_SOLID); /* green */
    }
    if (in->mqtt_session_ready && !in->ot_healthy) {
        return result(160, 0, 255, STATUS_LED_PATTERN_SOLID); /* purple */
    }
    if (!in->mqtt_session_ready && in->ot_healthy) {
        return result(0, 64, 255, STATUS_LED_PATTERN_SOLID); /* blue */
    }
    if (in->mqtt_connected && !in->mqtt_session_ready && !in->ot_healthy) {
        return result(0, 200, 200, STATUS_LED_PATTERN_SLOW); /* cyan */
    }
    if (in->sta_got_ip && !in->mqtt_connected) {
        return result(255, 200, 0, STATUS_LED_PATTERN_SLOW); /* yellow */
    }

    return result(255, 96, 0, STATUS_LED_PATTERN_SLOW); /* orange */
}
