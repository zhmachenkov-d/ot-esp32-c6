#include "unity.h"
#include "failsafe.h"
#include "app_config.h"

void setUp(void) {}
void tearDown(void) {}

void test_entry_timer_keeps_online_and_writes(void)
{
    failsafe_state_t st;
    failsafe_init(&st, 10000, 2000);
    failsafe_on_link(&st, false, false, 0);
    TEST_ASSERT_EQUAL(FAILSAFE_ENTRY_TIMER, st.phase);
    TEST_ASSERT_TRUE(failsafe_remote_writes_allowed(&st));
    TEST_ASSERT_TRUE(failsafe_app_availability_online(&st));
    failsafe_on_link(&st, false, false, 5000);
    TEST_ASSERT_EQUAL(FAILSAFE_ENTRY_TIMER, st.phase);
    TEST_ASSERT_TRUE(failsafe_app_availability_online(&st));
}

void test_active_after_timer_offline(void)
{
    failsafe_state_t st;
    failsafe_init(&st, 10000, 2000);
    failsafe_on_link(&st, false, true, 0);
    failsafe_on_link(&st, false, true, 10000);
    TEST_ASSERT_TRUE(failsafe_is_active(&st));
    TEST_ASSERT_FALSE(failsafe_remote_writes_allowed(&st));
    TEST_ASSERT_FALSE(failsafe_app_availability_online(&st));
}

void test_recover_before_expiry(void)
{
    failsafe_state_t st;
    failsafe_init(&st, 10000, 2000);
    failsafe_on_link(&st, false, false, 0);
    failsafe_on_link(&st, true, true, 3000);
    TEST_ASSERT_EQUAL(FAILSAFE_IDLE, st.phase);
    TEST_ASSERT_TRUE(failsafe_remote_writes_allowed(&st));
}

void test_recovery_debounce(void)
{
    failsafe_state_t st;
    failsafe_init(&st, 10000, 2000);
    failsafe_on_link(&st, false, false, 0);
    failsafe_on_link(&st, false, false, 10000);
    TEST_ASSERT_TRUE(failsafe_is_active(&st));
    failsafe_on_link(&st, true, true, 11000);
    TEST_ASSERT_TRUE(failsafe_is_active(&st));
    failsafe_on_link(&st, true, true, 13000);
    TEST_ASSERT_FALSE(failsafe_is_active(&st));
    TEST_ASSERT_TRUE(failsafe_remote_writes_allowed(&st));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_entry_timer_keeps_online_and_writes);
    RUN_TEST(test_active_after_timer_offline);
    RUN_TEST(test_recover_before_expiry);
    RUN_TEST(test_recovery_debounce);
    return UNITY_END();
}
