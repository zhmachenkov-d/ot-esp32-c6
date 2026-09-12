#include "unity.h"
#include "status_led.h"

void setUp(void) {}
void tearDown(void) {}

void test_init_idempotent(void)
{
    TEST_ASSERT_EQUAL(ESP_OK, status_led_init());
    TEST_ASSERT_EQUAL(ESP_OK, status_led_init());
}

void test_solid_always_on(void)
{
    TEST_ASSERT_TRUE(status_led_phase_on(STATUS_LED_PATTERN_SOLID, 0));
    TEST_ASSERT_TRUE(status_led_phase_on(STATUS_LED_PATTERN_SOLID, 500));
    TEST_ASSERT_TRUE(status_led_phase_on(STATUS_LED_PATTERN_SOLID, 999));
}

void test_slow_1hz_50pct_over_1s(void)
{
    /* SLOW: period 1000 ms → on [0,500), off [500,1000) */
    int on_ms = 0;
    int off_ms = 0;
    for (uint32_t t = 0; t < 1000; t++) {
        if (status_led_phase_on(STATUS_LED_PATTERN_SLOW, t)) {
            on_ms++;
        } else {
            off_ms++;
        }
    }
    TEST_ASSERT_EQUAL_INT(500, on_ms);
    TEST_ASSERT_EQUAL_INT(500, off_ms);
    TEST_ASSERT_TRUE(status_led_phase_on(STATUS_LED_PATTERN_SLOW, 0));
    TEST_ASSERT_TRUE(status_led_phase_on(STATUS_LED_PATTERN_SLOW, 499));
    TEST_ASSERT_FALSE(status_led_phase_on(STATUS_LED_PATTERN_SLOW, 500));
    TEST_ASSERT_FALSE(status_led_phase_on(STATUS_LED_PATTERN_SLOW, 999));
}

void test_fast_4hz_50pct_over_1s(void)
{
    /* FAST: period 250 ms → on 125 / off 125 per cycle; 4 cycles in 1 s */
    int on_ms = 0;
    int off_ms = 0;
    for (uint32_t t = 0; t < 1000; t++) {
        if (status_led_phase_on(STATUS_LED_PATTERN_FAST, t)) {
            on_ms++;
        } else {
            off_ms++;
        }
    }
    TEST_ASSERT_EQUAL_INT(500, on_ms);
    TEST_ASSERT_EQUAL_INT(500, off_ms);
    TEST_ASSERT_TRUE(status_led_phase_on(STATUS_LED_PATTERN_FAST, 0));
    TEST_ASSERT_TRUE(status_led_phase_on(STATUS_LED_PATTERN_FAST, 124));
    TEST_ASSERT_FALSE(status_led_phase_on(STATUS_LED_PATTERN_FAST, 125));
    TEST_ASSERT_FALSE(status_led_phase_on(STATUS_LED_PATTERN_FAST, 249));
    TEST_ASSERT_TRUE(status_led_phase_on(STATUS_LED_PATTERN_FAST, 250));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_init_idempotent);
    RUN_TEST(test_solid_always_on);
    RUN_TEST(test_slow_1hz_50pct_over_1s);
    RUN_TEST(test_fast_4hz_50pct_over_1s);
    return UNITY_END();
}
