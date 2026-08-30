#include "provision_softap.h"

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
