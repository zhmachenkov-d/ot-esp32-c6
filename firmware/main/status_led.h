#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    STATUS_LED_PATTERN_SOLID = 0,
    STATUS_LED_PATTERN_SLOW, /* 1 Hz, 50% duty */
    STATUS_LED_PATTERN_FAST, /* 4 Hz, 50% duty */
} status_led_pattern_t;

/**
 * Create the WS2812 strip on APP_STATUS_LED_GPIO, start driver-owned refresh,
 * and show dim white solid. Idempotent: second call returns ESP_OK without
 * reallocating.
 */
esp_err_t status_led_init(void);

/** Logical RGB before dim scale (~15%). */
void status_led_set_rgb(uint8_t r, uint8_t g, uint8_t b);

void status_led_set_pattern(status_led_pattern_t pattern);

/**
 * Pure blink phase helper (host-testable).
 * SOLID always on; SLOW/FAST use 50% duty over 1000 ms / 250 ms periods.
 */
bool status_led_phase_on(status_led_pattern_t pattern, uint32_t now_ms);

#ifdef __cplusplus
}
#endif
