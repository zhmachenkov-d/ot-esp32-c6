# Quickstart: Validate OpenTherm Wi‑Fi MQTT Gateway

**Feature**: `001-wifi-mqtt-opentherm`  
**Date**: 2026-08-30  
**Contracts**: [mqtt-ha-discovery.md](./contracts/mqtt-ha-discovery.md), [softap-provisioning.md](./contracts/softap-provisioning.md), [opentherm-master.md](./contracts/opentherm-master.md)  
**Data model**: [data-model.md](./data-model.md)

End-to-end checks that prove the feature works. Implementation lives under planned `firmware/` (see [plan.md](./plan.md)); this guide is the validation path, not the build task list.

## Prerequisites

- WeAct ESP32-C6 Mini + OpenTherm TTL adapter wired to GPIO2 (in) / GPIO3 (out)
- Boiler or OT slave simulator with a **known supported-ID set S**
- LAN Wi‑Fi and MQTT broker (e.g. Home Assistant Mosquitto add-on)
- Home Assistant with **MQTT** integration and **discovery enabled**
- Host with ESP-IDF 5.4 toolchain (`idf.py`), USB serial

## Setup

1. Build and flash firmware (target `esp32c6`) from `firmware/` once the app exists:
   - `idf.py set-target esp32c6`
   - `idf.py build flash monitor`
2. On first boot, join SoftAP `OTC6-XXXX`, open captive portal, submit Wi‑Fi + MQTT + CH min/max (defaults 10 / 90 °C).
3. Confirm device leaves SoftAP, joins STA, MQTT status `online`.

## Validation scenarios

### V1 — Commissioning (SC-005, FR-005)

- **Steps**: Power unconfigured board → SoftAP portal → save settings.
- **Expect**: STA + broker connect without serial credentials; HA shows device after discovery publish.
- **Re-provision**: Hold GPIO9 ≥5 s → credentials cleared → SoftAP returns.

### V2 — Discovery coverage (SC-007, FR-002/015)

- **Steps**: After catalog validation, list HA entities for the gateway.
- **Expect**: MQTT availability; boiler-link `binary_sensor`; one read entity per readable ID in S; one write control per writable ID in S; no live entities for IDs outside S.

### V3 — Read freshness (SC-001)

- **Steps**: Change a readable value on boiler/simulator (e.g. ID 25); note OT read success time vs HA UI update.
- **Expect**: HA updates within **5 s** of successful OT read under normal Wi‑Fi/broker.

### V4 — Write round-trip (SC-002, FR-004)

- **Steps**: From HA, set in-range ID 1 (and a Status/CH-enable control if in S).
- **Expect**: OT write occurs; reflected state within **2 s**; CH enable uses Status write path when supported (not only zeroing setpoint).

### V5 — Out-of-range reject (FR-013)

- **Steps**: Publish CH setpoint outside effective min/max.
- **Expect**: No out-of-range OT write; reflected setpoint unchanged; rejection topic/entity fires (`out_of_range`).

### V6 — Keepalive under load (SC-003, FR-011)

- **Steps**: Generate bursty HA writes across many writables while monitoring OT cadence (logic analyzer, firmware metric, or simulator timestamps).
- **Expect**: ≥1 keepalive/status cycle per second sustained.

### V7 — Fail-safe (SC-004, FR-006)

- **Steps**: Stop broker or drop Wi‑Fi during normal heat demand.
- **Expect**: Within **10 s**, fail-safe active; OT keepalive continues; last CH setpoint held; remote writes ignored; on restore, after **2 s** link-up debounce entities recover; no retained write spiral (at most one retained ID 1).

### V8 — Boiler-link vs MQTT availability (FR-012)

- **Steps**: Break OT adapter path only (Wi‑Fi/MQTT still up) until 3 consecutive OT failures; then restore OT.
- **Expect**: MQTT stays `online`; boiler-link → `unhealthy` then `healthy`; not solely mirrored as generic entity unavailable without the health entity changing.

### V9 — Out-of-scope transports (SC-006)

- **Steps**: Inspect firmware config / image features.
- **Expect**: No Zigbee join/control path; no OpenThread/Thread role.

## Host tests (CI / pre-flash)

Once `firmware/tests/host` exists, run the host suite for: f8.8 encoding, catalog classification fixtures, ID 1 reject bounds, discovery JSON shape, fail-safe FSM. Prefer these before HIL for regressions.

## Traceability

| Scenario | Spec anchors |
|----------|----------------|
| V1 | US1-A1, FR-005, SC-005 |
| V2 | US1-A2/A5, FR-002/015, SC-007 |
| V3 | US1-A3, FR-003, SC-001 |
| V4 | US2, FR-004, SC-002 |
| V5 | US2-A2, FR-013 |
| V6 | FR-011, SC-003 |
| V7 | US3, FR-006, SC-004 |
| V8 | FR-012 |
| V9 | FR-007/008, SC-006 |
