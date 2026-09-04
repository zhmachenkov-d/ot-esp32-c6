#include "unity.h"
#include "ota_update.h"
#include "app_config.h"

#include <string.h>

void setUp(void) {}
void tearDown(void) {}

void test_semver_parse_and_cmp(void)
{
    int a, b, c;
    TEST_ASSERT_TRUE(ota_semver_parse("0.2.0", &a, &b, &c));
    TEST_ASSERT_EQUAL(0, a);
    TEST_ASSERT_EQUAL(2, b);
    TEST_ASSERT_EQUAL(0, c);
    TEST_ASSERT_FALSE(ota_semver_parse("0.2.0-rc1", &a, &b, &c));
    TEST_ASSERT_FALSE(ota_semver_parse("1.0", &a, &b, &c));
    TEST_ASSERT_TRUE(ota_semver_cmp("0.2.0", "0.1.0") > 0);
    TEST_ASSERT_TRUE(ota_semver_cmp("0.1.0", "0.2.0") < 0);
    TEST_ASSERT_EQUAL(0, ota_semver_cmp("1.2.3", "1.2.3"));
    TEST_ASSERT_EQUAL(0, ota_semver_cmp("bad", "0.1.0")); /* unparsable → not-newer */
}

void test_sha256_hex_valid(void)
{
    TEST_ASSERT_TRUE(ota_sha256_hex_valid(
        "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef"));
    TEST_ASSERT_FALSE(ota_sha256_hex_valid("0123456789ABCDEF0123456789abcdef0123456789abcdef0123456789abcdef"));
    TEST_ASSERT_FALSE(ota_sha256_hex_valid("abc"));
}

void test_manifest_parse_happy(void)
{
    const char *json =
        "{\"manifest_version\":1,\"firmware_id\":\"otc6_gateway\",\"version\":\"0.2.0\","
        "\"url\":\"https://github.com/zhmachenkov-d/ot-esp32-c6/releases/download/v0.2.0/otc6_gateway.bin\","
        "\"sha256\":\"0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef\","
        "\"size\":1100000,\"release_url\":\"https://github.com/zhmachenkov-d/ot-esp32-c6/releases/tag/v0.2.0\","
        "\"title\":\"OpenTherm Gateway\",\"summary\":\"OTA support.\",\"unknown_key\":true}";
    ota_manifest_t m;
    TEST_ASSERT_TRUE(ota_manifest_parse(json, OTA_FIRMWARE_ID, &m));
    TEST_ASSERT_EQUAL_STRING("0.2.0", m.version);
    TEST_ASSERT_TRUE(m.has_sha256);
    TEST_ASSERT_TRUE(m.has_size);
    TEST_ASSERT_EQUAL(1100000, (int)m.size);
    TEST_ASSERT_TRUE(ota_manifest_is_newer(&m, "0.1.0"));
    TEST_ASSERT_FALSE(ota_manifest_is_newer(&m, "0.2.0"));
}

void test_manifest_reject_paths(void)
{
    ota_manifest_t m;
    TEST_ASSERT_FALSE(ota_manifest_parse(
        "{\"manifest_version\":2,\"firmware_id\":\"otc6_gateway\",\"version\":\"0.2.0\","
        "\"url\":\"https://example.com/a.bin\"}",
        OTA_FIRMWARE_ID, &m));
    TEST_ASSERT_FALSE(ota_manifest_parse(
        "{\"manifest_version\":1,\"firmware_id\":\"other\",\"version\":\"0.2.0\","
        "\"url\":\"https://example.com/a.bin\"}",
        OTA_FIRMWARE_ID, &m));
    TEST_ASSERT_FALSE(ota_manifest_parse(
        "{\"manifest_version\":1,\"firmware_id\":\"otc6_gateway\",\"version\":\"0.2.0\","
        "\"url\":\"http://example.com/a.bin\"}",
        OTA_FIRMWARE_ID, &m));
    TEST_ASSERT_FALSE(ota_manifest_parse(
        "{\"manifest_version\":1,\"firmware_id\":\"otc6_gateway\",\"version\":\"0.2.0\","
        "\"url\":\"https://example.com/a.bin\",\"sha256\":\"deadbeef\"}",
        OTA_FIRMWARE_ID, &m));
    TEST_ASSERT_FALSE(ota_manifest_parse(
        "{\"manifest_version\":1,\"firmware_id\":\"otc6_gateway\","
        "\"url\":\"https://example.com/a.bin\"}",
        OTA_FIRMWARE_ID, &m));
    TEST_ASSERT_FALSE(ota_manifest_parse(
        "{\"manifest_version\":1,\"firmware_id\":\"otc6_gateway\",\"version\":\"0.2.0-rc1\","
        "\"url\":\"https://example.com/a.bin\"}",
        OTA_FIRMWARE_ID, &m));
}

