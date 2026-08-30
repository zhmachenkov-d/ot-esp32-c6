#include "mqtt_commands.h"

#include "app_config.h"
#include "failsafe.h"
#include "mqtt_discovery.h"
#include "mqtt_ha.h"
#include "nvs_store.h"
#include "ot_codec.h"
#include "ot_poll.h"

#include "esp_log.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

static const char *TAG = "mqtt_cmd";

static char s_device_id[16];
static ot_catalog_t *s_cat;
static float s_ch_min = APP_CH_MIN_C_DEFAULT;
static float s_ch_max = APP_CH_MAX_C_DEFAULT;
static failsafe_state_t *s_fs;

static bool s_pending[OT_CATALOG_MAX_IDS];
static float s_pending_f[OT_CATALOG_MAX_IDS];

static bool s_post_recovery;
static bool s_retained_id1_applied;

void mqtt_commands_set_failsafe(failsafe_state_t *fs)
{
    s_fs = fs;
}

void mqtt_commands_begin_post_recovery(void)
{
    s_post_recovery = true;
    s_retained_id1_applied = false;
}

bool mqtt_commands_allow_inbound(uint8_t data_id, bool retain)
{
    if (!s_post_recovery) {
        return true;
    }
    if (!retain) {
        s_post_recovery = false;
        return true;
    }
    /* Retained storm gate */
    if (data_id != 1) {
        return false;
    }
    if (s_retained_id1_applied) {
        return false;
    }
    s_retained_id1_applied = true;
    return true;
}

const char *mqtt_commands_reason_wire(writable_cmd_outcome_t outcome)
{
    switch (outcome) {
    case WRITABLE_CMD_REJECTED_RANGE:
        return "out_of_range";
    case WRITABLE_CMD_REJECTED_FAILSAFE:
        return "rejected_failsafe";
    case WRITABLE_CMD_OT_FAILED:
        return "ot_failed";
    default:
        return "accepted";
    }
}

esp_err_t mqtt_commands_publish_rejection(const char *device_id, uint8_t id,
                                          writable_cmd_outcome_t outcome,
                                          float attempted, const ot_setpoint_bounds_t *bounds)
{
    char topic[96];
    char json[192];
    snprintf(topic, sizeof(topic), "%s%s/ot/%u/rejection", APP_MQTT_TOPIC_ROOT, device_id, (unsigned)id);
    const char *reason = mqtt_commands_reason_wire(outcome);
    if (outcome == WRITABLE_CMD_REJECTED_RANGE && bounds) {
        snprintf(json, sizeof(json),
                 "{\"reason\":\"%s\",\"attempted\":%.2f,\"min\":%.2f,\"max\":%.2f,\"ts\":0}",
                 reason, (double)attempted, (double)bounds->min_c, (double)bounds->max_c);
    } else {
        snprintf(json, sizeof(json),
                 "{\"reason\":\"%s\",\"attempted\":%.2f,\"ts\":0}",
                 reason, (double)attempted);
    }
    return mqtt_ha_publish(topic, json, 1, false);
}

static void publish_accepted_state(uint8_t id, float value_f)
{
    char state[32];
    if (id == 0) {
        snprintf(state, sizeof(state), "%s", value_f > 0.5f ? "1" : "0");
    } else {
        ot_codec_format_float(state, sizeof(state), value_f, true);
    }
    mqtt_discovery_publish_state(s_device_id, id, state);
    if (id == 0) {
        mqtt_discovery_publish_status_projections(s_device_id,
                                                  ot_poll_get_master_status_flags(),
                                                  ot_poll_get_slave_status_flags(),
                                                  true);
        mqtt_discovery_publish_climate_mode(s_device_id,
                                            value_f > 0.5f);
    }
}

