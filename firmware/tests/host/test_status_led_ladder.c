#include "unity.h"
#include "status_led_ladder.h"

void setUp(void) {}
void tearDown(void) {}

static status_led_ladder_input_t clear_input(void)
{
    status_led_ladder_input_t in = {0};
    return in;
}

static void assert_rgb_pattern(const status_led_ladder_result_t *got,
                               uint8_t r, uint8_t g, uint8_t b,
                               status_led_pattern_t pattern)
{
    TEST_ASSERT_EQUAL_UINT8(r, got->r);
    TEST_ASSERT_EQUAL_UINT8(g, got->g);
    TEST_ASSERT_EQUAL_UINT8(b, got->b);
    TEST_ASSERT_EQUAL_INT(pattern, got->pattern);
}

void test_softap_only_magenta_slow(void)
{
    status_led_ladder_input_t in = clear_input();
    in.softap_active = true;

    status_led_ladder_result_t out = status_led_ladder_eval(&in);
    assert_rgb_pattern(&out, 255, 0, 255, STATUS_LED_PATTERN_SLOW);
}

void test_softap_wins_over_critical(void)
{
    status_led_ladder_input_t in = clear_input();
    in.softap_active = true;
    in.failsafe_active = true;
    in.ota_failed = true;
    in.mqtt_session_ready = true;
    in.ot_healthy = true;

    status_led_ladder_result_t out = status_led_ladder_eval(&in);
    assert_rgb_pattern(&out, 255, 0, 255, STATUS_LED_PATTERN_SLOW);
}

void test_critical_failsafe_only(void)
{
    status_led_ladder_input_t in = clear_input();
    in.failsafe_active = true;
    in.mqtt_session_ready = true;
    in.ot_healthy = true;

    status_led_ladder_result_t out = status_led_ladder_eval(&in);
    assert_rgb_pattern(&out, 255, 0, 0, STATUS_LED_PATTERN_FAST);
}

void test_critical_provision_boot_run_no_mqtt(void)
{
    status_led_ladder_input_t in = clear_input();
    in.provision_boot_run_no_mqtt = true;
    in.sta_got_ip = true;

    status_led_ladder_result_t out = status_led_ladder_eval(&in);
    assert_rgb_pattern(&out, 255, 0, 0, STATUS_LED_PATTERN_FAST);
}

void test_critical_ota_failed(void)
{
    status_led_ladder_input_t in = clear_input();
    in.ota_failed = true;
    in.mqtt_connected = true;

    status_led_ladder_result_t out = status_led_ladder_eval(&in);
    assert_rgb_pattern(&out, 255, 0, 0, STATUS_LED_PATTERN_FAST);
}

void test_green_session_ready_ot_healthy(void)
{
    status_led_ladder_input_t in = clear_input();
    in.sta_got_ip = true;
    in.mqtt_connected = true;
    in.mqtt_session_ready = true;
    in.ot_healthy = true;

    status_led_ladder_result_t out = status_led_ladder_eval(&in);
    assert_rgb_pattern(&out, 0, 200, 0, STATUS_LED_PATTERN_SOLID);
}

void test_purple_session_ready_ot_unhealthy(void)
{
    status_led_ladder_input_t in = clear_input();
    in.sta_got_ip = true;
    in.mqtt_connected = true;
    in.mqtt_session_ready = true;
    in.ot_healthy = false;

    status_led_ladder_result_t out = status_led_ladder_eval(&in);
    assert_rgb_pattern(&out, 160, 0, 255, STATUS_LED_PATTERN_SOLID);
}

void test_blue_ot_only_beats_orange(void)
{
    /* Session not ready + OT healthy; Wi-Fi down — blue still wins. */
    status_led_ladder_input_t in = clear_input();
    in.sta_got_ip = false;
    in.mqtt_connected = false;
    in.mqtt_session_ready = false;
    in.ot_healthy = true;

    status_led_ladder_result_t out = status_led_ladder_eval(&in);
    assert_rgb_pattern(&out, 0, 64, 255, STATUS_LED_PATTERN_SOLID);
}

