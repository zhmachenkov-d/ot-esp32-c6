#include "mqtt_discovery.h"

#include "app_config.h"
#include "mqtt_ha.h"
#include "ot_codec.h"
#include "ot_poll.h"

#include "esp_log.h"

#include <stdio.h>
#include <string.h>

static const char *TAG = "mqtt_disc";

int mqtt_discovery_build_device_json(char *buf, size_t cap, const char *device_id,
                                    const char *fw_version)
{
    return snprintf(buf, cap,
                    "{\"identifiers\":[\"otc6_%s\"],\"manufacturer\":\"ot-esp32-c6\","
                    "\"model\":\"WeAct ESP32-C6 Mini OpenTherm Gateway\","
                    "\"name\":\"OpenTherm Gateway\",\"sw_version\":\"%s\"}",
                    device_id, fw_version ? fw_version : APP_FW_VERSION);
}

static int append_device_and_avail(char *buf, size_t cap, int off, const char *device_id)
{
    char device[256];
    char avail[80];
    mqtt_discovery_build_device_json(device, sizeof(device), device_id, APP_FW_VERSION);
    snprintf(avail, sizeof(avail), "%s%s/status", APP_MQTT_TOPIC_ROOT, device_id);
    return snprintf(buf + off, cap > (size_t)off ? cap - (size_t)off : 0,
                    ",\"device\":%s,\"availability_topic\":\"%s\","
                    "\"payload_available\":\"online\",\"payload_not_available\":\"offline\"}",
                    device, avail);
}

int mqtt_discovery_build_boiler_link_config(char *buf, size_t cap, const char *device_id)
{
    int n = snprintf(buf, cap,
                     "{\"name\":\"Boiler link\",\"unique_id\":\"otc6_%s_boiler_link\","
                     "\"state_topic\":\"%s%s/boiler_link\","
                     "\"payload_on\":\"healthy\",\"payload_off\":\"unhealthy\","
                     "\"device_class\":\"connectivity\"",
                     device_id, APP_MQTT_TOPIC_ROOT, device_id);
    if (n < 0 || (size_t)n >= cap) {
        return -1;
    }
    int m = append_device_and_avail(buf, cap, n, device_id);
    return (m < 0) ? -1 : n + m;
}

int mqtt_discovery_build_sensor_config(char *buf, size_t cap, const char *device_id,
                                       uint8_t id, const char *name,
                                       const char *unit, const char *device_class)
{
    int n = snprintf(buf, cap,
                     "{\"name\":\"%s\",\"unique_id\":\"otc6_%s_ot_%u\","
                     "\"state_topic\":\"%s%s/ot/%u/state\"",
                     name ? name : "OT", device_id, (unsigned)id,
                     APP_MQTT_TOPIC_ROOT, device_id, (unsigned)id);
    if (n < 0 || (size_t)n >= cap) {
        return -1;
    }
    if (unit && unit[0]) {
        int u = snprintf(buf + n, cap - (size_t)n, ",\"unit_of_measurement\":\"%s\"", unit);
        if (u < 0) {
            return -1;
        }
        n += u;
    }
    if (device_class && device_class[0]) {
        int d = snprintf(buf + n, cap - (size_t)n, ",\"device_class\":\"%s\"", device_class);
        if (d < 0) {
            return -1;
        }
        n += d;
    }
    int m = append_device_and_avail(buf, cap, n, device_id);
    return (m < 0) ? -1 : n + m;
}

int mqtt_discovery_build_number_config(char *buf, size_t cap, const char *device_id,
                                       uint8_t id, const char *name,
                                       bool has_range, float min_v, float max_v, float step)
{
    int n;
    if (has_range) {
        n = snprintf(buf, cap,
                     "{\"name\":\"%s\",\"unique_id\":\"otc6_%s_ot_%u\","
                     "\"state_topic\":\"%s%s/ot/%u/state\","
                     "\"command_topic\":\"%s%s/ot/%u/set\","
                     "\"min\":%.2f,\"max\":%.2f,\"step\":%.2f,\"mode\":\"box\"",
                     name ? name : "OT", device_id, (unsigned)id,
                     APP_MQTT_TOPIC_ROOT, device_id, (unsigned)id,
                     APP_MQTT_TOPIC_ROOT, device_id, (unsigned)id,
                     (double)min_v, (double)max_v, (double)step);
    } else {
        n = snprintf(buf, cap,
                     "{\"name\":\"%s\",\"unique_id\":\"otc6_%s_ot_%u\","
                     "\"state_topic\":\"%s%s/ot/%u/state\","
                     "\"command_topic\":\"%s%s/ot/%u/set\","
                     "\"step\":%.2f,\"mode\":\"box\"",
                     name ? name : "OT", device_id, (unsigned)id,
                     APP_MQTT_TOPIC_ROOT, device_id, (unsigned)id,
                     APP_MQTT_TOPIC_ROOT, device_id, (unsigned)id,
                     (double)step);
    }
    if (n < 0 || (size_t)n >= cap) {
        return -1;
    }
    int m = append_device_and_avail(buf, cap, n, device_id);
    return (m < 0) ? -1 : n + m;
}

