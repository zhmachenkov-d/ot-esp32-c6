#include "unity.h"
#include "mqtt_discovery.h"

#include <string.h>

void setUp(void) {}
void tearDown(void) {}

void test_device_block_shape(void)
{
    char buf[512];
    int n = mqtt_discovery_build_device_json(buf, sizeof(buf), "aabbccddeeff", "0.1.0");
    TEST_ASSERT_TRUE(n > 0);
    TEST_ASSERT_NOT_NULL(strstr(buf, "\"identifiers\":[\"otc6_aabbccddeeff\"]"));
    TEST_ASSERT_NOT_NULL(strstr(buf, "\"manufacturer\":\"ot-esp32-c6\""));
    TEST_ASSERT_NOT_NULL(strstr(buf, "\"sw_version\":\"0.1.0\""));
}

void test_boiler_link_config(void)
{
    char buf[1024];
    int n = mqtt_discovery_build_boiler_link_config(buf, sizeof(buf), "aabbccddeeff");
    TEST_ASSERT_TRUE(n > 0);
    TEST_ASSERT_NOT_NULL(strstr(buf, "boiler_link"));
    TEST_ASSERT_NOT_NULL(strstr(buf, "availability_topic"));
    TEST_ASSERT_NOT_NULL(strstr(buf, "payload_on\":\"healthy\""));
}

void test_sensor_config_per_id(void)
{
    char buf[1024];
    int n = mqtt_discovery_build_sensor_config(buf, sizeof(buf), "aabbccddeeff", 25,
                                              "OT 25 Tboiler", "°C", "temperature");
    TEST_ASSERT_TRUE(n > 0);
    TEST_ASSERT_NOT_NULL(strstr(buf, "ot/25/state"));
    TEST_ASSERT_NOT_NULL(strstr(buf, "unique_id\":\"otc6_aabbccddeeff_ot_25\""));
}

void test_number_config_range_checked_has_bounds(void)
{
    char buf[1024];
    int n = mqtt_discovery_build_number_config(buf, sizeof(buf), "aabbccddeeff", 1,
                                               "OT 1", true, 10.0f, 90.0f, 0.5f);
    TEST_ASSERT_TRUE(n > 0);
    TEST_ASSERT_NOT_NULL(strstr(buf, "\"min\":10.00"));
    TEST_ASSERT_NOT_NULL(strstr(buf, "\"max\":90.00"));
}

void test_number_config_non_range_omits_bounds(void)
{
    char buf[1024];
    int n = mqtt_discovery_build_number_config(buf, sizeof(buf), "aabbccddeeff", 9,
                                               "OT 9", false, 0.0f, 100.0f, 0.5f);
    TEST_ASSERT_TRUE(n > 0);
    TEST_ASSERT_NULL(strstr(buf, "\"min\":"));
    TEST_ASSERT_NULL(strstr(buf, "\"max\":"));
    TEST_ASSERT_NOT_NULL(strstr(buf, "command_topic"));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_device_block_shape);
    RUN_TEST(test_boiler_link_config);
    RUN_TEST(test_sensor_config_per_id);
    RUN_TEST(test_number_config_range_checked_has_bounds);
    RUN_TEST(test_number_config_non_range_omits_bounds);
    return UNITY_END();
}
