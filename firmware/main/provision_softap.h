#pragma once

#include <stdbool.h>

#include "esp_err.h"
#include "nvs_store.h"

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
    char mqtt_ca[NVS_MQTT_CA_PEM_MAX];
    float ch_min_c;
    float ch_max_c;
} provision_form_t;

typedef enum {
    PROVISION_OK = 0,
    PROVISION_ERR_SSID,
    PROVISION_ERR_MQTT_HOST,
    PROVISION_ERR_MQTT_PORT,
    PROVISION_ERR_MQTT_CA,
    PROVISION_ERR_CH_BOUNDS,
} provision_validate_result_t;

/** Host-testable SoftAP form validation. */
provision_validate_result_t provision_validate(const provision_form_t *form);

/**
 * Boot SoftAP / MQTT policy once NVS has been read (host-testable).
 * Missing TLS CA must not auto-open SoftAP; recovery is GPIO9 long-press only.
 */
typedef enum {
    PROVISION_BOOT_SOFTAP = 0,  /**< No Wi‑Fi/MQTT credentials — open SoftAP */
    PROVISION_BOOT_RUN,         /**< STA + MQTT as configured */
    PROVISION_BOOT_RUN_NO_MQTT, /**< STA only; TLS on but CA missing/empty */
} provision_boot_action_t;

provision_boot_action_t provision_boot_action(bool has_wifi_credentials, bool has_mqtt_config,
                                              bool mqtt_tls, bool ca_pem_ok);

esp_err_t provision_softap_start(const char *device_id);
void provision_softap_stop(void);

/** Start GPIO9 long-press monitor (≥5 s clears credentials + SoftAP). */
esp_err_t provision_button_start(void);

bool provision_is_active(void);

#ifdef __cplusplus
}
#endif
