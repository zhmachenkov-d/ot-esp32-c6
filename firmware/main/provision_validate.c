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