writable_cmd_outcome_t mqtt_commands_on_write_complete(uint8_t data_id,
                                                       ot_exchange_result_t result,
                                                       writable_command_t *out_cmd)
{
    writable_command_t cmd = { .data_id = data_id, .outcome = WRITABLE_CMD_OT_FAILED };
    if (data_id >= OT_CATALOG_MAX_IDS || !s_pending[data_id]) {
        if (out_cmd) {
            *out_cmd = cmd;
        }
        return cmd.outcome;
    }
    cmd.value_f = s_pending_f[data_id];
    s_pending[data_id] = false;

    if (result == OT_EXCHANGE_OK) {
        cmd.outcome = WRITABLE_CMD_ACCEPTED;
        if (data_id == 1) {
            nvs_store_set_last_ch_setpoint(cmd.value_f);
        }
        publish_accepted_state(data_id, cmd.value_f);
    } else {
        cmd.outcome = WRITABLE_CMD_OT_FAILED;
        mqtt_commands_publish_rejection(s_device_id, data_id, cmd.outcome, cmd.value_f, NULL);
        ESP_LOGW(TAG, "OT write failed id=%u", (unsigned)data_id);
    }
    if (out_cmd) {
        *out_cmd = cmd;
    }
    return cmd.outcome;
}

static void on_ot_write_complete(uint8_t data_id, uint16_t raw, ot_exchange_result_t result, void *ctx)
{
    (void)raw;
    (void)ctx;
    mqtt_commands_on_write_complete(data_id, result, NULL);
}

writable_cmd_outcome_t mqtt_commands_handle(uint8_t data_id, const char *payload,
                                            bool remote_writes_allowed,
                                            writable_command_t *out_cmd)
{
    writable_command_t cmd = { .data_id = data_id, .outcome = WRITABLE_CMD_OT_FAILED };
    if (out_cmd) {
        *out_cmd = cmd;
    }
    if (!payload) {
        return WRITABLE_CMD_OT_FAILED;
    }
    if (!remote_writes_allowed) {
        cmd.outcome = WRITABLE_CMD_REJECTED_FAILSAFE;
        cmd.value_f = strtof(payload, NULL);
        if (out_cmd) {
            *out_cmd = cmd;
        }
        return cmd.outcome;
    }

    if (s_cat) {
        const ot_catalog_entry_t *e = ot_catalog_get(s_cat, data_id);
        if (!e || e->support != OT_SUPPORT_AVAILABLE || !e->writable) {
            cmd.outcome = WRITABLE_CMD_OT_FAILED;
            if (out_cmd) {
                *out_cmd = cmd;
            }
            return cmd.outcome;
        }
    }

    /* ID 0: CH enable via master Status flags on READ exchange — never WRITE-DATA */
    if (data_id == 0) {
        bool on = (strcmp(payload, "1") == 0 || strcasecmp(payload, "ON") == 0 ||
                   strcasecmp(payload, "true") == 0 || strcasecmp(payload, "heat") == 0);
        if (strcasecmp(payload, "off") == 0 || strcmp(payload, "0") == 0) {
            on = false;
        }
        uint8_t hb = ot_poll_get_master_status_flags();
        hb = ot_codec_flag8_set(hb, 0, on);
        ot_poll_set_master_status_flags(hb);
        cmd.outcome = WRITABLE_CMD_ACCEPTED;
        cmd.value_f = on ? 1.0f : 0.0f;
        if (out_cmd) {
            *out_cmd = cmd;
        }
        return cmd.outcome;
    }

    float v = strtof(payload, NULL);
    cmd.value_f = v;

    if (ot_catalog_is_range_checked(data_id) && s_cat) {
        ot_setpoint_bounds_t b;
        if (ot_catalog_resolve_bounds(s_cat, data_id, s_ch_min, s_ch_max, &b)) {
            if (v < b.min_c || v > b.max_c) {
                cmd.outcome = WRITABLE_CMD_REJECTED_RANGE;
                if (out_cmd) {
                    *out_cmd = cmd;
                }
                return cmd.outcome;
            }
        }
    }

    uint16_t raw;
    if (data_id == 1 || data_id == 8 || data_id == 56 || data_id == 57) {
        raw = ot_codec_float_to_f88(v);
    } else if (data_id == 7 || data_id == 14) {
        raw = ot_codec_pack_hb_lb((uint8_t)v, 0);
    } else {
        raw = (uint16_t)v;
    }
    cmd.value_raw = raw;
    cmd.use_raw = true;

    if (!ot_poll_enqueue_write(data_id, raw)) {
        cmd.outcome = WRITABLE_CMD_OT_FAILED;
    } else {
        /* Enqueue is not acceptance — wait for OT write completion (T045). */
        if (data_id < OT_CATALOG_MAX_IDS) {
            s_pending[data_id] = true;
            s_pending_f[data_id] = v;
        }
        cmd.outcome = WRITABLE_CMD_QUEUED;
    }
    if (out_cmd) {
        *out_cmd = cmd;
    }
    return cmd.outcome;
}

