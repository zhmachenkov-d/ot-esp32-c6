#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    FAILSAFE_IDLE = 0,
    FAILSAFE_ENTRY_TIMER,
    FAILSAFE_ACTIVE,
} failsafe_phase_t;

typedef struct failsafe_state {
    failsafe_phase_t phase;
    bool remote_writes_allowed;
    bool has_held_ch;
    float held_ch_setpoint_c;
    uint32_t entry_timer_ms;
    uint32_t link_up_debounce_ms;
    uint32_t timer_started_ms;
    uint32_t link_up_since_ms;
} failsafe_state_t;

esp_err_t failsafe_init(failsafe_state_t *st, uint32_t entry_timer_ms, uint32_t debounce_ms);

/**
 * Notify link health. Call periodically with monotonic now_ms.
 * wifi_up && mqtt_up = combined healthy.
 */
void failsafe_on_link(failsafe_state_t *st, bool wifi_up, bool mqtt_up, uint32_t now_ms);

bool failsafe_is_active(const failsafe_state_t *st);
bool failsafe_remote_writes_allowed(const failsafe_state_t *st);

/** Option A: during ENTRY_TIMER treat availability as online; ACTIVE → offline. */
bool failsafe_app_availability_online(const failsafe_state_t *st);

void failsafe_set_held_ch(failsafe_state_t *st, float celsius);

#ifdef __cplusplus
}
#endif