void test_poll_policy(void)
{
    /* SoftAP / in-flight / not ready → never */
    TEST_ASSERT_FALSE(ota_should_poll_manifest(true, false, true, true, 10000, 0, 43200000, 3600000));
    TEST_ASSERT_FALSE(ota_should_poll_manifest(false, true, true, true, 10000, 0, 43200000, 3600000));
    TEST_ASSERT_FALSE(ota_should_poll_manifest(false, false, false, true, 10000, 0, 43200000, 3600000));

    /* MQTT ready trigger with no prior poll */
    TEST_ASSERT_TRUE(ota_should_poll_manifest(false, false, true, true, 1000, 0, 43200000, 3600000));

    /* Min gap blocks MQTT-ready storm */
    TEST_ASSERT_FALSE(ota_should_poll_manifest(false, false, true, true, 1000, 500, 43200000, 3600000));

    /* Periodic after interval */
    TEST_ASSERT_TRUE(ota_should_poll_manifest(false, false, true, false, 43200000 + 100,
                                              100, 43200000, 3600000));
}

void test_install_reject_paths(void)
{
    TEST_ASSERT_FALSE(ota_should_start_install("0.2.0", "0.2.0", true, false, false));
    TEST_ASSERT_FALSE(ota_should_start_install("0.2.0", "0.3.0", false, false, false));
    TEST_ASSERT_FALSE(ota_should_start_install("0.2.0", "0.3.0", true, true, false));
    TEST_ASSERT_FALSE(ota_should_start_install("0.2.0", "0.3.0", true, false, true));
    TEST_ASSERT_TRUE(ota_should_start_install("0.2.0", "0.3.0", true, false, false));
}

void test_confirm_gate_and_timeout(void)
{
    TEST_ASSERT_TRUE(ota_should_confirm(true, true));
    TEST_ASSERT_FALSE(ota_should_confirm(false, true)); /* already valid → no-op path */
    TEST_ASSERT_FALSE(ota_should_confirm(true, false));
    TEST_ASSERT_TRUE(ota_confirm_timeout_elapsed(true, 900000, 0, 900000));
    TEST_ASSERT_FALSE(ota_confirm_timeout_elapsed(true, 899999, 0, 900000));
    TEST_ASSERT_FALSE(ota_confirm_timeout_elapsed(false, 900000, 0, 900000));
}

void test_state_json_progress(void)
{
    ota_progress_state_t st = { 0 };
    strncpy(st.installed_version, "0.2.0", sizeof(st.installed_version) - 1);
    strncpy(st.latest_version, "0.3.0", sizeof(st.latest_version) - 1);
    strncpy(st.release_url, "https://example.com/r", sizeof(st.release_url) - 1);
    st.in_progress = true;
    st.has_percentage = true;
    st.update_percentage = 42;
    char buf[512];
    TEST_ASSERT_TRUE(ota_build_state_json(buf, sizeof(buf), &st) > 0);
    TEST_ASSERT_NOT_NULL(strstr(buf, "\"installed_version\":\"0.2.0\""));
    TEST_ASSERT_NOT_NULL(strstr(buf, "\"latest_version\":\"0.3.0\""));
    TEST_ASSERT_NOT_NULL(strstr(buf, "\"in_progress\":true"));
    TEST_ASSERT_NOT_NULL(strstr(buf, "\"update_percentage\":42"));
    TEST_ASSERT_NOT_NULL(strstr(buf, "\"release_url\":\"https://example.com/r\""));
}

