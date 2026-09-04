---
title: 'Add OTA firmware updates'
type: 'feature'
created: '2026-09-04'
status: 'done'
route: 'dispatch'
review_loop_iteration: 0
context: []
baseline_commit: 'd0debc9414920da56bfb470d5c77131ba11952ef'
---

<frozen-after-approval reason="human-owned intent — do not modify unless human renegotiates">

## Intent

**Problem:** Field devices can only be updated over USB serial flash. There is no over-the-air path, so firmware fixes require physical access.

**Approach:** Add ESP-IDF dual-slot OTA on the WeAct ESP32-C6 Mini (4 MiB flash). The device polls a GitHub Releases `manifest.json` for `latest_version` automation, exposes a Home Assistant MQTT Discovery `update` entity (progress on `state_topic`; fixed Install command), HTTPS-downloads the cached asset URL with public CA roots (not the MQTT broker CA), then confirms the new slot after MQTT is online so bootloader rollback can recover a bad image.

**Decisions:**
- TRIGGER_CHANNEL = B — HA MQTT Discovery `update` entity as sole control plane. Discovery uses a fixed `payload_install`; Install does **not** carry a firmware URL. Device publishes JSON on `state_topic` with at least `installed_version`, `latest_version`, and during OTA `in_progress` / `update_percentage` (plus `release_url` when known).
- UPDATE_SOURCE = C — GitHub Releases. Device periodically GETs `…/releases/latest/download/manifest.json` (compile-time repo/base URL), caches `version` + firmware asset `url` (and `sha256` when present), and drives `latest_version` automation from that. On Install, download the cached asset URL via `esp_https_ota`.
- IMAGE_TRUST = C — HTTPS with **public CA roots** (ESP-IDF default bundle / equivalent). OTA trust anchor is **not** the MQTT broker CA in NVS; NVS/SoftAP OTA CA still deferred. Optional manifest `sha256` verified when present.
- FLASH_LAYOUT = A — Drop `factory`; `otadata` + `ota_0`/`ota_1` reclaim 4 MiB (~1.96 MiB per slot). Build/CI gate: app image size < slot size − **256 KiB**.
- ROLLBACK = B — `CONFIG_BOOTLOADER_APP_ROLLBACK_ENABLE`; mark app valid only after **`s_mqtt_session_ready`** post-reboot (pending-verify only). If still pending after **15 min** (`OTA_CONFIRM_TIMEOUT_MS`), restart to force bootloader rollback.

## Boundaries & Constraints

**Always:**
- Keep OpenTherm fail-safe and MQTT command handling responsive during OTA (cancelable OTA task; do not wedge the OT poll loop).
- Change partitions only via `firmware/partitions.csv` and `firmware/sdkconfig.defaults` (never hand-edit `sdkconfig` / `managed_components` / `dependencies.lock`).
- Publish running `APP_FW_VERSION` as `installed_version` on the HA update entity / device discovery so HA shows the installed version after success.
- One USB flash of the new partition table is required before the first OTA; document it in `firmware/README.md`.
- Document the Release contract in `firmware/README.md`: each GitHub Release that should be offered OTA must include `manifest.json` and the firmware `.bin` asset named by that manifest; CI upload may be manual for v1.

**Never:**
- SoftAP firmware upload, raw MQTT-only OTA command API, or an in-repo release CDN service (see `deferred-work.md`). Hosting firmware on **GitHub Releases** (manifest + `.bin` assets) is in scope.
- Secure Boot / flash encryption.
- Zigbee/Thread OTA stacks.
- Reusing the MQTT broker CA as the OTA trust anchor in this story.
- Compile-time project OTA CA PEM / requiring operators to terminate TLS with a lab CA.

## I/O & Edge-Case Matrix