int mqtt_discovery_build_switch_config(char *buf, size_t cap, const char *device_id,
                                       uint8_t id, const char *name)
{
    int n = snprintf(buf, cap,
                     "{\"name\":\"%s\",\"unique_id\":\"otc6_%s_ot_%u\","
                     "\"state_topic\":\"%s%s/ot/%u/state\","
                     "\"command_topic\":\"%s%s/ot/%u/set\","
                     "\"payload_on\":\"1\",\"payload_off\":\"0\"",
                     name ? name : "OT", device_id, (unsigned)id,
                     APP_MQTT_TOPIC_ROOT, device_id, (unsigned)id,
                     APP_MQTT_TOPIC_ROOT, device_id, (unsigned)id);
    if (n < 0 || (size_t)n >= cap) {
        return -1;
    }
    int m = append_device_and_avail(buf, cap, n, device_id);
    return (m < 0) ? -1 : n + m;
}

int mqtt_discovery_build_status_flag_binary(char *buf, size_t cap, const char *device_id,
                                            const char *object_suffix, const char *name,
                                            const char *state_topic)
{
    int n = snprintf(buf, cap,
                     "{\"name\":\"%s\",\"unique_id\":\"otc6_%s_%s\","
                     "\"state_topic\":\"%s\",\"payload_on\":\"ON\",\"payload_off\":\"OFF\"",
                     name, device_id, object_suffix, state_topic);
    if (n < 0 || (size_t)n >= cap) {
        return -1;
    }
    int m = append_device_and_avail(buf, cap, n, device_id);
    return (m < 0) ? -1 : n + m;
}

int mqtt_discovery_build_update_config(char *buf, size_t cap, const char *device_id)
{
    int n = snprintf(buf, cap,
                     "{\"name\":\"Firmware\",\"unique_id\":\"otc6_%s_fw\","
                     "\"device_class\":\"firmware\","
                     "\"state_topic\":\"%s%s/update/state\","
                     "\"command_topic\":\"%s%s/update/set\","
                     "\"payload_install\":\"%s\"",
                     device_id,
                     APP_MQTT_TOPIC_ROOT, device_id,
                     APP_MQTT_TOPIC_ROOT, device_id,
                     OTA_PAYLOAD_INSTALL);
    if (n < 0 || (size_t)n >= cap) {
        return -1;
    }
    int m = append_device_and_avail(buf, cap, n, device_id);
    return (m < 0) ? -1 : n + m;
}

static void publish_config(const char *component, const char *object_id, const char *json)
{
    char topic[160];
    snprintf(topic, sizeof(topic), "homeassistant/%s/%s/config", component, object_id);
    mqtt_ha_publish(topic, json, 1, true);
}

esp_err_t mqtt_discovery_publish_state(const char *device_id, uint8_t id, const char *state_str)
{
    char topic[96];
    snprintf(topic, sizeof(topic), "%s%s/ot/%u/state", APP_MQTT_TOPIC_ROOT, device_id, (unsigned)id);
    return mqtt_ha_publish(topic, state_str ? state_str : "", 0, true);
}

typedef struct {
    const char *suffix;
    const char *name;
    bool on;
    bool is_switch;
} status_flag_proj_t;