void test_state_json_after_abort(void)
{
    /* Simulated post-abort publish: in_progress false, no percentage (device fail path). */
    ota_progress_state_t st = { 0 };
    strncpy(st.installed_version, "0.2.0", sizeof(st.installed_version) - 1);
    strncpy(st.latest_version, "0.3.0", sizeof(st.latest_version) - 1);
    st.in_progress = false;
    st.has_percentage = false;
    st.failed = true;
    char buf[512];
    TEST_ASSERT_TRUE(ota_build_state_json(buf, sizeof(buf), &st) > 0);
    TEST_ASSERT_NOT_NULL(strstr(buf, "\"in_progress\":false"));
    TEST_ASSERT_NOT_NULL(strstr(buf, "\"failed\":true"));
    TEST_ASSERT_NULL(strstr(buf, "update_percentage"));
}

void test_state_json_reboot_handoff(void)
{
    /* Pre-reboot retained publish must clear in_progress after successful OTA. */
    ota_progress_state_t st = { 0 };
    strncpy(st.installed_version, "0.2.0", sizeof(st.installed_version) - 1);
    strncpy(st.latest_version, "0.3.0", sizeof(st.latest_version) - 1);
    st.in_progress = true;
    st.has_percentage = true;
    st.update_percentage = 100;
    ota_progress_clear_in_flight(&st);
    char buf[512];
    TEST_ASSERT_TRUE(ota_build_state_json(buf, sizeof(buf), &st) > 0);
    TEST_ASSERT_NOT_NULL(strstr(buf, "\"in_progress\":false"));
    TEST_ASSERT_NULL(strstr(buf, "update_percentage"));
    TEST_ASSERT_NULL(strstr(buf, "\"failed\""));
    TEST_ASSERT_NOT_NULL(strstr(buf, "\"latest_version\":\"0.3.0\""));
}

void test_json_escape_quotes(void)
{
    char esc[64];
    TEST_ASSERT_TRUE(ota_json_escape(esc, sizeof(esc), "a\"b\\c") > 0);
    TEST_ASSERT_EQUAL_STRING("a\\\"b\\\\c", esc);
    ota_progress_state_t st = { 0 };
    strncpy(st.installed_version, "0.2.0", sizeof(st.installed_version) - 1);
    strncpy(st.latest_version, "0.3.0", sizeof(st.latest_version) - 1);
    strncpy(st.title, "Say \"hi\"", sizeof(st.title) - 1);
    char buf[512];
    TEST_ASSERT_TRUE(ota_build_state_json(buf, sizeof(buf), &st) > 0);
    TEST_ASSERT_NOT_NULL(strstr(buf, "\"title\":\"Say \\\"hi\\\"\""));
}

void test_size_fits_slot(void)
{
    const size_t slot = 0x1F0000;
    TEST_ASSERT_TRUE(ota_size_fits_slot(false, 0, slot));
    TEST_ASSERT_TRUE(ota_size_fits_slot(true, (int64_t)slot, slot));
    TEST_ASSERT_TRUE(ota_size_fits_slot(true, 1100000, slot));
    TEST_ASSERT_FALSE(ota_size_fits_slot(true, (int64_t)slot + 1, slot));
    TEST_ASSERT_FALSE(ota_size_fits_slot(true, -1, slot));
}

void test_install_topic(void)
{
    TEST_ASSERT_TRUE(ota_update_is_install_topic("otc6/aabbccddeeff/update/set", "aabbccddeeff"));
    TEST_ASSERT_FALSE(ota_update_is_install_topic("otc6/aabbccddeeff/ot/1/set", "aabbccddeeff"));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_semver_parse_and_cmp);
    RUN_TEST(test_sha256_hex_valid);
    RUN_TEST(test_manifest_parse_happy);
    RUN_TEST(test_manifest_reject_paths);
    RUN_TEST(test_poll_policy);
    RUN_TEST(test_install_reject_paths);
    RUN_TEST(test_confirm_gate_and_timeout);
    RUN_TEST(test_state_json_progress);
    RUN_TEST(test_state_json_after_abort);
    RUN_TEST(test_state_json_reboot_handoff);
    RUN_TEST(test_json_escape_quotes);
    RUN_TEST(test_size_fits_slot);
    RUN_TEST(test_install_topic);
    return UNITY_END();
}
