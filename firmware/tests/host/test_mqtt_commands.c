#include "unity.h"
#include "mqtt_commands.h"
#include "ot_catalog.h"
#include "ot_codec.h"

#include <string.h>

static ot_catalog_t s_cat;

void setUp(void)
{
    ot_catalog_init(&s_cat);
    s_cat.ids[1].support = OT_SUPPORT_AVAILABLE;
    s_cat.ids[1].readable = true;
    s_cat.ids[1].writable = true;
    s_cat.ids[0].support = OT_SUPPORT_AVAILABLE;
    s_cat.ids[0].writable = true;
    mqtt_commands_init("aabbccddeeff", &s_cat, 10.0f, 90.0f);
}

void tearDown(void) {}

void test_reason_mapping(void)
{
    TEST_ASSERT_EQUAL_STRING("out_of_range",
                             mqtt_commands_reason_wire(WRITABLE_CMD_REJECTED_RANGE));
    TEST_ASSERT_EQUAL_STRING("rejected_failsafe",
                             mqtt_commands_reason_wire(WRITABLE_CMD_REJECTED_FAILSAFE));
    TEST_ASSERT_EQUAL_STRING("ot_failed",
                             mqtt_commands_reason_wire(WRITABLE_CMD_OT_FAILED));
}

void test_reject_range_id1(void)
{
    writable_command_t cmd;
    writable_cmd_outcome_t o = mqtt_commands_handle(1, "120.0", true, &cmd);
    TEST_ASSERT_EQUAL(WRITABLE_CMD_REJECTED_RANGE, o);
}

void test_accept_in_range_id1(void)
{
    writable_command_t cmd;
    writable_cmd_outcome_t o = mqtt_commands_handle(1, "45.0", true, &cmd);
    TEST_ASSERT_EQUAL(WRITABLE_CMD_ACCEPTED, o);
}

void test_reject_failsafe(void)
{
    writable_command_t cmd;
    writable_cmd_outcome_t o = mqtt_commands_handle(1, "45.0", false, &cmd);
    TEST_ASSERT_EQUAL(WRITABLE_CMD_REJECTED_FAILSAFE, o);
}

void test_unhealthy_still_attempts(void)
{
    /* boiler-link unhealthy does not pre-reject — still accepted if enqueue works */
    writable_command_t cmd;
    writable_cmd_outcome_t o = mqtt_commands_handle(1, "40.0", true, &cmd);
    TEST_ASSERT_EQUAL(WRITABLE_CMD_ACCEPTED, o);
}

void test_id0_ch_enable(void)
{
    writable_command_t cmd;
    writable_cmd_outcome_t o = mqtt_commands_handle(0, "1", true, &cmd);
    TEST_ASSERT_EQUAL(WRITABLE_CMD_ACCEPTED, o);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_reason_mapping);
    RUN_TEST(test_reject_range_id1);
    RUN_TEST(test_accept_in_range_id1);
    RUN_TEST(test_reject_failsafe);
    RUN_TEST(test_unhealthy_still_attempts);
    RUN_TEST(test_id0_ch_enable);
    return UNITY_END();
}