static void status_flag_projections(uint8_t master_hb, uint8_t slave_lb,
                                    status_flag_proj_t out[5])
{
    out[0] = (status_flag_proj_t){ "fault", "Fault", ot_codec_flag8_get(slave_lb, 0), false };
    out[1] = (status_flag_proj_t){ "ch_active", "CH active", ot_codec_flag8_get(slave_lb, 1), false };
    out[2] = (status_flag_proj_t){ "dhw_active", "DHW active", ot_codec_flag8_get(slave_lb, 2), false };
    out[3] = (status_flag_proj_t){ "flame", "Flame", ot_codec_flag8_get(slave_lb, 3), false };
    out[4] = (status_flag_proj_t){ "ch_enable", "CH enable", ot_codec_flag8_get(master_hb, 0), true };
}

esp_err_t mqtt_discovery_publish_status_flag_states(const char *device_id,
                                                    uint8_t master_hb,
                                                    uint8_t slave_lb)
{
    char topic[96];
    status_flag_proj_t flags[5];
    status_flag_projections(master_hb, slave_lb, flags);

    for (size_t i = 0; i < 5; i++) {
        snprintf(topic, sizeof(topic), "%s%s/status_flag/%s", APP_MQTT_TOPIC_ROOT, device_id,
                 flags[i].suffix);
        mqtt_ha_publish(topic, flags[i].on ? "ON" : "OFF", 0, true);
    }
    return ESP_OK;
}

esp_err_t mqtt_discovery_publish_status_projections(const char *device_id,
                                                    uint8_t master_hb,
                                                    uint8_t slave_lb,
                                                    bool writable_ch_enable)
{
    char topic[96];
    char json[768];
    status_flag_proj_t flags[5];
    status_flag_projections(master_hb, slave_lb, flags);

    mqtt_discovery_publish_status_flag_states(device_id, master_hb, slave_lb);

    for (size_t i = 0; i < 5; i++) {
        snprintf(topic, sizeof(topic), "%s%s/status_flag/%s", APP_MQTT_TOPIC_ROOT, device_id,
                 flags[i].suffix);

        char object_id[64];
        snprintf(object_id, sizeof(object_id), "otc6_%s_%s", device_id, flags[i].suffix);
        if (flags[i].is_switch && writable_ch_enable) {
            /* switch discovery for CH enable uses ot/0/set path via mqtt_commands */
            char cmd[96];
            snprintf(cmd, sizeof(cmd), "%s%s/ot/0/set", APP_MQTT_TOPIC_ROOT, device_id);
            int n = snprintf(json, sizeof(json),
                             "{\"name\":\"%s\",\"unique_id\":\"%s\",\"state_topic\":\"%s\","
                             "\"command_topic\":\"%s\",\"payload_on\":\"1\",\"payload_off\":\"0\"",
                             flags[i].name, object_id, topic, cmd);
            if (n > 0) {
                append_device_and_avail(json, sizeof(json), n, device_id);
                publish_config("switch", object_id, json);
            }
        } else if (!flags[i].is_switch) {
            if (mqtt_discovery_build_status_flag_binary(json, sizeof(json), device_id,
                                                        flags[i].suffix, flags[i].name, topic) > 0) {
                publish_config("binary_sensor", object_id, json);
            }
        }
    }
    return ESP_OK;
}

esp_err_t mqtt_discovery_publish_climate(const char *device_id, float ch_min, float ch_max)
{
    char json[1024];
    char device[256];
    char avail[80];
    mqtt_discovery_build_device_json(device, sizeof(device), device_id, APP_FW_VERSION);
    snprintf(avail, sizeof(avail), "%s%s/status", APP_MQTT_TOPIC_ROOT, device_id);
    snprintf(json, sizeof(json),
             "{\"name\":\"CH Climate\",\"unique_id\":\"otc6_%s_climate\","
             "\"temperature_command_topic\":\"%s%s/ot/1/set\","
             "\"temperature_state_topic\":\"%s%s/ot/1/state\","
             "\"min_temp\":%.1f,\"max_temp\":%.1f,\"temp_step\":0.5,"
             "\"mode_command_topic\":\"%s%s/climate/mode/set\","
             "\"mode_state_topic\":\"%s%s/climate/mode\","
             "\"modes\":[\"off\",\"heat\"],"
             "\"availability_topic\":\"%s\","
             "\"payload_available\":\"online\",\"payload_not_available\":\"offline\","
             "\"device\":%s}",
             device_id,
             APP_MQTT_TOPIC_ROOT, device_id,
             APP_MQTT_TOPIC_ROOT, device_id,
             (double)ch_min, (double)ch_max,
             APP_MQTT_TOPIC_ROOT, device_id,
             APP_MQTT_TOPIC_ROOT, device_id,
             avail, device);
    char object_id[64];
    snprintf(object_id, sizeof(object_id), "otc6_%s_climate", device_id);
    publish_config("climate", object_id, json);
    ESP_LOGI(TAG, "published additive climate");
    return ESP_OK;
}

