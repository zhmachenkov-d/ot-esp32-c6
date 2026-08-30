#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "esp_err.h"
#include "ot_catalog.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t mqtt_discovery_publish_all(const char *device_id, const ot_catalog_t *cat,
                                     float ch_min, float ch_max);

esp_err_t mqtt_discovery_publish_state(const char *device_id, uint8_t id,
                                       const char *state_str);

/** Additive ID 0 Status flag projections (FR-002 / T021b). */
esp_err_t mqtt_discovery_publish_status_projections(const char *device_id,
                                                    uint8_t master_hb,
                                                    uint8_t slave_lb,
                                                    bool writable_ch_enable);

/** Optional additive climate for ID 1 (T041). */
esp_err_t mqtt_discovery_publish_climate(const char *device_id, float ch_min, float ch_max);

/** Publish climate mode state ("heat" / "off") from CH enable. */
esp_err_t mqtt_discovery_publish_climate_mode(const char *device_id, bool heat_on);

/** Build discovery JSON into buf (host-testable). Returns bytes written or -1. */
int mqtt_discovery_build_device_json(char *buf, size_t cap, const char *device_id,
                                    const char *fw_version);

int mqtt_discovery_build_sensor_config(char *buf, size_t cap, const char *device_id,
                                       uint8_t id, const char *name,
                                       const char *unit, const char *device_class);

int mqtt_discovery_build_boiler_link_config(char *buf, size_t cap, const char *device_id);

int mqtt_discovery_build_number_config(char *buf, size_t cap, const char *device_id,
                                       uint8_t id, const char *name,
                                       float min_v, float max_v, float step);

int mqtt_discovery_build_switch_config(char *buf, size_t cap, const char *device_id,
                                       uint8_t id, const char *name);

int mqtt_discovery_build_status_flag_binary(char *buf, size_t cap, const char *device_id,
                                            const char *object_suffix, const char *name,
                                            const char *state_topic);

#ifdef __cplusplus
}
#endif
