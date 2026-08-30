#include "unity.h"
#include "ot_catalog.h"
#include "ot_poll.h"

void setUp(void) {}
void tearDown(void) {}

void test_classify_ack_available_writable_safe(void)
{
    ot_catalog_entry_t e;
    ot_catalog_classify_read(&e, 1, OT_EXCHANGE_OK, 0x1580, true, true, false);
    TEST_ASSERT_EQUAL(OT_SUPPORT_AVAILABLE, e.support);
    TEST_ASSERT_TRUE(e.readable);
    TEST_ASSERT_TRUE(e.writable);
    TEST_ASSERT_TRUE(e.has_raw);
}

void test_classify_unknown_unsupported(void)
{
    ot_catalog_entry_t e;
    ot_catalog_classify_read(&e, 99, OT_EXCHANGE_UNKNOWN_ID, 0, false, false, false);
    TEST_ASSERT_EQUAL(OT_SUPPORT_UNSUPPORTED, e.support);
    TEST_ASSERT_FALSE(e.readable);
    TEST_ASSERT_FALSE(e.writable);
}

void test_classify_data_invalid_available(void)
{
    ot_catalog_entry_t e;
    ot_catalog_classify_read(&e, 25, OT_EXCHANGE_INVALID, 0, false, false, false);
    TEST_ASSERT_EQUAL(OT_SUPPORT_AVAILABLE, e.support);
    TEST_ASSERT_TRUE(e.readable);
    TEST_ASSERT_FALSE(e.has_raw);
}

void test_classify_id0_writable_no_write_data(void)
{
    ot_catalog_entry_t e;
    ot_catalog_classify_read(&e, 0, OT_EXCHANGE_OK, 0x010A, true, true, false);
    TEST_ASSERT_TRUE(e.writable);
}

void test_classify_non_writable_directory(void)
{
    ot_catalog_entry_t e;
    ot_catalog_classify_read(&e, 25, OT_EXCHANGE_OK, 0x2000, false, false, false);
    TEST_ASSERT_TRUE(e.readable);
    TEST_ASSERT_FALSE(e.writable);
}

void test_mandatory_fixture_ids(void)
{
    /* fixtures/mandatory_data_ids.json — 0,1,3,14,17,25 */
    const uint8_t ids[] = { 0, 1, 3, 14, 17, 25 };
    for (unsigned i = 0; i < sizeof(ids); i++) {
        ot_catalog_entry_t e;
        ot_catalog_classify_read(&e, ids[i], OT_EXCHANGE_OK, 0x1000, true,
                                 ids[i] == 0 || ids[i] == 1, false);
        TEST_ASSERT_EQUAL(OT_SUPPORT_AVAILABLE, e.support);
    }
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_classify_ack_available_writable_safe);
    RUN_TEST(test_classify_unknown_unsupported);
    RUN_TEST(test_classify_data_invalid_available);
    RUN_TEST(test_classify_id0_writable_no_write_data);
    RUN_TEST(test_classify_non_writable_directory);
    RUN_TEST(test_mandatory_fixture_ids);
    return UNITY_END();
}