| Scenario | Input / State | Expected Output / Behavior | Error Handling |
|----------|--------------|---------------------------|----------------|
| Happy path | Manifest offers newer version; HA Install; asset URL reachable; public CA trusts host | Progress on state → download inactive slot → reboot → MQTT up → mark valid; HA shows new `installed_version` | N/A |
| Manifest check | Periodic/boot GET of `manifest.json` succeeds | Cache version+url; publish `latest_version` (and `release_url` when set) | On failure / required-field / `firmware_id` mismatch / bad `manifest_version`: keep last good cache or leave `latest_version` unchanged; no Install of unknown URL |
| Mid-download UX | OTA running | State JSON: `in_progress` true; `update_percentage` when known | On abort: `in_progress` false; failure visible on update entity |
| Reboot gap | Device reboots into new slot | Entity may go unavailable; after MQTT up, discovery/state show new version | N/A |
| Bad URL / TLS / HTTP error | Unreachable host, cert fail, or 4xx/5xx on asset or manifest | Stay on current slot if Install; update entity reports failure | No reboot on Install failure |
| Corrupt / truncated image | Incomplete download, header mismatch, or sha256 mismatch when present | Abort; current slot remains bootable | Failure state on update entity |
| Image too large | Size > inactive slot (or would violate slot−256 KiB policy at build) | Reject; no boot flip | Failure state |
| SoftAP provisioning | Device not in STA+MQTT operation | OTA does not start; manifest poll suppressed until operational | Ignore/reject until operational |
| Poll suppress | SoftAP active or OTA in flight | No manifest GET | Resume after operational / OTA idle, respecting min gap |
| Power loss mid-write | Reset while flashing inactive slot | Bootloader boots last valid app | Old firmware runs |
| Bad new image (crash before confirm) | Reboot into B but MQTT never confirms | Bootloader rolls back to A on crash/reboot without mark | A runs after rollback |
| Confirm timeout (hang, no MQTT) | Image pending verify; `s_mqtt_session_ready` never true for 15 min | Log + `esp_restart()` → bootloader rolls back to A | A runs; README documents window |
| OT/MQTT during OTA | CH commands while download runs | OTA task does not permanently block fail-safe | Cancel or run parallel without wedging OT |
| Install when current | `latest_version` ≤ `installed_version` or no cached URL | OTA does not start | Reject / no-op with failure or ignored Install |

</frozen-after-approval>

## Code Map

- `firmware/partitions.csv` — replace `factory` with `otadata` + `ota_0`/`ota_1` (~1.96 MiB slots on 4 MiB).
- `firmware/sdkconfig.defaults` — custom partition table; enable `CONFIG_BOOTLOADER_APP_ROLLBACK_ENABLE`; public CA / HTTPS client defaults as needed for OTA.
- `firmware/main/app_config.h` — `APP_FW_VERSION`; compile-time GitHub repo/base URL; `OTA_MANIFEST_POLL_INTERVAL_S` (43200), `OTA_MANIFEST_MIN_INTERVAL_S` (3600); `OTA_CONFIRM_TIMEOUT_MS` (900000).
- `firmware/main/ota_update.c` / `.h` (new) — manifest fetch/parse; `esp_https_ota` + `app_update`; start/status/progress; optional sha256; pending-check; `esp_ota_mark_app_valid_cancel_rollback`; confirm-timeout → `esp_restart()`.
- `firmware/main/CMakeLists.txt` — register OTA sources; `REQUIRES` `app_update`, `esp_https_ota`, `esp_http_client`.
- `firmware/main/main.c` — init OTA; schedule manifest poll; when `s_mqtt_session_ready` and image pending verify → mark valid; tick confirm timeout.
- `firmware/main/mqtt_discovery.c` / `.h` — publish HA `update` discovery (fixed `payload_install`) + device `sw_version`.
- `firmware/main/mqtt_ha.c` / `.h` — pub/sub helpers for update command/state JSON (`installed_version`, `latest_version`, progress).
- `firmware/main/nvs_store.c` / `.h` — pattern reference only; do not overload MQTT CA for OTA.
- `firmware/main/provision_softap.c` — unchanged for OTA (no upload, no OTA CA field).
- `firmware/tests/host/` — unit-test manifest parse, reject paths, confirm-gate, progress state helpers separable from IDF.
- `firmware/README.md` — USB partition migration; Release manifest+bin contract; HA update operator steps.

## Tasks & Acceptance

**Execution:**
- [x] `firmware/partitions.csv` + `firmware/sdkconfig.defaults` -- Dual OTA slots + otadata (~1.96 MiB); enable bootloader rollback; size gate slot−256 KiB -- Slot switching foundation.
- [x] `firmware/main/ota_update.c` + `.h` + `firmware/main/CMakeLists.txt` -- Manifest poll/parse; HTTPS OTA with public CA; optional sha256; progress; confirm/cancel APIs -- Core engine.
- [x] `firmware/main/mqtt_discovery.c` + `firmware/main/mqtt_ha.c` -- HA MQTT `update` discovery (fixed Install); state JSON versions + progress -- Operator UX.
- [x] `firmware/main/main.c` + `firmware/main/app_config.h` -- Wire OTA init + manifest base URL; bump `APP_FW_VERSION`; mark valid after MQTT up -- Boot + rollback confirm.
- [x] `firmware/tests/host/` -- Cover I/O matrix pure logic (manifest, reject paths, confirm-before-valid, progress helpers) -- Regression without hardware.
- [x] `firmware/README.md` -- USB reflash; Release manifest+bin contract; HA Install + progress expectations -- Migration clarity.

