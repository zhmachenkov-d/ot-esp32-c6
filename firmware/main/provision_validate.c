#include "provision_softap.h"

#include "app_config.h"
#include "ota_update.h"

#include <string.h>
#include <stdio.h>

void provision_softap_ota_gate(bool portal_active)
{
    ota_update_set_softap_active(portal_active);
}

provision_validate_result_t provision_validate(const provision_form_t *form)
{
    if (!form) {
        return PROVISION_ERR_SSID;
    }
    if (form->wifi_ssid[0] == '\0') {
        return PROVISION_ERR_SSID;
    }
    if (form->mqtt_host[0] == '\0') {
        return PROVISION_ERR_MQTT_HOST;
    }
    if (form->mqtt_port == 0) {
        return PROVISION_ERR_MQTT_PORT;
    }
    if (form->mqtt_tls && form->mqtt_ca[0] == '\0') {
        return PROVISION_ERR_MQTT_CA;
    }
    if (!(form->ch_min_c < form->ch_max_c)) {
        return PROVISION_ERR_CH_BOUNDS;
    }
    return PROVISION_OK;
}

provision_boot_action_t provision_boot_action(bool has_wifi_credentials, bool has_mqtt_config,
                                              bool mqtt_tls, bool ca_pem_ok)
{
    if (!has_wifi_credentials || !has_mqtt_config) {
        return PROVISION_BOOT_SOFTAP;
    }
    if (mqtt_tls && !ca_pem_ok) {
        return PROVISION_BOOT_RUN_NO_MQTT;
    }
    return PROVISION_BOOT_RUN;
}

bool provision_softap_build_ap_params(const char *device_id, const char *psk,
                                      provision_softap_ap_params_t *out)
{
    if (!out || !psk) {
        return false;
    }
    size_t psk_len = strlen(psk);
    if (psk_len < 8 || psk_len > 63) {
        return false;
    }
    memset(out, 0, sizeof(*out));
    const char *suffix =
        (device_id && strlen(device_id) >= 4) ? device_id + strlen(device_id) - 4 : "0000";
    snprintf(out->ssid, sizeof(out->ssid), "%s%s", APP_SOFTAP_SSID_PREFIX, suffix);
    strncpy(out->password, psk, sizeof(out->password) - 1);
    out->authmode = PROVISION_SOFTAP_AUTH_WPA2_PSK;
    return true;
}

void provision_softap_plan_wifi(bool wifi_already_init, bool wifi_started,
                                provision_softap_wifi_plan_t *out)
{
    if (!out) {
        return;
    }
    out->call_wifi_init = !wifi_already_init;
    /* Stop whenever the driver is up so set_mode(AP) is not racing STA. */
    out->call_wifi_stop = wifi_started;
}

bool provision_save_auth_set(provision_save_auth_t *auth, const char *token)
{
    if (!auth || !token || token[0] == '\0') {
        return false;
    }
    size_t n = strlen(token);
    if (n >= PROVISION_SAVE_TOKEN_MAX) {
        return false;
    }
    memset(auth, 0, sizeof(*auth));
    memcpy(auth->token, token, n);
    auth->consumed = false;
    return true;
}

static unsigned provision_save_token_diff(const char *expect, const char *got)
{
    size_t expect_len = strlen(expect);
    size_t got_len = strlen(got);
    unsigned diff = (unsigned)(expect_len ^ got_len);
    size_t n = expect_len > got_len ? expect_len : got_len;
    for (size_t i = 0; i < n; i++) {
        unsigned char a = (i < expect_len) ? (unsigned char)expect[i] : 0;
        unsigned char b = (i < got_len) ? (unsigned char)got[i] : 0;
        diff |= (unsigned)(a ^ b);
    }
    return diff;
}

bool provision_save_auth_matches(const provision_save_auth_t *auth, const char *submitted)
{
    if (!auth || !submitted || auth->token[0] == '\0' || auth->consumed) {
        return false;
    }
    return provision_save_token_diff(auth->token, submitted) == 0;
}

bool provision_save_auth_consume(provision_save_auth_t *auth, const char *submitted)
{
    if (!provision_save_auth_matches(auth, submitted)) {
        return false;
    }
    auth->consumed = true;
    return true;
}
