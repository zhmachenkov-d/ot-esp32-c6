#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "esp_err.h"
#include "ot_poll.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    OT_SUPPORT_UNSUPPORTED = 0,
    OT_SUPPORT_AVAILABLE,
} ot_support_t;

typedef enum {
    OT_TIER_SLOW = 0,
    OT_TIER_FAST,
    OT_TIER_PROMOTED,
} ot_poll_tier_t;

typedef enum {
    OT_HA_SENSOR = 0,
    OT_HA_BINARY_SENSOR,
    OT_HA_NUMBER,
    OT_HA_SWITCH,
} ot_ha_component_t;

typedef struct {
    uint8_t id;
    ot_support_t support;
    bool readable;
    bool writable;
    bool has_raw;
    uint16_t last_raw;
    ot_poll_tier_t poll_tier;
    ot_ha_component_t ha_component;
} ot_catalog_entry_t;

typedef struct {
    float min_c;
    float max_c;
    bool from_boiler_min;
    bool from_boiler_max;
} ot_setpoint_bounds_t;

#define OT_CATALOG_MAX_IDS 128

typedef struct ot_catalog {
    uint32_t version;
    bool validated;
    ot_catalog_entry_t ids[OT_CATALOG_MAX_IDS];
} ot_catalog_t;

esp_err_t ot_catalog_init(ot_catalog_t *cat);

/** Classify a read-probe response into available/unsupported (+ writable rules). */
void ot_catalog_classify_read(ot_catalog_entry_t *e, uint8_t id,
                             ot_exchange_result_t result, uint16_t raw,
                             bool directory_writable, bool write_safe_fixture,
                             bool write_probe_ok);

/** Resolve effective bounds for v1 range-checked IDs (1,8,7,14,56,57). */
bool ot_catalog_resolve_bounds(const ot_catalog_t *cat, uint8_t id,
                               float softap_ch_min, float softap_ch_max,
                               ot_setpoint_bounds_t *out);

bool ot_catalog_is_range_checked(uint8_t id);

/** Persist / load via NVS blob helpers. */
esp_err_t ot_catalog_save_nvs(const ot_catalog_t *cat);
esp_err_t ot_catalog_load_nvs(ot_catalog_t *cat);

/** Run discovery probe 0–127 (uses ot_poll_exchange). */
esp_err_t ot_catalog_discover(ot_catalog_t *cat);

const ot_catalog_entry_t *ot_catalog_get(const ot_catalog_t *cat, uint8_t id);

#ifdef __cplusplus
}
#endif
