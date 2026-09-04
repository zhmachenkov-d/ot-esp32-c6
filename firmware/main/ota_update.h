#pragma once

/**
 * Dual-slot HTTPS OTA from GitHub Releases manifest + HA update entity helpers.
 * Pure logic (parse/semver/poll/confirm/state JSON) is host-testable under HOST_TEST.
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

#define OTA_MANIFEST_URL_MAX        256
#define OTA_VERSION_MAX             24
#define OTA_SHA256_HEX_LEN          64
#define OTA_TITLE_MAX               64
#define OTA_SUMMARY_MAX             192

typedef struct {
    int manifest_version;
    char firmware_id[32];
    char version[OTA_VERSION_MAX];
    char url[OTA_MANIFEST_URL_MAX];
    bool has_sha256;
    char sha256[OTA_SHA256_HEX_LEN + 1];
    bool has_size;
    int64_t size;
    char release_url[OTA_MANIFEST_URL_MAX];
    char title[OTA_TITLE_MAX];
    char summary[OTA_SUMMARY_MAX];
} ota_manifest_t;

typedef struct {
    char installed_version[OTA_VERSION_MAX];
    char latest_version[OTA_VERSION_MAX];
    char release_url[OTA_MANIFEST_URL_MAX];
    char title[OTA_TITLE_MAX];
    char summary[OTA_SUMMARY_MAX];
    bool in_progress;
    bool has_percentage;
    int update_percentage; /* 0..100 when has_percentage */
    bool failed;
} ota_progress_state_t;

/** Parse MAJOR.MINOR.PATCH only; returns false if not strict semver. */
bool ota_semver_parse(const char *s, int *maj, int *min, int *pat);

/**
 * Numeric triple compare. Returns >0 if a newer than b, <0 if older, 0 if equal.
 * Unparsable either side → treated as not-newer (returns <= 0 vs installed).
 */
int ota_semver_cmp(const char *a, const char *b);

/** True if hex is exactly 64 lowercase hex digits. */
bool ota_sha256_hex_valid(const char *hex);

/**
 * Parse manifest_version 1 JSON. Rejects missing required fields, firmware_id
 * mismatch, non-https url, bad sha256 format. Ignores unknown keys.
 * expected_firmware_id must be non-NULL (e.g. OTA_FIRMWARE_ID).
 */
bool ota_manifest_parse(const char *json, const char *expected_firmware_id, ota_manifest_t *out);

bool ota_manifest_is_newer(const ota_manifest_t *m, const char *installed_version);

/**
 * Poll policy B: suppress SoftAP / in-flight OTA; require operational MQTT session;
 * honor min gap; allow MQTT-ready trigger or periodic interval.
 */
bool ota_should_poll_manifest(bool softap_active, bool ota_in_flight, bool mqtt_session_ready,
                              bool mqtt_ready_trigger, uint32_t now_ms, uint32_t last_poll_ms,
                              uint32_t poll_interval_ms, uint32_t min_interval_ms);

bool ota_should_start_install(const char *installed_version, const char *latest_version,
                              bool has_cached_url, bool softap_active, bool already_in_progress);

/** Pending-only confirm gate: mark valid only when pending_verify && mqtt_session_ready. */
bool ota_should_confirm(bool pending_verify, bool mqtt_session_ready);

bool ota_confirm_timeout_elapsed(bool pending_verify, uint32_t now_ms, uint32_t boot_ms,
                                 uint32_t timeout_ms);

/**
 * Preflight: when has_size is false, accept (size unknown).
 * When present, require 0 <= size <= slot_bytes.
 */
bool ota_size_fits_slot(bool has_size, int64_t size, size_t slot_bytes);

/** Build HA update state JSON. Returns bytes written or -1. */
int ota_build_state_json(char *buf, size_t cap, const ota_progress_state_t *st);

/**
 * Escape src for a JSON string value (quotes/backslashes/controls).
 * Returns bytes written excluding NUL, or -1 on overflow/NULL.
 */
int ota_json_escape(char *dst, size_t cap, const char *src);

bool ota_update_is_install_topic(const char *topic, const char *device_id);

/** Handle HA Install (fixed payload). Host builds provide a recording stub. */
esp_err_t ota_update_handle_install(const char *payload);

#ifndef HOST_TEST

esp_err_t ota_update_init(const char *device_id);

/** SoftAP provisioning: suppress poll and reject Install. */
void ota_update_set_softap_active(bool active);

/** Call when s_mqtt_session_ready becomes true (confirm + poll trigger). */
void ota_update_on_mqtt_session_ready(void);

/** Periodic tick: confirm timeout + 12h poll (pass live s_mqtt_session_ready). */
void ota_update_tick(uint32_t now_ms, bool mqtt_session_ready);

bool ota_update_in_progress(void);

/** Request cancel of in-flight OTA task (non-blocking; OT poll stays responsive). */
void ota_update_cancel(void);

/** Publish current update state JSON on MQTT. */
void ota_update_publish_state(void);

#endif /* !HOST_TEST */

#ifdef __cplusplus
}
#endif