**Acceptance Criteria:**
- Given partition table flashed and image A running with a Release manifest offering B, when HA Install succeeds, then state shows progress, device reboots to B, confirms after MQTT up, and HA shows B’s `installed_version`.
- Given download/TLS/verify/sha256 failure, when OTA aborts, then boot slot stays A and the update entity reports failure.
- Given power loss mid-write, when device resets, then last valid app boots.
- Given B boots but never reaches MQTT confirm (`s_mqtt_session_ready`), when `OTA_CONFIRM_TIMEOUT_MS` (15 min) elapses, then device restarts and returns to A.
- Given SoftAP provisioning mode, when Install is attempted, then OTA does not run.
- Given a newer manifest on GitHub Releases latest, when the device polls, then HA shows updated `latest_version` without a USB flash.
- Given `firmware/tests/host ./run.sh`, when executed, then new OTA unit tests pass.
- Given a production build, when image size is checked, then size < OTA slot − 256 KiB.

## Implementation Notes

- Slot size is `0x1F0000` (~1.91 MiB) per app partition because OTA app offsets must be 64 KiB-aligned on 4 MiB; size gate uses that slot − 256 KiB.
- Host matrix coverage: manifest parse/reject, poll suppress, SoftAP/install reject, confirm/timeout, progress + abort state JSON, size-vs-slot (`test_ota_update` + discovery update config). Ran: `firmware/tests/host ./run.sh` 11/11.
- Hardware-only matrix rows (power-loss bootloader, reboot gap UX, live TLS/HTTP/sha256-on-device, e2e HA Install, confirm-timeout rollback on silicon) → `firmware/tests/hil/v10_ota.md` (not executed in this session).

## Spec Change Log

- 2026-09-04 — Renegotiated frozen Decisions (party): UPDATE_SOURCE=GitHub Releases + `manifest.json` poll; IMAGE_TRUST=public CA roots (≠ MQTT CA); TRIGGER clarified as fixed Install + state JSON progress/versions; FLASH_LAYOUT dual ~1.96 MiB slots with slot−256 KiB gate. Boundaries/matrix/code map/AC aligned.
- 2026-09-04 — Manifest JSON shape approved (party majority): required `manifest_version`/`firmware_id`/`version`/`url`; optional `sha256`/`size`/`release_url`/`title`/`summary`; semver X.Y.Z compare; ignore unknown keys.
- 2026-09-04 — Manifest poll policy B: 12h periodic + MQTT-up trigger; 1h min gap; suppress during SoftAP and in-flight OTA.
- 2026-09-04 — Confirm gate = `s_mqtt_session_ready` (pending-only mark valid); T1 confirm timeout 15 min → `esp_restart()` for rollback.

## Review Triage Log

- medium | patch | Manifest HTTPS GET runs synchronously on failsafe_task / SoftAP tick — stalls fail-safe up to HTTP timeout. Evidence: `poll_manifest_now` → `http_get_body` from `ota_update_tick` on failsafe loop.
- medium | patch | Same root: concurrent session-ready + tick can race `poll_manifest_now` / `s_cache` without single-flight.
- medium | patch | Pre-reboot retained state leaves `in_progress:true` (pct 100) across reboot gap until new image publishes.
- medium | patch | `ota_manifest_parse` accepts non-semver `version` into cache; Design Notes require X.Y.Z — reject on parse.
- medium | patch | `ota_build_state_json` snprintf optional fields lack remaining-capacity checks → buffer overrun risk.
- medium | patch | State JSON interpolates title/summary/release_url without escaping quotes/backslashes.
- medium | patch | Progress MQTT publishes every ~50 ms in OTA loop can flood broker — throttle.
- medium | patch | Chunked/unknown Content-Length uses fixed 8192 read cap → truncated manifest possible.
- low | patch | `s_failed` never serialized; matrix wants failure visible — emit `failed` when set (HA may ignore; still distinct JSON).
- medium | patch | verification-gap: `mqtt_discovery_publish_all` never asserted for `homeassistant/update/` config.
- medium | patch | verification-gap: Install subscribe/route/retain compiled out under HOST_TEST — no host coverage.
- medium | patch | verification-gap: size-gate `OTA_SLOT_BYTES` not tied to `partitions.csv`.
- low | patch | sdkconfig.defaults comment still says ~1.96 MiB; slots are 0x1F0000 (~1.91 MiB).
- medium | defer | SoftAP `ota_update_set_softap_active` call sites not in host harness — pure helpers covered; wiring needs HIL/harness.
- low | defer | HIL v10 / live TLS/HTTP/sha256-on-device / e2e Install not executed this session.
- low | defer | failsafe→`ota_update_cancel` path lacks dedicated host test.
- false | reject | Updating `s_last_poll_ms` on fetch failure — intentional min-gap between attempts (policy B).
- false | reject | Code Map “provision_softap unchanged” — fix would only edit this build’s spec; SoftAP wiring is correct.
- low | reject | uint32 tick wrap over ~49d pending-verify — not everyday for this device class.
- low | reject | Confirm `s_boot_ms` set on first tick vs `init` — seconds vs 15 min window.
- low | reject | Semver INT_MAX overflow — unrealistic version components.
- maybe-false | defer | `image_pending_verify` on NULL/failed state query — need device path; treat as unverified medium if true.

