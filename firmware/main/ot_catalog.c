#include "ot_catalog.h"

#include "app_config.h"
#include "nvs_store.h"
#include "ot_codec.h"

#include <string.h>

/* Compact blob: version u32 + validated u8 + 4 bytes × OT_CATALOG_MAX_IDS */
_Static_assert(5 + OT_CATALOG_MAX_IDS * 4 <= NVS_CATALOG_BLOB_MAX,
               "NVS_CATALOG_BLOB_MAX too small for catalog blob");

/* v1 known write-safe set: 0, 1 + fixtures may extend via write_safe_fixture flag */
static bool id_is_directory_writable_default(uint8_t id)
{
    /* Conservative master-write class defaults for common IDs */
    switch (id) {
    case 0:  /* Status master flags via READ exchange */
    case 1:  /* TSet */
    case 7:  /* Cooling control */
    case 8:  /* CH2 TSet */
    case 14: /* Max relative modulation */
    case 56: /* TdhwSet */
    case 57: /* MaxTSet */
        return true;
    default:
        return false;
    }
}

static bool id_in_known_write_safe(uint8_t id)
{
    return id == 0 || id == 1;
}

bool ot_catalog_is_range_checked(uint8_t id)
{
    switch (id) {
    case 1:
    case 7:
    case 8:
    case 14:
    case 56:
    case 57:
        return true;
    default:
        return false;
    }
}

esp_err_t ot_catalog_init(ot_catalog_t *cat)
{
    if (!cat) {
        return ESP_ERR_INVALID_ARG;
    }
    memset(cat, 0, sizeof(*cat));
    cat->version = 1;
    for (int i = 0; i < OT_CATALOG_MAX_IDS; i++) {
        cat->ids[i].id = (uint8_t)i;
        cat->ids[i].support = OT_SUPPORT_UNSUPPORTED;
        cat->ids[i].poll_tier = OT_TIER_SLOW;
        cat->ids[i].ha_component = OT_HA_SENSOR;
    }
    /* Fast-tier defaults for status / common sensors */
    cat->ids[0].poll_tier = OT_TIER_FAST;
    cat->ids[1].poll_tier = OT_TIER_FAST;
    cat->ids[25].poll_tier = OT_TIER_FAST;
    cat->ids[17].poll_tier = OT_TIER_FAST;
    return ESP_OK;
}

void ot_catalog_classify_read(ot_catalog_entry_t *e, uint8_t id,
                             ot_exchange_result_t result, uint16_t raw,
                             bool directory_writable, bool write_safe_fixture,
                             bool write_probe_ok)
{
    if (!e) {
        return;
    }
    e->id = id;
    e->has_raw = false;

    if (result == OT_EXCHANGE_UNKNOWN_ID) {
        e->support = OT_SUPPORT_UNSUPPORTED;
        e->readable = false;
        e->writable = false;
        return;
    }

    if (result == OT_EXCHANGE_OK || result == OT_EXCHANGE_INVALID) {
        e->support = OT_SUPPORT_AVAILABLE;
        e->readable = true;
        if (result == OT_EXCHANGE_OK) {
            e->has_raw = true;
            e->last_raw = raw;
        }

        bool dir_w = directory_writable || id_is_directory_writable_default(id);
        bool safe = write_safe_fixture || id_in_known_write_safe(id) || write_probe_ok;
        /* ID 0: write-safe by fixture/Status ACK — never WRITE-DATA probe */
        if (id == 0) {
            e->writable = dir_w && (write_safe_fixture || id_in_known_write_safe(0));
            e->ha_component = OT_HA_SENSOR; /* raw + additive projections */
        } else {
            e->writable = dir_w && safe;
            e->ha_component = e->writable ? OT_HA_NUMBER : OT_HA_SENSOR;
            if (id == 0) {
                /* unreachable */
            }
        }
        return;
    }

    e->support = OT_SUPPORT_UNSUPPORTED;
    e->readable = false;
    e->writable = false;
}

