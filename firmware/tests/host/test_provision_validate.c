#include "unity.h"
#include "provision_softap.h"

void setUp(void) {}
void tearDown(void) {}

void test_valid_form(void)
{
    provision_form_t f = {
        .wifi_ssid = "home",
        .mqtt_host = "192.168.1.10",
        .mqtt_port = 1883,
        .ch_min_c = 10.0f,
        .ch_max_c = 90.0f,
    };
    TEST_ASSERT_EQUAL(PROVISION_OK, provision_validate(&f));
}

void test_reject_empty_host(void)
{
    provision_form_t f = {
        .wifi_ssid = "home",
        .mqtt_host = "",
        .mqtt_port = 1883,
        .ch_min_c = 10.0f,
        .ch_max_c = 90.0f,
    };
    TEST_ASSERT_EQUAL(PROVISION_ERR_MQTT_HOST, provision_validate(&f));
}

void test_reject_ch_bounds(void)
{
    provision_form_t f = {
        .wifi_ssid = "home",
        .mqtt_host = "broker.local",
        .mqtt_port = 1883,
        .ch_min_c = 90.0f,
        .ch_max_c = 10.0f,
    };
    TEST_ASSERT_EQUAL(PROVISION_ERR_CH_BOUNDS, provision_validate(&f));
}

void test_reject_port_zero(void)
{
    provision_form_t f = {
        .wifi_ssid = "home",
        .mqtt_host = "broker.local",
        .mqtt_port = 0,
        .ch_min_c = 10.0f,
        .ch_max_c = 90.0f,
    };
    TEST_ASSERT_EQUAL(PROVISION_ERR_MQTT_PORT, provision_validate(&f));
}

void test_reject_tls_without_ca(void)
{
    provision_form_t f = {
        .wifi_ssid = "home",
        .mqtt_host = "192.168.1.10",
        .mqtt_port = 8883,
        .mqtt_tls = true,
        .mqtt_ca = "",
        .ch_min_c = 10.0f,
        .ch_max_c = 90.0f,
    };
    TEST_ASSERT_EQUAL(PROVISION_ERR_MQTT_CA, provision_validate(&f));
}

void test_accept_tls_with_ca(void)
{
    provision_form_t f = {
        .wifi_ssid = "home",
        .mqtt_host = "192.168.1.10",
        .mqtt_port = 8883,
        .mqtt_tls = true,
        .mqtt_ca = "-----BEGIN CERTIFICATE-----\nMIIB\n-----END CERTIFICATE-----\n",
        .ch_min_c = 10.0f,
        .ch_max_c = 90.0f,
    };
    TEST_ASSERT_EQUAL(PROVISION_OK, provision_validate(&f));
}

void test_boot_no_credentials_opens_softap(void)
{
    TEST_ASSERT_EQUAL(PROVISION_BOOT_SOFTAP,
                      provision_boot_action(false, false, false, false));
    TEST_ASSERT_EQUAL(PROVISION_BOOT_SOFTAP,
                      provision_boot_action(true, false, true, false));
}

void test_boot_tls_missing_ca_does_not_open_softap(void)
{
    /* Credentials present + mqtt_tls + missing CA → SoftAP must not auto-start. */
    TEST_ASSERT_EQUAL(PROVISION_BOOT_RUN_NO_MQTT,
                      provision_boot_action(true, true, true, false));
}

void test_boot_tls_with_ca_runs_mqtt(void)
{
    TEST_ASSERT_EQUAL(PROVISION_BOOT_RUN,
                      provision_boot_action(true, true, true, true));
    TEST_ASSERT_EQUAL(PROVISION_BOOT_RUN,
                      provision_boot_action(true, true, false, false));
}

void test_boot_after_button_clears_credentials_opens_softap(void)
{
    /* Long-press clears credentials then restart → first-boot SoftAP path. */
    TEST_ASSERT_EQUAL(PROVISION_BOOT_SOFTAP,
                      provision_boot_action(false, false, true, false));
}

void test_softap_ap_params_wpa2_when_psk_present(void)
{
    provision_softap_ap_params_t p;
    TEST_ASSERT_TRUE(provision_softap_build_ap_params("aabbccddeeff", "0123456789abcdef", &p));
    TEST_ASSERT_EQUAL_STRING("OTC6-eeff", p.ssid);
    TEST_ASSERT_EQUAL_STRING("0123456789abcdef", p.password);
    TEST_ASSERT_EQUAL(PROVISION_SOFTAP_AUTH_WPA2_PSK, p.authmode);
    TEST_ASSERT_NOT_EQUAL(PROVISION_SOFTAP_AUTH_OPEN, p.authmode);
}

void test_softap_ap_params_reject_short_or_missing_psk(void)
{
    provision_softap_ap_params_t p;
    TEST_ASSERT_FALSE(provision_softap_build_ap_params("aabbccddeeff", "short", &p));
    TEST_ASSERT_FALSE(provision_softap_build_ap_params("aabbccddeeff", NULL, &p));
    TEST_ASSERT_FALSE(provision_softap_build_ap_params("aabbccddeeff", "", &p));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_valid_form);
    RUN_TEST(test_reject_empty_host);
    RUN_TEST(test_reject_ch_bounds);
    RUN_TEST(test_reject_port_zero);
    RUN_TEST(test_reject_tls_without_ca);
    RUN_TEST(test_accept_tls_with_ca);
    RUN_TEST(test_boot_no_credentials_opens_softap);
    RUN_TEST(test_boot_tls_missing_ca_does_not_open_softap);
    RUN_TEST(test_boot_tls_with_ca_runs_mqtt);
    RUN_TEST(test_boot_after_button_clears_credentials_opens_softap);
    RUN_TEST(test_softap_ap_params_wpa2_when_psk_present);
    RUN_TEST(test_softap_ap_params_reject_short_or_missing_psk);
    return UNITY_END();
}