## Design Notes

Drop `factory`; use standard `ota_0`/`ota_1` reclaiming 4 MiB (~1.96 MiB/slot). HA Update entity is the only control plane: fixed `payload_install`; device owns firmware URL from cached manifest. Compile-time base points at this repo’s Releases latest download path (`…/releases/latest/download/manifest.json`). TLS: public CA bundle, never MQTT NVS CA. Confirm rollback only after MQTT is meaningfully up (same readiness used for discovery/subscriptions), not merely STA associated. CI that uploads bin+manifest may stay manual for v1; README owns the Release contract.

### Manifest JSON (`manifest_version` 1)

**Required:**
- `manifest_version` (number) — must be `1`; higher/unknown → skip update check (do not crash).
- `firmware_id` (string) — must equal compile-time id (e.g. `otc6_gateway`); mismatch → reject manifest.
- `version` (string) — semver `MAJOR.MINOR.PATCH` only (no `-rc` / build metadata in v1); same string space as `APP_FW_VERSION`. Newer-than-installed via numeric triple compare; unparsable → treat as not-newer.
- `url` (string) — absolute `https://` firmware asset URL; non-https → reject.

**Optional:**
- `sha256` (string) — lowercase hex; verify when present; wrong length/format → reject manifest; mismatch after download → abort OTA.
- `size` (number) — bytes; when present, preflight reject if `size` > inactive slot.
- `release_url` (string) — surfaced on HA update entity when present.
- `title` (string) — optional HA title.
- `summary` (string) — optional HA `release_summary`.

**Rules:** Ignore unknown keys (forward compatible). Missing any required field → reject; keep prior good cache if any.

**Example (README / Release asset):**
```json
{
  "manifest_version": 1,
  "firmware_id": "otc6_gateway",
  "version": "0.2.0",
  "url": "https://github.com/ORG/REPO/releases/download/v0.2.0/otc6_gateway.bin",
  "sha256": "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef",
  "size": 1100000,
  "release_url": "https://github.com/ORG/REPO/releases/tag/v0.2.0",
  "title": "OpenTherm Gateway",
  "summary": "OTA support and bugfixes."
}
```

### Manifest poll policy (B)

- **Periodic:** every **12h** (`OTA_MANIFEST_POLL_INTERVAL_S` = 43200) while STA+MQTT operational.
- **On MQTT session ready:** attempt a check (first connect after boot/provision, and later reconnects) subject to min gap.
- **Min gap:** at least **1h** (`OTA_MANIFEST_MIN_INTERVAL_S` = 3600) between attempts — prevents broker-flap storms.
- **Suppress:** no poll during SoftAP provisioning; no poll while an OTA download/install is in flight.
- **Fleet jitter:** not in v1 (single-device assumption).

### Confirm gate & rollback timeout

- **Confirm gate:** call `esp_ota_mark_app_valid_cancel_rollback()` only when the running image is **pending verify** and existing **`s_mqtt_session_ready`** is true (post-`APP_LINK_UP_DEBOUNCE_MS` discovery/subscribe path in `main.c`) — not merely `mqtt_ha_connected()` or STA/IP.
- **Pending-only:** if the image is already valid, do not treat confirm as an error; no-op.
- **Timeout (T1):** `OTA_CONFIRM_TIMEOUT_MS` = **900000** (15 min) from boot while still pending verify without reaching the confirm gate → log and `esp_restart()` so the bootloader rolls back to the previous slot. Not NVS-configurable in v1.
- **README:** document that MQTT/session readiness must succeed within ~15 minutes after an OTA reboot or the update undoes itself (including SoftAP-only / dead-broker cases).

## Verification

**Commands:**
- `cd firmware && idf.py set-target esp32c6 build` -- expected: dual OTA partitions ~1.96 MiB; app size < slot − 256 KiB; rollback Kconfig on
- `cd firmware/tests/host && ./run.sh` -- expected: all tests pass including OTA/manifest cases

**Manual checks:**
- USB flash new table once; publish a GitHub Release with `manifest.json` + `.bin`; device poll shows `latest_version`; HA Install → progress → reboot → version bump; bad/missing asset → failure, no version change; kill MQTT on a deliberately broken B for 15+ min to observe restart rollback to A.
