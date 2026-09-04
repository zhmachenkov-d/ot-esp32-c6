#pragma once

/**
 * Compile-time defaults for the OpenTherm Wi‑Fi MQTT Gateway.
 * Secrets and SoftAP CH bounds live in NVS; these seed first boot / fallbacks.
 */

#include <stdint.h>

#define APP_FW_VERSION              "0.2.2"

/* OTA — GitHub Releases manifest (public HTTPS CA; not MQTT broker CA) */
#define OTA_FIRMWARE_ID             "otc6_gateway"
#define OTA_GITHUB_REPO             "zhmachenkov-d/ot-esp32-c6"
#define OTA_MANIFEST_URL \
    "https://github.com/" OTA_GITHUB_REPO "/releases/latest/download/manifest.json"
#define OTA_MANIFEST_POLL_INTERVAL_S    43200
#define OTA_MANIFEST_MIN_INTERVAL_S     3600
#define OTA_CONFIRM_TIMEOUT_MS          900000
#define OTA_PAYLOAD_INSTALL             "install"

/* OpenTherm adapter pins (WeAct ESP32-C6 Mini; keep USB Serial/JTAG on 12/13) */
#define APP_OT_GPIO_IN              2
#define APP_OT_GPIO_OUT             3

/* SoftAP re-provision button (WeAct SW2) */
#define APP_SOFTAP_BUTTON_GPIO      9
#define APP_SOFTAP_LONG_PRESS_MS    5000

/* SoftAP / CH setpoint fallback seeds (°C) */
#define APP_CH_MIN_C_DEFAULT        10.0f
#define APP_CH_MAX_C_DEFAULT        90.0f

/* ID 56 (TdhwSet) / ID 57 (MaxTSet) fallbacks when ID 48/49 absent */
#define APP_DHW_SET_MIN_C_DEFAULT   0.0f
#define APP_DHW_SET_MAX_C_DEFAULT   90.0f
#define APP_MAX_TSET_MIN_C_DEFAULT  0.0f
#define APP_MAX_TSET_MAX_C_DEFAULT  90.0f

/* ID 7 (Cooling-control) / ID 14 (Max-rel-mod): 0..100 unless fixture override */
#define APP_PCT_MIN_DEFAULT         0.0f
#define APP_PCT_MAX_DEFAULT         100.0f

/* Boiler-link health: consecutive keepalive/status failures */
#define APP_BOILER_LINK_FAIL_THRESHOLD  3

/* Fail-safe Option A */
#define APP_FAILSAFE_ENTRY_TIMER_MS     10000
#define APP_LINK_UP_DEBOUNCE_MS         2000

/* MQTT topic root (device_id appended) */
#define APP_MQTT_TOPIC_ROOT             "otc6/"

/* OpenTherm poll cadence */
#define APP_OT_KEEPALIVE_INTERVAL_MS    1000
#define APP_OT_INTER_FRAME_GAP_MS       120
#define APP_OT_SLOW_INTERVAL_MS         60000
#define APP_OT_PROMOTE_MS               300000

/* SoftAP SSID prefix; XXXX = device_id suffix */
#define APP_SOFTAP_SSID_PREFIX          "OTC6-"
