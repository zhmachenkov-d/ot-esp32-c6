#include "failsafe.h"

#include <string.h>

esp_err_t failsafe_init(failsafe_state_t *st, uint32_t entry_timer_ms, uint32_t debounce_ms)
{
    if (!st) {
        return ESP_ERR_INVALID_ARG;
    }
    memset(st, 0, sizeof(*st));
    st->phase = FAILSAFE_IDLE;
    st->remote_writes_allowed = true;
    st->entry_timer_ms = entry_timer_ms;
    st->link_up_debounce_ms = debounce_ms;
    return ESP_OK;
}

void failsafe_on_link(failsafe_state_t *st, bool wifi_up, bool mqtt_up, uint32_t now_ms)
{
    if (!st) {
        return;
    }
    const bool link_up = wifi_up && mqtt_up;

    if (!link_up) {
        st->link_up_since_ms = 0;
        if (st->phase == FAILSAFE_IDLE) {
            st->phase = FAILSAFE_ENTRY_TIMER;
            st->timer_started_ms = now_ms;
            st->remote_writes_allowed = true;
        } else if (st->phase == FAILSAFE_ENTRY_TIMER) {
            uint32_t elapsed = now_ms - st->timer_started_ms;
            if (elapsed >= st->entry_timer_ms) {
                st->phase = FAILSAFE_ACTIVE;
                st->remote_writes_allowed = false;
            }
        }
        /* ACTIVE stays active until recovery debounce */
        return;
    }

    /* Link up */
    if (st->phase == FAILSAFE_ENTRY_TIMER) {
        /* Recover before expiry — cancel timer */
        st->phase = FAILSAFE_IDLE;
        st->remote_writes_allowed = true;
        st->timer_started_ms = 0;
        st->link_up_since_ms = 0;
        return;
    }

    if (st->phase == FAILSAFE_ACTIVE) {
        if (st->link_up_since_ms == 0) {
            st->link_up_since_ms = now_ms;
        }
        if ((now_ms - st->link_up_since_ms) >= st->link_up_debounce_ms) {
            st->phase = FAILSAFE_IDLE;
            st->remote_writes_allowed = true;
            st->link_up_since_ms = 0;
        }
        return;
    }

    /* IDLE + link up */
    st->link_up_since_ms = 0;
    st->remote_writes_allowed = true;
}

bool failsafe_is_active(const failsafe_state_t *st)
{
    return st && st->phase == FAILSAFE_ACTIVE;
}

bool failsafe_remote_writes_allowed(const failsafe_state_t *st)
{
    return st && st->remote_writes_allowed;
}

bool failsafe_app_availability_online(const failsafe_state_t *st)
{
    if (!st) {
        return false;
    }
    /* Option A: online during IDLE and ENTRY_TIMER; offline when ACTIVE */
    return st->phase != FAILSAFE_ACTIVE;
}

void failsafe_set_held_ch(failsafe_state_t *st, float celsius)
{
    if (!st) {
        return;
    }
    st->has_held_ch = true;
    st->held_ch_setpoint_c = celsius;
}
