#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "esp_err.h"
#include "failsafe.h"
#include "ot_catalog.h"
#include "ot_poll.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    WRITABLE_CMD_ACCEPTED = 0,
    WRITABLE_CMD_REJECTED_RANGE,
    WRITABLE_CMD_REJECTED_FAILSAFE,
    WRITABLE_CMD_OT_FAILED,
    /** Queued for OT WRITE-DATA; not yet accepted — wait for on_write_complete. */
    WRITABLE_CMD_QUEUED,
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

/**
 * After fail-safe recovery debounce: allow at most one retained ot/1/set;
 * drop retained storms for other writables until a live (non-retained) command.
 */
void mqtt_commands_begin_post_recovery(void);

/** Host-testable retained-write gate. Returns true if the set should be applied. */
bool mqtt_commands_allow_inbound(uint8_t data_id, bool retain);

/** Process one inbound set payload (host-testable core). May return QUEUED. */
writable_cmd_outcome_t mqtt_commands_handle(uint8_t data_id, const char *payload,
                                            bool remote_writes_allowed,
                                            writable_command_t *out_cmd);

/**
 * Finalize a previously QUEUED write after OT exchange. On success → ACCEPTED
 * (and NVS last-CH for id 1); on bus failure → OT_FAILED (no state change).
 */
writable_cmd_outcome_t mqtt_commands_on_write_complete(uint8_t data_id,
                                                       ot_exchange_result_t result,
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
