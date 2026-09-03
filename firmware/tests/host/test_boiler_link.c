#include "unity.h"
#include "app_config.h"
#include "ot_poll.h"

void setUp(void) {}
void tearDown(void) {}

void test_unhealthy_after_three_keepalive_fails(void)
{
    ot_boiler_link_fsm_t fsm = { .consecutive_fails = 0, .healthy = true };

    TEST_ASSERT_FALSE(ot_boiler_link_fsm_note(&fsm, OT_EXCHANGE_TIMEOUT,
                                              APP_BOILER_LINK_FAIL_THRESHOLD));
    TEST_ASSERT_TRUE(fsm.healthy);
    TEST_ASSERT_EQUAL(1, fsm.consecutive_fails);

    TEST_ASSERT_FALSE(ot_boiler_link_fsm_note(&fsm, OT_EXCHANGE_ERROR,
                                              APP_BOILER_LINK_FAIL_THRESHOLD));
    TEST_ASSERT_TRUE(fsm.healthy);
    TEST_ASSERT_EQUAL(2, fsm.consecutive_fails);

    TEST_ASSERT_TRUE(ot_boiler_link_fsm_note(&fsm, OT_EXCHANGE_TIMEOUT,
                                             APP_BOILER_LINK_FAIL_THRESHOLD));
    TEST_ASSERT_FALSE(fsm.healthy);
    TEST_ASSERT_EQUAL(3, fsm.consecutive_fails);
}

void test_healthy_after_one_success(void)
{
    ot_boiler_link_fsm_t fsm = { .consecutive_fails = 3, .healthy = false };

    TEST_ASSERT_TRUE(ot_boiler_link_fsm_note(&fsm, OT_EXCHANGE_OK,
                                             APP_BOILER_LINK_FAIL_THRESHOLD));
    TEST_ASSERT_TRUE(fsm.healthy);
    TEST_ASSERT_EQUAL(0, fsm.consecutive_fails);
}

void test_data_invalid_counts_as_keepalive_success(void)
{
    ot_boiler_link_fsm_t fsm = { .consecutive_fails = 2, .healthy = true };

    TEST_ASSERT_FALSE(ot_boiler_link_fsm_note(&fsm, OT_EXCHANGE_INVALID,
                                              APP_BOILER_LINK_FAIL_THRESHOLD));
    TEST_ASSERT_TRUE(fsm.healthy);
    TEST_ASSERT_EQUAL(0, fsm.consecutive_fails);
}

void test_two_fails_then_success_does_not_go_unhealthy(void)
{
    ot_boiler_link_fsm_t fsm = { .consecutive_fails = 0, .healthy = true };

    ot_boiler_link_fsm_note(&fsm, OT_EXCHANGE_TIMEOUT, APP_BOILER_LINK_FAIL_THRESHOLD);
    ot_boiler_link_fsm_note(&fsm, OT_EXCHANGE_TIMEOUT, APP_BOILER_LINK_FAIL_THRESHOLD);
    TEST_ASSERT_TRUE(fsm.healthy);
    TEST_ASSERT_EQUAL(2, fsm.consecutive_fails);

    TEST_ASSERT_FALSE(ot_boiler_link_fsm_note(&fsm, OT_EXCHANGE_OK,
                                              APP_BOILER_LINK_FAIL_THRESHOLD));
    TEST_ASSERT_TRUE(fsm.healthy);
    TEST_ASSERT_EQUAL(0, fsm.consecutive_fails);
}

void test_already_unhealthy_stays_until_success(void)
{
    ot_boiler_link_fsm_t fsm = { .consecutive_fails = 5, .healthy = false };

    TEST_ASSERT_FALSE(ot_boiler_link_fsm_note(&fsm, OT_EXCHANGE_ERROR,
                                              APP_BOILER_LINK_FAIL_THRESHOLD));
    TEST_ASSERT_FALSE(fsm.healthy);

    TEST_ASSERT_TRUE(ot_boiler_link_fsm_note(&fsm, OT_EXCHANGE_OK,
                                             APP_BOILER_LINK_FAIL_THRESHOLD));
    TEST_ASSERT_TRUE(fsm.healthy);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_unhealthy_after_three_keepalive_fails);
    RUN_TEST(test_healthy_after_one_success);
    RUN_TEST(test_data_invalid_counts_as_keepalive_success);
    RUN_TEST(test_two_fails_then_success_does_not_go_unhealthy);
    RUN_TEST(test_already_unhealthy_stays_until_success);
    return UNITY_END();
}