bool ot_catalog_resolve_bounds(const ot_catalog_t *cat, uint8_t id,
                               float softap_ch_min, float softap_ch_max,
                               ot_setpoint_bounds_t *out)
{
    if (!cat || !out || !ot_catalog_is_range_checked(id)) {
        return false;
    }
    memset(out, 0, sizeof(*out));

    switch (id) {
    case 1:
    case 8: {
        out->min_c = softap_ch_min;
        out->max_c = softap_ch_max;
        out->from_boiler_min = false;
        out->from_boiler_max = false;
        /* Prefer ID 57 MaxTSet as max when available with raw */
        const ot_catalog_entry_t *maxe = &cat->ids[57];
        if (maxe->support == OT_SUPPORT_AVAILABLE && maxe->has_raw) {
            out->max_c = ot_codec_f88_to_float(maxe->last_raw);
            out->from_boiler_max = true;
        }
        return true;
    }
    case 56: {
        out->min_c = APP_DHW_SET_MIN_C_DEFAULT;
        out->max_c = APP_DHW_SET_MAX_C_DEFAULT;
        const ot_catalog_entry_t *lim = &cat->ids[48];
        if (lim->support == OT_SUPPORT_AVAILABLE && lim->has_raw) {
            uint8_t ub, lb;
            ot_codec_unpack_hb_lb(lim->last_raw, &ub, &lb);
            out->max_c = (float)ub;
            out->min_c = (float)lb;
            out->from_boiler_max = true;
            out->from_boiler_min = true;
        }
        return true;
    }
    case 57: {
        out->min_c = APP_MAX_TSET_MIN_C_DEFAULT;
        out->max_c = APP_MAX_TSET_MAX_C_DEFAULT;
        const ot_catalog_entry_t *lim = &cat->ids[49];
        if (lim->support == OT_SUPPORT_AVAILABLE && lim->has_raw) {
            uint8_t ub, lb;
            ot_codec_unpack_hb_lb(lim->last_raw, &ub, &lb);
            out->max_c = (float)ub;
            out->min_c = (float)lb;
            out->from_boiler_max = true;
            out->from_boiler_min = true;
        }
        return true;
    }
    case 7:
    case 14:
        out->min_c = APP_PCT_MIN_DEFAULT;
        out->max_c = APP_PCT_MAX_DEFAULT;
        return true;
    default:
        return false;
    }
}

esp_err_t ot_catalog_save_nvs(const ot_catalog_t *cat)
{
    if (!cat) {
        return ESP_ERR_INVALID_ARG;
    }
    /* Compact: version + 128 entries of {support, writable, has_raw, last_raw hi/lo} */
    uint8_t blob[NVS_CATALOG_BLOB_MAX];
    size_t off = 0;
    if (off + 4 > sizeof(blob)) {
        return ESP_ERR_NO_MEM;
    }
    blob[off++] = (uint8_t)(cat->version & 0xFF);
    blob[off++] = (uint8_t)((cat->version >> 8) & 0xFF);
    blob[off++] = (uint8_t)((cat->version >> 16) & 0xFF);
    blob[off++] = (uint8_t)((cat->version >> 24) & 0xFF);
    blob[off++] = cat->validated ? 1 : 0;

    for (int i = 0; i < OT_CATALOG_MAX_IDS; i++) {
        if (off + 4 > sizeof(blob)) {
            return ESP_ERR_NO_MEM;
        }
        const ot_catalog_entry_t *e = &cat->ids[i];
        uint8_t flags = 0;
        if (e->support == OT_SUPPORT_AVAILABLE) {
            flags |= 0x01;
        }
        if (e->writable) {
            flags |= 0x02;
        }
        if (e->has_raw) {
            flags |= 0x04;
        }
        blob[off++] = flags;
        blob[off++] = (uint8_t)(e->last_raw & 0xFF);
        blob[off++] = (uint8_t)((e->last_raw >> 8) & 0xFF);
        blob[off++] = (uint8_t)e->poll_tier;
    }
    return nvs_store_catalog_save(blob, off);
}

