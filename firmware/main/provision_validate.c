#include "provision_softap.h"

#include "app_config.h"

#include <string.h>
#include <stdio.h>

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
