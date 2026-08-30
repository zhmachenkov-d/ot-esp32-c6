# Implementation Plan: OpenTherm Wi‑Fi MQTT Gateway

**Branch**: `001-wifi-mqtt-opentherm` | **Date**: 2026-08-30 | **Spec**: [spec.md](./spec.md)

**Input**: Feature specification from `/specs/001-wifi-mqtt-opentherm/spec.md`

**Note**: This template is filled in by the `/speckit-plan` command; its definition describes the execution workflow.

## Summary

Build first-party ESP-IDF firmware for the WeAct ESP32-C6 Mini that acts as an OpenTherm **master**, commissions Wi‑Fi + MQTT via SoftAP/captive portal, and exposes **every boiler-supported readable/writable OpenTherm Data ID** to Home Assistant through MQTT Discovery—plus MQTT availability and a dedicated boiler-link health entity. OpenTherm keepalive (≥1 Hz) stays first-class under HA traffic; on Wi‑Fi/MQTT loss the gateway holds the last CH setpoint, keeps OT alive, and refuses remote writes. Zigbee and OpenThread are out of scope.

Technical approach: native ESP-IDF app (C) with `sazanof/opentherm` (validate on C6; Melnyk-derived timing/knowledge as fallback), ESP-MQTT client, NVS for credentials/catalog/bounds, SoftAP + `esp_http_server` captive portal, tiered OT poll engine adapted from existing bridge knowledge (without Zigbee routing).

## Technical Context

**Language/Version**: C11 via ESP-IDF 5.4 (project default); target `esp32c6`

**Primary Dependencies**: ESP-IDF (Wi‑Fi STA/SoftAP, NVS, FreeRTOS, `esp_http_server`, DNS captive helpers); `espressif/mqtt` (ESP-MQTT); `sazanof/opentherm` ^1.0.3 (native OT master; C6 validation required—see research.md); OpenTherm TTL adapter on GPIO2/3

**Storage**: ESP32 NVS namespaces for Wi‑Fi/MQTT credentials, SoftAP CH min/max fallbacks, OT discovery catalog cache, last-accepted CH setpoint, and device identity

**Testing**: Host-side unit tests (Unity or CMocka via IDF/`idf.py` host tests where practical) for Data ID encoding, setpoint reject/clamp-not, catalog classification, MQTT discovery payload builders, and fail-safe state machine; on-device / HIL scripts for SoftAP commission, MQTT Discovery appearance, OT keepalive under load, and fail-safe (documented in quickstart.md)

**Target Platform**: WeAct ESP32-C6 Mini (ESP32-C6FH4), Wi‑Fi STA + SoftAP; physical OpenTherm adapter; operator LAN MQTT broker (e.g. HA Mosquitto); Home Assistant MQTT Discovery

**Project Type**: Embedded firmware (single ESP-IDF application + components)

**Performance Goals**: OT keepalive/status ≥1 cycle/s (SC-003); readable ID → HA within 5 s of successful OT read (SC-001); writable command → OT attempt + reflected state within 2 s (SC-002); fail-safe entered within 10 s of Wi‑Fi/MQTT loss (SC-004)

**Constraints**: No Zigbee / no OpenThread in delivered image (FR-007/008); SoftAP + button long-press re-provision only (FR-005); reject out-of-range CH setpoint with explicit signal (FR-013); HA messaging must not starve OT (FR-011); secrets never in git; flash/RAM within ESP32-C6FH4 (4 MB) headroom

**Scale/Scope**: Single gateway, single boiler; Data IDs 0–127 discovery → full supported read/write HA exposure (not a fixed small subset); SoftAP settings UI; meta entities (availability, boiler-link health, setpoint rejection diagnostic)

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

