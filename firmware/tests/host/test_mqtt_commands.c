#include "unity.h"
#include "mqtt_commands.h"
#include "ot_catalog.h"
#include "ot_codec.h"
#include "ot_poll.h"

#include <string.h>

/* Declared in host_ot_poll_stub.c */
void host_ot_poll_stub_set_write_result(ot_exchange_result_t r);
void host_ot_poll_stub_set_auto_complete(bool enable);
void host_ot_poll_stub_set_enqueue(bool ok);
void host_ot_poll_stub_set_status_result(ot_exchange_result_t r, uint16_t raw);
void host_ot_poll_stub_set_auto_status_complete(bool enable);
void host_ot_poll_stub_fire_status_complete(void);

void host_mqtt_ha_reset_subscribes(void);
int host_mqtt_ha_count_subscribe_substr(const char *substr);
void host_mqtt_ha_inject_message(const char *topic, const char *payload, bool retain);
void host_ota_install_reset(void);
int host_ota_install_call_count(void);
const char *host_ota_install_last_payload(void);

static ot_catalog_t s_cat;

void setUp(void)
{
    ot_catalog_init(&s_cat);
    s_cat.ids[1].support = OT_SUPPORT_AVAILABLE;
    s_cat.ids[1].readable = true;
    s_cat.ids[1].writable = true;
    s_cat.ids[0].support = OT_SUPPORT_AVAILABLE;
    s_cat.ids[0].writable = true;
    s_cat.ids[14].support = OT_SUPPORT_AVAILABLE;
    s_cat.ids[14].writable = true;
    host_ot_poll_stub_set_enqueue(true);
    host_ot_poll_stub_set_auto_complete(false);
    host_ot_poll_stub_set_write_result(OT_EXCHANGE_OK);
    host_ot_poll_stub_set_auto_status_complete(false);
    host_ot_poll_stub_set_status_result(OT_EXCHANGE_OK, 0x0100);
    host_mqtt_ha_reset_subscribes();
    host_ota_install_reset();
    mqtt_commands_set_time_ms(0);
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

void test_queue_in_range_id1(void)
{
    writable_command_t cmd;
    writable_cmd_outcome_t o = mqtt_commands_handle(1, "45.0", true, &cmd);
    TEST_ASSERT_EQUAL(WRITABLE_CMD_QUEUED, o);
}

void test_accept_only_after_ot_ok(void)
{
    writable_command_t cmd;
    TEST_ASSERT_EQUAL(WRITABLE_CMD_QUEUED, mqtt_commands_handle(1, "45.0", true, &cmd));
    writable_cmd_outcome_t o = mqtt_commands_on_write_complete(1, OT_EXCHANGE_OK, &cmd);
    TEST_ASSERT_EQUAL(WRITABLE_CMD_ACCEPTED, o);
}

void test_ot_failed_leaves_unaccepted(void)
{
    writable_command_t cmd;
    TEST_ASSERT_EQUAL(WRITABLE_CMD_QUEUED, mqtt_commands_handle(1, "40.0", true, &cmd));
    writable_cmd_outcome_t o = mqtt_commands_on_write_complete(1, OT_EXCHANGE_TIMEOUT, &cmd);
    TEST_ASSERT_EQUAL(WRITABLE_CMD_OT_FAILED, o);
}

void test_reject_failsafe(void)
{
    writable_command_t cmd;
    writable_cmd_outcome_t o = mqtt_commands_handle(1, "45.0", false, &cmd);
    TEST_ASSERT_EQUAL(WRITABLE_CMD_REJECTED_FAILSAFE, o);
}

void test_unhealthy_still_attempts(void)
{
    /* boiler-link unhealthy does not pre-reject — still queues if enqueue works */
    writable_command_t cmd;
    writable_cmd_outcome_t o = mqtt_commands_handle(1, "40.0", true, &cmd);
    TEST_ASSERT_EQUAL(WRITABLE_CMD_QUEUED, o);
}

void test_id0_ch_enable_queued_until_status(void)
{
    writable_command_t cmd;
    writable_cmd_outcome_t o = mqtt_commands_handle(0, "1", true, &cmd);
    TEST_ASSERT_EQUAL(WRITABLE_CMD_QUEUED, o);
    TEST_ASSERT_TRUE((ot_poll_get_master_status_flags() & 0x01) != 0);

    host_ot_poll_stub_set_status_result(OT_EXCHANGE_OK, 0x010A);
    o = mqtt_commands_on_status_complete(OT_EXCHANGE_OK, 0x010A, &cmd);
    TEST_ASSERT_EQUAL(WRITABLE_CMD_ACCEPTED, o);
    TEST_ASSERT_TRUE(s_cat.ids[0].has_raw);
    TEST_ASSERT_EQUAL_UINT16(0x010A, s_cat.ids[0].last_raw);
}

void test_id0_ch_enable_ot_failed_reverts(void)
{
    ot_poll_set_master_status_flags(0x00);
    writable_command_t cmd;
    TEST_ASSERT_EQUAL(WRITABLE_CMD_QUEUED, mqtt_commands_handle(0, "1", true, &cmd));
    TEST_ASSERT_TRUE((ot_poll_get_master_status_flags() & 0x01) != 0);

    writable_cmd_outcome_t o =
        mqtt_commands_on_status_complete(OT_EXCHANGE_TIMEOUT, 0, &cmd);
    TEST_ASSERT_EQUAL(WRITABLE_CMD_OT_FAILED, o);
    TEST_ASSERT_EQUAL_UINT8(0x00, ot_poll_get_master_status_flags());
}

void test_post_recovery_retained_policy(void)
{
    mqtt_commands_set_time_ms(0);
    mqtt_commands_begin_post_recovery();
    /* Before 2 s debounce: drop all retained */
    TEST_ASSERT_FALSE(mqtt_commands_allow_inbound(1, true));
    TEST_ASSERT_FALSE(mqtt_commands_allow_inbound(14, true));

    mqtt_commands_set_time_ms(2000);
    TEST_ASSERT_FALSE(mqtt_commands_allow_inbound(14, true));
    TEST_ASSERT_TRUE(mqtt_commands_allow_inbound(1, true));
    TEST_ASSERT_FALSE(mqtt_commands_allow_inbound(1, true)); /* at most one */
    TEST_ASSERT_TRUE(mqtt_commands_allow_inbound(14, false)); /* live ends gate */
    TEST_ASSERT_TRUE(mqtt_commands_allow_inbound(14, true));  /* gate cleared */
}

void test_reconnect_rearms_retained_gate(void)
{
    mqtt_commands_set_time_ms(0);
    mqtt_commands_begin_post_recovery();
    mqtt_commands_set_time_ms(2000);
    TEST_ASSERT_TRUE(mqtt_commands_allow_inbound(1, true));

    /* Plain MQTT reconnect arms again — another single retained ID 1 allowed */
    mqtt_commands_set_time_ms(5000);
    mqtt_commands_begin_post_recovery();
    TEST_ASSERT_FALSE(mqtt_commands_allow_inbound(1, true)); /* debounce again */
    mqtt_commands_set_time_ms(7000);
    TEST_ASSERT_TRUE(mqtt_commands_allow_inbound(1, true));
}

void test_subscribe_includes_update_set(void)
{
    host_mqtt_ha_reset_subscribes();
    TEST_ASSERT_EQUAL(ESP_OK, mqtt_commands_start_subscriptions());
    TEST_ASSERT_EQUAL(1, host_mqtt_ha_count_subscribe_substr("otc6/aabbccddeeff/update/set"));
}

void test_install_non_retained_invokes_handler(void)
{
    host_ota_install_reset();
    host_mqtt_ha_inject_message("otc6/aabbccddeeff/update/set", "install", false);
    TEST_ASSERT_EQUAL(1, host_ota_install_call_count());
    TEST_ASSERT_EQUAL_STRING("install", host_ota_install_last_payload());
}

void test_install_retained_ignored(void)
{
    host_ota_install_reset();
    host_mqtt_ha_inject_message("otc6/aabbccddeeff/update/set", "install", true);
    TEST_ASSERT_EQUAL(0, host_ota_install_call_count());
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_reason_mapping);
    RUN_TEST(test_reject_range_id1);
    RUN_TEST(test_queue_in_range_id1);
    RUN_TEST(test_accept_only_after_ot_ok);
    RUN_TEST(test_ot_failed_leaves_unaccepted);
    RUN_TEST(test_reject_failsafe);
    RUN_TEST(test_unhealthy_still_attempts);
    RUN_TEST(test_id0_ch_enable_queued_until_status);
    RUN_TEST(test_id0_ch_enable_ot_failed_reverts);
    RUN_TEST(test_post_recovery_retained_policy);
    RUN_TEST(test_reconnect_rearms_retained_gate);
    RUN_TEST(test_subscribe_includes_update_set);
    RUN_TEST(test_install_non_retained_invokes_handler);
    RUN_TEST(test_install_retained_ignored);
    return UNITY_END();
}
