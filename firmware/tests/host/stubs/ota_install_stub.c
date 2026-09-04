#include "ota_update.h"

#include <string.h>

static int s_install_calls;
static char s_last_payload[64];
static bool s_last_was_null;
static bool s_softap_active;

void host_ota_install_reset(void)
{
    s_install_calls = 0;
    s_last_payload[0] = '\0';
    s_last_was_null = false;
    s_softap_active = false;
}

int host_ota_install_call_count(void)
{
    return s_install_calls;
}

const char *host_ota_install_last_payload(void)
{
    return s_last_was_null ? NULL : s_last_payload;
}

bool host_ota_softap_is_active(void)
{
    return s_softap_active;
}

void ota_update_set_softap_active(bool active)
{
    s_softap_active = active;
}

esp_err_t ota_update_handle_install(const char *payload)
{
    s_install_calls++;
    if (!payload) {
        s_last_was_null = true;
        s_last_payload[0] = '\0';
        return ESP_ERR_INVALID_ARG;
    }
    s_last_was_null = false;
    strncpy(s_last_payload, payload, sizeof(s_last_payload) - 1);
    s_last_payload[sizeof(s_last_payload) - 1] = '\0';
    return ESP_OK;
}
