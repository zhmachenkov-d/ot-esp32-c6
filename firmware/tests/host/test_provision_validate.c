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

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_valid_form);
    RUN_TEST(test_reject_empty_host);
    RUN_TEST(test_reject_ch_bounds);
    RUN_TEST(test_reject_port_zero);
    RUN_TEST(test_reject_tls_without_ca);
    RUN_TEST(test_accept_tls_with_ca);
    return UNITY_END();
}