| Principle / constraint | Status | Notes |
|------------------------|--------|-------|
| I. Code Quality — domain modular boundaries | PASS | Modules: OT master/poll/catalog, MQTT discovery/state/commands, provisioning UI, fail-safe/link health, NVS config. Names follow Data IDs and MQTT roles, not Zigbee clusters. |
| II. Testing Standards | PASS | Plan requires deterministic tests for encoding, catalog classification, reject bounds, discovery payloads, fail-safe transitions before treating behavior done. |
| III. UX Consistency | PASS (mapped) | Constitution text is Zigbee/ZCL-framed; this feature applies the same rule to **MQTT Discovery entities**: consistent units, unavailable sentinels, availability vs boiler-link separation, versioned breaking topic/entity changes. |
| IV. Performance — OT poll budgets | PASS | Reuse tiered poll + ≥1 Hz keepalive; HA/MQTT work off the critical OT tick; measured budgets in design (research + data-model). |
| V. Simplicity & YAGNI | PASS | No dual-radio, no Thread, no custom HA Core integration, no OTGW TCP serial bridge. Full supported-ID exposure is **spec-mandated**, not speculative. |
| Embedded constraints (`knowledge/` OT/GPIO facts) | PASS | GPIO2/3 (Mini; avoid USB 12/13), adapter required, catalog/poll knowledge reused; Zigbee playbooks are historical only for this feature. |
| Secrets | PASS | Credentials via SoftAP → NVS; `.env`/examples only in repo. |
| Governance mismatch (constitution still Zigbee product language) | JUSTIFIED EXCEPTION | Spec Assumptions + assessment decision already require constitution amendment as governance follow-up. This plan does **not** implement Zigbee. See Complexity Tracking. |

**Gate result (pre-Phase 0)**: PASS WITH JUSTIFIED EXCEPTION (constitution Zigbee framing).

**Gate result (post-Phase 1)**: PASS WITH JUSTIFIED EXCEPTION — design artifacts stay MQTT/Wi‑Fi-only; no Zigbee/Thread interfaces introduced. Constitution amend remains a tracked follow-up outside Phase 1 deliverables.

## Project Structure

### Documentation (this feature)

```text
specs/001-wifi-mqtt-opentherm/
├── plan.md              # This file (/speckit-plan command output)
├── research.md          # Phase 0 output (/speckit-plan command)
├── data-model.md        # Phase 1 output (/speckit-plan command)
├── quickstart.md        # Phase 1 output (/speckit-plan command)
├── contracts/           # Phase 1 output (/speckit-plan command)
└── tasks.md             # Phase 2 output (/speckit-tasks command - NOT created by /speckit-plan)
```

### Source Code (repository root)

Greenfield ESP-IDF application (no production firmware tree yet). Planned layout:

```text
firmware/                        # ESP-IDF project root (idf.py)
├── CMakeLists.txt
├── sdkconfig.defaults
├── main/
│   ├── CMakeLists.txt
│   ├── main.c                   # boot, task wiring
│   ├── app_config.h             # compile-time defaults (CH min/max seeds, thresholds)
│   ├── ot_poll.c / .h           # keepalive + tiered reads/writes
│   ├── ot_catalog.c / .h        # discovery / NVS catalog (patterns from knowledge/bridge)
│   ├── mqtt_ha.c / .h           # ESP-MQTT client, availability, LWT
│   ├── mqtt_discovery.c / .h    # HA discovery publish + entity map
│   ├── mqtt_commands.c / .h     # subscribe, validate, enqueue OT writes
│   ├── failsafe.c / .h          # Wi‑Fi/MQTT loss → hold setpoint, reject writes
│   ├── provision_softap.c / .h  # SoftAP + captive portal + long-press re-entry
│   └── nvs_store.c / .h         # credentials, bounds, identity
├── components/                  # optional local components if split later
│   └── (none required initially — prefer IDF component manager deps)
└── tests/
    ├── host/                    # encoding, catalog, reject, discovery JSON, failsafe FSM
    └── hil/                     # scripts/checklists referenced by quickstart.md

tools/okf/                       # existing knowledge tooling (unchanged)
knowledge/                       # domain facts (OT/GPIO/poll); Zigbee bridge = historical
```

**Structure Decision**: Single ESP-IDF project under `firmware/` at repo root. Keeps Spec Kit / knowledge / assessments separate from flashable code. Tests live under `firmware/tests/` with host-first coverage for protocol/mapping logic.

## Complexity Tracking

| Violation | Why Needed | Simpler Alternative Rejected Because |
|-----------|------------|-------------------------------------|
| Constitution still describes OpenTherm↔Zigbee product / ZCL UX | Product direction is Wi‑Fi MQTT (assessment go + FR-007/008); amending constitution is governance, not blocked by this plan’s MQTT design | Waiting to rewrite constitution before planning would stall delivery; silent Zigbee implementation would violate the feature spec |
| Full boiler-supported Data ID catalog in HA (vs “thin” assessment handoff) | Clarifications overturned fixed small entity set (FR-002/010/SC-007) | Small fixed set no longer meets acceptance; tiered polling addresses bus load without omitting IDs |
