#pragma once

#include <stdbool.h>

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    char wifi_ssid[33];
    char wifi_password[65];
    char mqtt_host[128];
    uint16_t mqtt_port;
    char mqtt_username[64];
    char mqtt_password[64];
    bool mqtt_tls;
    float ch_min_c;
    float ch_max_c;
} provision_form_t;

typedef enum {
    PROVISION_OK = 0,
    PROVISION_ERR_SSID,
    PROVISION_ERR_MQTT_HOST,
    PROVISION_ERR_MQTT_PORT,
    PROVISION_ERR_CH_BOUNDS,
} provision_validate_result_t;

/** Host-testable SoftAP form validation. */
provision_validate_result_t provision_validate(const provision_form_t *form);

esp_err_t provision_softap_start(const char *device_id);
void provision_softap_stop(void);

/** Start GPIO9 long-press monitor (≥5 s clears credentials + SoftAP). */
esp_err_t provision_button_start(void);

bool provision_is_active(void);

#ifdef __cplusplus
}
#endif