void test_blue_beats_yellow_when_sta_ip_mqtt_down(void)
{
    status_led_ladder_input_t in = clear_input();
    in.sta_got_ip = true;
    in.mqtt_connected = false;
    in.mqtt_session_ready = false;
    in.ot_healthy = true;

    status_led_ladder_result_t out = status_led_ladder_eval(&in);
    assert_rgb_pattern(&out, 0, 64, 255, STATUS_LED_PATTERN_SOLID);
}

void test_green_beats_yellow_when_mqtt_connected_false(void)
{
    status_led_ladder_input_t in = clear_input();
    in.sta_got_ip = true;
    in.mqtt_connected = false;
    in.mqtt_session_ready = true;
    in.ot_healthy = true;

    status_led_ladder_result_t out = status_led_ladder_eval(&in);
    assert_rgb_pattern(&out, 0, 200, 0, STATUS_LED_PATTERN_SOLID);
}

void test_purple_beats_yellow_when_mqtt_connected_false(void)
{
    status_led_ladder_input_t in = clear_input();
    in.sta_got_ip = true;
    in.mqtt_connected = false;
    in.mqtt_session_ready = true;
    in.ot_healthy = false;

    status_led_ladder_result_t out = status_led_ladder_eval(&in);
    assert_rgb_pattern(&out, 160, 0, 255, STATUS_LED_PATTERN_SOLID);
}

void test_cyan_mqtt_up_session_not_ready_ot_down(void)
{
    status_led_ladder_input_t in = clear_input();
    in.sta_got_ip = true;
    in.mqtt_connected = true;
    in.mqtt_session_ready = false;
    in.ot_healthy = false;

    status_led_ladder_result_t out = status_led_ladder_eval(&in);
    assert_rgb_pattern(&out, 0, 200, 200, STATUS_LED_PATTERN_SLOW);
}

void test_yellow_sta_ip_mqtt_not_connected(void)
{
    status_led_ladder_input_t in = clear_input();
    in.sta_got_ip = true;
    in.mqtt_connected = false;
    in.mqtt_session_ready = false;
    in.ot_healthy = false;

    status_led_ladder_result_t out = status_led_ladder_eval(&in);
    assert_rgb_pattern(&out, 255, 200, 0, STATUS_LED_PATTERN_SLOW);
}

void test_orange_no_sta_ip(void)
{
    status_led_ladder_input_t in = clear_input();

    status_led_ladder_result_t out = status_led_ladder_eval(&in);
    assert_rgb_pattern(&out, 255, 96, 0, STATUS_LED_PATTERN_SLOW);
}

void test_no_sticky_high_water(void)
{
    status_led_ladder_input_t in = clear_input();
    in.sta_got_ip = true;
    in.mqtt_connected = true;
    in.mqtt_session_ready = true;
    in.ot_healthy = true;

    status_led_ladder_result_t high = status_led_ladder_eval(&in);
    assert_rgb_pattern(&high, 0, 200, 0, STATUS_LED_PATTERN_SOLID);

    in.mqtt_session_ready = false;
    in.ot_healthy = false;
    in.mqtt_connected = false;
    /* STA+IP remains → yellow, not sticky green */

    status_led_ladder_result_t low = status_led_ladder_eval(&in);
    assert_rgb_pattern(&low, 255, 200, 0, STATUS_LED_PATTERN_SLOW);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_softap_only_magenta_slow);
    RUN_TEST(test_softap_wins_over_critical);
    RUN_TEST(test_critical_failsafe_only);
    RUN_TEST(test_critical_provision_boot_run_no_mqtt);
    RUN_TEST(test_critical_ota_failed);
    RUN_TEST(test_green_session_ready_ot_healthy);
    RUN_TEST(test_purple_session_ready_ot_unhealthy);
    RUN_TEST(test_blue_ot_only_beats_orange);
    RUN_TEST(test_blue_beats_yellow_when_sta_ip_mqtt_down);
    RUN_TEST(test_green_beats_yellow_when_mqtt_connected_false);
    RUN_TEST(test_purple_beats_yellow_when_mqtt_connected_false);
    RUN_TEST(test_cyan_mqtt_up_session_not_ready_ot_down);
    RUN_TEST(test_yellow_sta_ip_mqtt_not_connected);
    RUN_TEST(test_orange_no_sta_ip);
    RUN_TEST(test_no_sticky_high_water);
    return UNITY_END();
}
