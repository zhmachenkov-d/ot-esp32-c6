#include "unity.h"
#include "ot_codec.h"

void setUp(void) {}
void tearDown(void) {}

void test_f88_roundtrip_21_5(void)
{
    uint16_t raw = ot_codec_float_to_f88(21.5f);
    TEST_ASSERT_EQUAL_UINT16(0x1580, raw);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 21.5f, ot_codec_f88_to_float(raw));
}

void test_flag8_bits(void)
{
    uint8_t f = 0;
    f = ot_codec_flag8_set(f, 0, true);
    TEST_ASSERT_TRUE(ot_codec_flag8_get(f, 0));
    f = ot_codec_flag8_set(f, 3, true);
    TEST_ASSERT_TRUE(ot_codec_flag8_get(f, 3));
    f = ot_codec_flag8_set(f, 0, false);
    TEST_ASSERT_FALSE(ot_codec_flag8_get(f, 0));
}

void test_format_empty_when_invalid(void)
{
    char buf[16];
    TEST_ASSERT_EQUAL(0, ot_codec_format_float(buf, sizeof(buf), 1.0f, false));
    TEST_ASSERT_EQUAL_STRING("", buf);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_f88_roundtrip_21_5);
    RUN_TEST(test_flag8_bits);
    RUN_TEST(test_format_empty_when_invalid);
    return UNITY_END();
}
