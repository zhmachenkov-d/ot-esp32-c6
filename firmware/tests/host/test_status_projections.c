#include "unity.h"
#include "mqtt_discovery.h"
#include "ot_codec.h"

#include <string.h>

/* Declared in stubs/mqtt_ha_stub.c */
void host_mqtt_ha_reset_publishes(void);
int host_mqtt_ha_publish_count(void);
int host_mqtt_ha_count_topic_substr(const char *substr);

void setUp(void)
{
    host_mqtt_ha_reset_publishes();
}
void tearDown(void) {}

void test_status_flag_binary_config(void)
{
    char buf[1024];
    int n = mqtt_discovery_build_status_flag_binary(buf, sizeof(buf), "deadbeefcafe",
                                                   "flame", "Flame",
                                                   "otc6/deadbeefcafe/status_flag/flame");
    TEST_ASSERT_TRUE(n > 0);
    TEST_ASSERT_NOT_NULL(strstr(buf, "status_flag/flame"));
    TEST_ASSERT_NOT_NULL(strstr(buf, "payload_on\":\"ON\""));
}

void test_slave_flag_bits_match_knowledge(void)
{
    /* knowledge/opentherm/data-id-0-status.md */
    uint8_t lb = 0;
    lb = ot_codec_flag8_set(lb, 0, true); /* fault */
    lb = ot_codec_flag8_set(lb, 1, true); /* CH active */
    lb = ot_codec_flag8_set(lb, 2, true); /* DHW */
    lb = ot_codec_flag8_set(lb, 3, true); /* flame */
    TEST_ASSERT_TRUE(ot_codec_flag8_get(lb, 0));
    TEST_ASSERT_TRUE(ot_codec_flag8_get(lb, 3));
    uint8_t hb = ot_codec_flag8_set(0, 0, true); /* CH enable */
    TEST_ASSERT_TRUE(ot_codec_flag8_get(hb, 0));
}

void test_status_flag_states_no_discovery_config(void)
{
    uint8_t slave_lb = ot_codec_flag8_set(0, 3, true); /* flame */
    uint8_t master_hb = ot_codec_flag8_set(0, 0, true); /* CH enable */

    TEST_ASSERT_EQUAL(ESP_OK,
                      mqtt_discovery_publish_status_flag_states("deadbeefcafe", master_hb, slave_lb));

    TEST_ASSERT_EQUAL(5, host_mqtt_ha_publish_count());
    TEST_ASSERT_EQUAL(5, host_mqtt_ha_count_topic_substr("status_flag/"));
    TEST_ASSERT_EQUAL(0, host_mqtt_ha_count_topic_substr("homeassistant/"));
    TEST_ASSERT_EQUAL(0, host_mqtt_ha_count_topic_substr("/config"));
}

void test_status_projections_publishes_discovery_config(void)
{
    uint8_t slave_lb = ot_codec_flag8_set(0, 3, true);
    uint8_t master_hb = ot_codec_flag8_set(0, 0, true);

    TEST_ASSERT_EQUAL(ESP_OK,
                      mqtt_discovery_publish_status_projections("deadbeefcafe", master_hb, slave_lb,
                                                                true));

    /* 5 state + 4 binary_sensor configs + 1 switch config */
    TEST_ASSERT_EQUAL(10, host_mqtt_ha_publish_count());
    TEST_ASSERT_EQUAL(5, host_mqtt_ha_count_topic_substr("status_flag/"));
    TEST_ASSERT_EQUAL(5, host_mqtt_ha_count_topic_substr("homeassistant/"));
    TEST_ASSERT_EQUAL(4, host_mqtt_ha_count_topic_substr("homeassistant/binary_sensor/"));
    TEST_ASSERT_EQUAL(1, host_mqtt_ha_count_topic_substr("homeassistant/switch/"));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_status_flag_binary_config);
    RUN_TEST(test_slave_flag_bits_match_knowledge);
    RUN_TEST(test_status_flag_states_no_discovery_config);
    RUN_TEST(test_status_projections_publishes_discovery_config);
    return UNITY_END();
}
