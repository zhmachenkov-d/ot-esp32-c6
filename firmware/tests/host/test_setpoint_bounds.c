#include "unity.h"
#include "ot_catalog.h"
#include "ot_codec.h"

void setUp(void) {}
void tearDown(void) {}

void test_id1_uses_softap_then_id57_max(void)
{
    ot_catalog_t cat;
    ot_catalog_init(&cat);
    cat.ids[57].support = OT_SUPPORT_AVAILABLE;
    cat.ids[57].has_raw = true;
    cat.ids[57].last_raw = ot_codec_float_to_f88(75.0f);

    ot_setpoint_bounds_t b;
    TEST_ASSERT_TRUE(ot_catalog_resolve_bounds(&cat, 1, 10.0f, 90.0f, &b));
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 10.0f, b.min_c);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 75.0f, b.max_c);
    TEST_ASSERT_TRUE(b.from_boiler_max);
}

void test_id14_pct_0_100(void)
{
    ot_catalog_t cat;
    ot_catalog_init(&cat);
    ot_setpoint_bounds_t b;
    TEST_ASSERT_TRUE(ot_catalog_resolve_bounds(&cat, 14, 10.0f, 90.0f, &b));
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, b.min_c);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 100.0f, b.max_c);
}

void test_id56_from_id48(void)
{
    ot_catalog_t cat;
    ot_catalog_init(&cat);
    cat.ids[48].support = OT_SUPPORT_AVAILABLE;
    cat.ids[48].has_raw = true;
    cat.ids[48].last_raw = ot_codec_pack_hb_lb(60, 20);
    ot_setpoint_bounds_t b;
    TEST_ASSERT_TRUE(ot_catalog_resolve_bounds(&cat, 56, 10.0f, 90.0f, &b));
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 20.0f, b.min_c);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 60.0f, b.max_c);
}

void test_range_checked_set(void)
{
    TEST_ASSERT_TRUE(ot_catalog_is_range_checked(1));
    TEST_ASSERT_TRUE(ot_catalog_is_range_checked(8));
    TEST_ASSERT_TRUE(ot_catalog_is_range_checked(7));
    TEST_ASSERT_TRUE(ot_catalog_is_range_checked(14));
    TEST_ASSERT_TRUE(ot_catalog_is_range_checked(56));
    TEST_ASSERT_TRUE(ot_catalog_is_range_checked(57));
    TEST_ASSERT_FALSE(ot_catalog_is_range_checked(25));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_id1_uses_softap_then_id57_max);
    RUN_TEST(test_id14_pct_0_100);
    RUN_TEST(test_id56_from_id48);
    RUN_TEST(test_range_checked_set);
    return UNITY_END();
}
