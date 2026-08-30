#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "esp_err.h"
#include "failsafe.h"
#include "ot_catalog.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    WRITABLE_CMD_ACCEPTED = 0,
    WRITABLE_CMD_REJECTED_RANGE,
    WRITABLE_CMD_REJECTED_FAILSAFE,
    WRITABLE_CMD_OT_FAILED,
} writable_cmd_outcome_t;

typedef struct {
    uint8_t data_id;
    float value_f;
    uint16_t value_raw;
    bool use_raw;
    writable_cmd_outcome_t outcome;
} writable_command_t;

esp_err_t mqtt_commands_init(const char *device_id, ot_catalog_t *cat,
                             float softap_ch_min, float softap_ch_max);

void mqtt_commands_set_failsafe(failsafe_state_t *fs);

esp_err_t mqtt_commands_start_subscriptions(void);

/** Process one inbound set payload (host-testable core). */
writable_cmd_outcome_t mqtt_commands_handle(uint8_t data_id, const char *payload,
                                            bool remote_writes_allowed,
                                            writable_command_t *out_cmd);

/** Map outcome → wire rejection reason string. */
const char *mqtt_commands_reason_wire(writable_cmd_outcome_t outcome);

/** Publish rejection JSON on ot/<N>/rejection. */
esp_err_t mqtt_commands_publish_rejection(const char *device_id, uint8_t id,
                                          writable_cmd_outcome_t outcome,
                                          float attempted, const ot_setpoint_bounds_t *bounds);

#ifdef __cplusplus
}
#endif