static bool parse_set_topic(const char *topic, uint8_t *id_out)
{
    /* otc6/<device_id>/ot/<N>/set */
    const char *p = strstr(topic, "/ot/");
    if (!p) {
        return false;
    }
    p += 4;
    char *end = NULL;
    long id = strtol(p, &end, 10);
    if (!end || strncmp(end, "/set", 4) != 0) {
        return false;
    }
    if (id < 0 || id > 127) {
        return false;
    }
    *id_out = (uint8_t)id;
    return true;
}

static bool is_climate_mode_set(const char *topic)
{
    return topic && strstr(topic, "/climate/mode/set") != NULL;
}

static void apply_outcome(uint8_t id, writable_cmd_outcome_t o, const writable_command_t *cmd)
{
    if (o == WRITABLE_CMD_QUEUED) {
        return;
    }
    if (o != WRITABLE_CMD_ACCEPTED) {
        ot_setpoint_bounds_t b;
        const ot_setpoint_bounds_t *bp = NULL;
        if (o == WRITABLE_CMD_REJECTED_RANGE && s_cat &&
            ot_catalog_resolve_bounds(s_cat, id, s_ch_min, s_ch_max, &b)) {
            bp = &b;
        }
        mqtt_commands_publish_rejection(s_device_id, id, o, cmd->value_f, bp);
        ESP_LOGW(TAG, "reject id=%u reason=%s", (unsigned)id, mqtt_commands_reason_wire(o));
        return;
    }
    publish_accepted_state(id, cmd->value_f);
}

static void on_mqtt_message(const char *topic, const char *payload, int len, bool retain, void *ctx)
{
    (void)len;
    (void)ctx;
    uint8_t id;
    if (is_climate_mode_set(topic)) {
        id = 0;
    } else if (!parse_set_topic(topic, &id)) {
        return;
    }

    if (!mqtt_commands_allow_inbound(id, retain)) {
        ESP_LOGI(TAG, "drop retained set id=%u (post-recovery policy)", (unsigned)id);
        return;
    }

    bool allowed = true;
    if (s_fs) {
        allowed = failsafe_remote_writes_allowed(s_fs);
    }
    writable_command_t cmd;
    writable_cmd_outcome_t o = mqtt_commands_handle(id, payload, allowed, &cmd);
    apply_outcome(id, o, &cmd);
}

esp_err_t mqtt_commands_init(const char *device_id, ot_catalog_t *cat,
                             float softap_ch_min, float softap_ch_max)
{
    if (!device_id) {
        return ESP_ERR_INVALID_ARG;
    }
    strncpy(s_device_id, device_id, sizeof(s_device_id) - 1);
    s_cat = cat;
    s_ch_min = softap_ch_min;
    s_ch_max = softap_ch_max;
    memset(s_pending, 0, sizeof(s_pending));
    ot_poll_set_write_complete_cb(on_ot_write_complete, NULL);
    mqtt_ha_set_message_callback(on_mqtt_message, NULL);
    return ESP_OK;
}

esp_err_t mqtt_commands_start_subscriptions(void)
{
    if (!s_cat) {
        return ESP_ERR_INVALID_STATE;
    }
    char topic[96];
    for (int id = 0; id < OT_CATALOG_MAX_IDS; id++) {
        if (s_cat->ids[id].support == OT_SUPPORT_AVAILABLE && s_cat->ids[id].writable) {
            snprintf(topic, sizeof(topic), "%s%s/ot/%u/set", APP_MQTT_TOPIC_ROOT, s_device_id, (unsigned)id);
            mqtt_ha_subscribe(topic, 1);
        }
    }
    snprintf(topic, sizeof(topic), "%s%s/climate/mode/set", APP_MQTT_TOPIC_ROOT, s_device_id);
    mqtt_ha_subscribe(topic, 1);
    return ESP_OK;
}