esp_err_t mqtt_discovery_publish_climate_mode(const char *device_id, bool heat_on)
{
    char topic[96];
    snprintf(topic, sizeof(topic), "%s%s/climate/mode", APP_MQTT_TOPIC_ROOT, device_id);
    return mqtt_ha_publish(topic, heat_on ? "heat" : "off", 0, true);
}

static const char *id_name(uint8_t id)
{
    switch (id) {
    case 0: return "OT 0 Status";
    case 1: return "OT 1 TSet";
    case 3: return "OT 3 Slave config";
    case 14: return "OT 14 Max rel mod";
    case 17: return "OT 17 Rel mod";
    case 25: return "OT 25 Tboiler";
    case 56: return "OT 56 TdhwSet";
    case 57: return "OT 57 MaxTSet";
    default: return NULL;
    }
}

esp_err_t mqtt_discovery_publish_all(const char *device_id, const ot_catalog_t *cat,
                                     float ch_min, float ch_max)
{
    if (!device_id || !cat) {
        return ESP_ERR_INVALID_ARG;
    }
    char json[1024];
    char object_id[64];
    char namebuf[40];

    if (mqtt_discovery_build_boiler_link_config(json, sizeof(json), device_id) > 0) {
        snprintf(object_id, sizeof(object_id), "otc6_%s_boiler_link", device_id);
        publish_config("binary_sensor", object_id, json);
    }

    if (mqtt_discovery_build_update_config(json, sizeof(json), device_id) > 0) {
        snprintf(object_id, sizeof(object_id), "otc6_%s_fw", device_id);
        publish_config("update", object_id, json);
    }

    for (int id = 0; id < OT_CATALOG_MAX_IDS; id++) {
        const ot_catalog_entry_t *e = &cat->ids[id];
        if (e->support != OT_SUPPORT_AVAILABLE) {
            continue;
        }
        const char *nm = id_name((uint8_t)id);
        if (!nm) {
            snprintf(namebuf, sizeof(namebuf), "OT %u", (unsigned)id);
            nm = namebuf;
        }
        snprintf(object_id, sizeof(object_id), "otc6_%s_ot_%u", device_id, (unsigned)id);

        if (e->writable && id != 0) {
            bool has_range = ot_catalog_is_range_checked((uint8_t)id);
            ot_setpoint_bounds_t b = { .min_c = ch_min, .max_c = ch_max };
            if (has_range) {
                ot_catalog_resolve_bounds(cat, (uint8_t)id, ch_min, ch_max, &b);
            }
            if (mqtt_discovery_build_number_config(json, sizeof(json), device_id, (uint8_t)id, nm,
                                                   has_range, b.min_c, b.max_c, 0.5f) > 0) {
                publish_config("number", object_id, json);
            }
        } else {
            const char *unit = (id == 1 || id == 25 || id == 56 || id == 57) ? "°C" : NULL;
            const char *dc = unit ? "temperature" : NULL;
            if (mqtt_discovery_build_sensor_config(json, sizeof(json), device_id, (uint8_t)id, nm,
                                                   unit, dc) > 0) {
                publish_config("sensor", object_id, json);
            }
        }

        /* state: empty if no sample */
        char state[32];
        if (e->has_raw) {
            if (id == 1 || id == 25 || id == 56 || id == 57 || id == 8) {
                ot_codec_format_float(state, sizeof(state), ot_codec_f88_to_float(e->last_raw), true);
            } else {
                ot_codec_format_u16(state, sizeof(state), e->last_raw, true);
            }
        } else {
            state[0] = '\0';
        }
        mqtt_discovery_publish_state(device_id, (uint8_t)id, state);
    }

    if (cat->ids[0].support == OT_SUPPORT_AVAILABLE) {
        mqtt_discovery_publish_status_projections(device_id,
                                                  ot_poll_get_master_status_flags(),
                                                  ot_poll_get_slave_status_flags(),
                                                  cat->ids[0].writable);
    }

    mqtt_ha_publish_boiler_link(ot_poll_boiler_link_healthy());
    ESP_LOGI(TAG, "discovery publish complete");
    return ESP_OK;
}
