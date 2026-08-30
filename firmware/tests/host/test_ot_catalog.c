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

void test_catalog_nvs_full_round_trip(void)
{
    ot_catalog_t src;
    ot_catalog_t dst;
    TEST_ASSERT_EQUAL(ESP_OK, ot_catalog_init(&src));
    src.version = 1;
    src.validated = true;
    for (int i = 0; i < OT_CATALOG_MAX_IDS; i++) {
        src.ids[i].support = (i % 3 == 0) ? OT_SUPPORT_AVAILABLE : OT_SUPPORT_UNSUPPORTED;
        src.ids[i].readable = src.ids[i].support == OT_SUPPORT_AVAILABLE;
        src.ids[i].writable = (i % 5 == 0);
        src.ids[i].has_raw = (i % 2 == 0);
        src.ids[i].last_raw = (uint16_t)(0x1000 + i);
        src.ids[i].poll_tier = (i % 2 == 0) ? OT_TIER_FAST : OT_TIER_SLOW;
    }

    TEST_ASSERT_EQUAL(ESP_OK, ot_catalog_save_nvs(&src));
    TEST_ASSERT_EQUAL(ESP_OK, ot_catalog_load_nvs(&dst));

    TEST_ASSERT_EQUAL_UINT32(src.version, dst.version);
    TEST_ASSERT_EQUAL(src.validated, dst.validated);
    for (int i = 0; i < OT_CATALOG_MAX_IDS; i++) {
        TEST_ASSERT_EQUAL(src.ids[i].support, dst.ids[i].support);
        TEST_ASSERT_EQUAL(src.ids[i].writable, dst.ids[i].writable);
        TEST_ASSERT_EQUAL(src.ids[i].has_raw, dst.ids[i].has_raw);
        TEST_ASSERT_EQUAL_HEX16(src.ids[i].last_raw, dst.ids[i].last_raw);
        TEST_ASSERT_EQUAL(src.ids[i].poll_tier, dst.ids[i].poll_tier);
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
    RUN_TEST(test_catalog_nvs_full_round_trip);
    return UNITY_END();
}
