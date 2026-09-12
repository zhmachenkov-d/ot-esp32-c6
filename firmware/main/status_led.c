#include "status_led.h"

#include "app_config.h"

#ifndef HOST_TEST
#include "esp_log.h"
#include "esp_timer.h"
#include "led_strip.h"
#endif

#define STATUS_LED_BRIGHTNESS_NUM   38  /* ≈15% of 255 */
#define STATUS_LED_BRIGHTNESS_DEN   255
#define STATUS_LED_SLOW_PERIOD_MS   1000
#define STATUS_LED_FAST_PERIOD_MS   250
#define STATUS_LED_REFRESH_US       (25000) /* 40 Hz — fine for 4 Hz blink */

static bool s_inited;

bool status_led_phase_on(status_led_pattern_t pattern, uint32_t now_ms)
{
    uint32_t period_ms;
    switch (pattern) {
    case STATUS_LED_PATTERN_SOLID:
        return true;
    case STATUS_LED_PATTERN_SLOW:
        period_ms = STATUS_LED_SLOW_PERIOD_MS;
        break;
    case STATUS_LED_PATTERN_FAST:
        period_ms = STATUS_LED_FAST_PERIOD_MS;
        break;
    default:
        return true;
    }
    return (now_ms % period_ms) < (period_ms / 2);
}

#ifndef HOST_TEST

static const char *TAG = "status_led";

static led_strip_handle_t s_strip;
static esp_timer_handle_t s_timer;
static uint8_t s_r = 255;
static uint8_t s_g = 255;
static uint8_t s_b = 255;
static status_led_pattern_t s_pattern = STATUS_LED_PATTERN_SOLID;
static bool s_lit;

static uint8_t dim_channel(uint8_t v)
{
    return (uint8_t)((uint16_t)v * STATUS_LED_BRIGHTNESS_NUM / STATUS_LED_BRIGHTNESS_DEN);
}

static void apply_output(bool on)
{
    if (!s_strip) {
        return;
    }
    esp_err_t err;
    if (on) {
        err = led_strip_set_pixel(s_strip, 0, dim_channel(s_r), dim_channel(s_g), dim_channel(s_b));
        if (err != ESP_OK) {
            return;
        }
        err = led_strip_refresh(s_strip);
    } else {
        err = led_strip_clear(s_strip);
    }
    if (err == ESP_OK) {
        s_lit = on;
    }
}

static void refresh_cb(void *arg)
{
    (void)arg;
    uint32_t now_ms = (uint32_t)(esp_timer_get_time() / 1000ULL);
    bool on = status_led_phase_on(s_pattern, now_ms);
    if (on == s_lit) {
        return;
    }
    apply_output(on);
}

/** Create strip + timer and show dim white solid. Not idempotent. */
static esp_err_t status_led_hw_init(void)
{
    led_strip_config_t strip_config = {
        .strip_gpio_num = APP_STATUS_LED_GPIO,
        .max_leds = 1,
        .led_pixel_format = LED_PIXEL_FORMAT_GRB,
        .led_model = LED_MODEL_WS2812,
        .flags = {
            .invert_out = false,
        },
    };
    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000,
        .flags = {
            .with_dma = false,
        },
    };

    esp_err_t err = led_strip_new_rmt_device(&strip_config, &rmt_config, &s_strip);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "led_strip create failed: %s", esp_err_to_name(err));
        return err;
    }

    const esp_timer_create_args_t timer_args = {
        .callback = refresh_cb,
        .name = "status_led",
    };
    err = esp_timer_create(&timer_args, &s_timer);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "timer create failed: %s", esp_err_to_name(err));
        (void)led_strip_del(s_strip);
        s_strip = NULL;
        return err;
    }

    s_r = 255;
    s_g = 255;
    s_b = 255;
    s_pattern = STATUS_LED_PATTERN_SOLID;
    s_lit = false;
    apply_output(true);

    err = esp_timer_start_periodic(s_timer, STATUS_LED_REFRESH_US);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "timer start failed: %s", esp_err_to_name(err));
        apply_output(false);
        (void)esp_timer_delete(s_timer);
        s_timer = NULL;
        (void)led_strip_del(s_strip);
        s_strip = NULL;
        return err;
    }

    ESP_LOGI(TAG, "WS2812 on GPIO %d (dim white solid)", APP_STATUS_LED_GPIO);
    return ESP_OK;
}

void status_led_set_rgb(uint8_t r, uint8_t g, uint8_t b)
{
    if (r == s_r && g == s_g && b == s_b) {
        return;
    }
    s_r = r;
    s_g = g;
    s_b = b;
    if (s_inited && s_lit) {
        apply_output(true);
    }
}

void status_led_set_pattern(status_led_pattern_t pattern)
{
    if (pattern == s_pattern) {
        return;
    }
    s_pattern = pattern;
    if (s_inited) {
        /* Force re-evaluate so SOLID↔blink transitions apply immediately. */
        s_lit = !status_led_phase_on(s_pattern, (uint32_t)(esp_timer_get_time() / 1000ULL));
        refresh_cb(NULL);
    }
}

#else /* HOST_TEST */

/** Host stub: succeeds once; second call fails (would be a leak without the gate). */
static esp_err_t status_led_hw_init(void)
{
    static bool s_hw_claimed;
    if (s_hw_claimed) {
        return ESP_ERR_INVALID_STATE;
    }
    s_hw_claimed = true;
    return ESP_OK;
}

void status_led_set_rgb(uint8_t r, uint8_t g, uint8_t b)
{
    (void)r;
    (void)g;
    (void)b;
}

void status_led_set_pattern(status_led_pattern_t pattern)
{
    (void)pattern;
}

#endif /* HOST_TEST */

esp_err_t status_led_init(void)
{
    if (s_inited) {
        return ESP_OK;
    }
    esp_err_t err = status_led_hw_init();
    if (err != ESP_OK) {
        return err;
    }
    s_inited = true;
    return ESP_OK;
}