esp_err_t ot_catalog_load_nvs(ot_catalog_t *cat)
{
    if (!cat) {
        return ESP_ERR_INVALID_ARG;
    }
    uint8_t blob[NVS_CATALOG_BLOB_MAX];
    size_t len = 0;
    esp_err_t err = nvs_store_catalog_load(blob, sizeof(blob), &len);
    if (err != ESP_OK) {
        return err;
    }
    if (len < 5) {
        return ESP_ERR_INVALID_SIZE;
    }
    ot_catalog_init(cat);
    size_t off = 0;
    cat->version = (uint32_t)blob[off] | ((uint32_t)blob[off + 1] << 8) |
                   ((uint32_t)blob[off + 2] << 16) | ((uint32_t)blob[off + 3] << 24);
    off += 4;
    cat->validated = blob[off++] != 0;
    for (int i = 0; i < OT_CATALOG_MAX_IDS && off + 4 <= len; i++) {
        uint8_t flags = blob[off++];
        uint16_t raw = (uint16_t)blob[off] | ((uint16_t)blob[off + 1] << 8);
        off += 2;
        uint8_t tier = blob[off++];
        cat->ids[i].support = (flags & 0x01) ? OT_SUPPORT_AVAILABLE : OT_SUPPORT_UNSUPPORTED;
        cat->ids[i].readable = cat->ids[i].support == OT_SUPPORT_AVAILABLE;
        cat->ids[i].writable = (flags & 0x02) != 0;
        cat->ids[i].has_raw = (flags & 0x04) != 0;
        cat->ids[i].last_raw = raw;
        cat->ids[i].poll_tier = (ot_poll_tier_t)tier;
        cat->ids[i].ha_component = cat->ids[i].writable ? OT_HA_NUMBER : OT_HA_SENSOR;
    }
    return ESP_OK;
}

esp_err_t ot_catalog_discover(ot_catalog_t *cat)
{
    if (!cat) {
        return ESP_ERR_INVALID_ARG;
    }
    for (int id = 0; id < OT_CATALOG_MAX_IDS; id++) {
        ot_exchange_t ex = {
            .data_id = (uint8_t)id,
            .request_value = 0,
            .is_write = false,
        };
        if (id == 0) {
            ex.request_value = ot_codec_pack_hb_lb(ot_poll_get_master_status_flags(), 0);
        }
        ot_exchange_result_t r = ot_poll_exchange(&ex);
        bool write_safe = (id == 0 || id == 1);
        ot_catalog_classify_read(&cat->ids[id], (uint8_t)id, r, ex.response_value,
                                 id_is_directory_writable_default((uint8_t)id),
                                 write_safe, false);
        /* Safe echo write-probe for directory-writable non-known-safe with valid raw */
        if (cat->ids[id].support == OT_SUPPORT_AVAILABLE &&
            id_is_directory_writable_default((uint8_t)id) &&
            !write_safe && cat->ids[id].has_raw && id != 0) {
            ot_exchange_t wx = {
                .data_id = (uint8_t)id,
                .request_value = cat->ids[id].last_raw,
                .is_write = true,
            };
            ot_exchange_result_t wr = ot_poll_exchange(&wx);
            bool probe_ok = (wr == OT_EXCHANGE_OK);
            ot_catalog_classify_read(&cat->ids[id], (uint8_t)id, r, ex.response_value,
                                     true, false, probe_ok);
        }
    }
    cat->validated = true;
    return ot_catalog_save_nvs(cat);
}

const ot_catalog_entry_t *ot_catalog_get(const ot_catalog_t *cat, uint8_t id)
{
    if (!cat || id >= OT_CATALOG_MAX_IDS) {
        return NULL;
    }
    return &cat->ids[id];
}
