#include "unity.h"
#include "mqtt_discovery.h"
#include "ot_codec.h"

#include <string.h>

void setUp(void) {}
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

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_status_flag_binary_config);
    RUN_TEST(test_slave_flag_bits_match_knowledge);
    return UNITY_END();
}
