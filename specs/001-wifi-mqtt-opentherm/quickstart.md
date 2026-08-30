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
2. On first boot, join **open** SoftAP `OTC6-XXXX`, open captive portal, submit Wi‑Fi + MQTT + CH min/max (defaults 10 / 90 °C).
3. Confirm device leaves SoftAP, joins STA, MQTT status `online`.

### MQTT TLS (private CA / self-signed)

Firmware verifies `mqtts` against a **provisioned CA or server PEM**, not the public CA bundle. Use this for Home Assistant Mosquitto with a self-signed cert:

1. Export the broker certificate (leaf is fine if self-signed), e.g. from a LAN host:
   - `openssl s_client -connect <broker-ip>:8883 -showcerts </dev/null 2>/dev/null | openssl x509 -outform PEM`
   - or copy the Mosquitto add-on `certfile` / CA PEM from the HA host
2. SoftAP portal: check **MQTT TLS**, set port **8883** (or your TLS port), paste the PEM into **MQTT CA PEM**, save.
3. Re-provision (GPIO9 ≥5 s) if TLS was enabled earlier without a CA — serial will show `-0x2700` / `Failed to verify peer certificate!` until the PEM is stored.
4. Expect `mqtt_ha: connected` on the monitor.

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

### V5 — Out-of-range / command reject (FR-013 / FR-004)

- **Steps**: Publish out-of-range values for v1 range-checked IDs present in S (at least ID **1**; also **8** if available, and spot-check **7** / **14** / **56** / **57** when writable); optionally force an OT write failure on a non-range-checked writable.
- **Expect**: No out-of-range OT write; reflected value unchanged; `ot/<N>/rejection` fires (`out_of_range`) for range-checked IDs. Other writable failures publish `ot/<N>/rejection` (`ot_failed` / `rejected_failsafe`; optional HA entity not required).

### V6 — Keepalive under load (SC-003, FR-011)

- **Steps**: Generate bursty HA writes across many writables while monitoring OT cadence (logic analyzer, firmware metric, or simulator timestamps).
- **Expect**: ≥1 keepalive/status cycle per second sustained.

### V7 — Fail-safe (SC-004, FR-006)

- **Steps**: Stop broker or drop Wi‑Fi during normal heat demand; observe through the entry timer; restore.
- **Expect**: During the **10 000 ms** entry timer, application availability stays **`online`** and writes may still apply (Option A). After the timer, fail-safe active; application availability presents as **`offline`** (live publish and/or LWT + next-connect birth); OT keepalive continues; last CH setpoint held; remote writes refused (`ot/<N>/rejection` / `rejected_failsafe`); on restore, after **2 s** link-up debounce entities recover; no retained write spiral (at most one retained ID 1).

### V8 — Boiler-link vs MQTT availability (FR-012)

- **Steps**: Break OT adapter path only (Wi‑Fi/MQTT still up) until 3 consecutive **keepalive/status** failures; then restore OT.
- **Expect**: MQTT stays `online`; boiler-link → `unhealthy` then `healthy`; not solely mirrored as generic entity unavailable without the health entity changing.

### V9 — Out-of-scope transports (SC-006)

- **Steps**: Inspect firmware config / image features.
- **Expect**: No Zigbee join/control path; no OpenThread/Thread role.

## Host tests (CI / pre-flash)

Once `firmware/tests/host` exists, run the host suite for: f8.8 encoding, catalog classification fixtures (`firmware/tests/host/fixtures/` incl. mandatory IDs 0/1/3/14/17/25), ID 1 reject bounds, per-ID `ot/<N>/rejection`, discovery JSON shape, Status flag projections (fault/CH/DHW/flame/CH enable), fail-safe FSM (Option A). Prefer these before HIL for regressions.

## Traceability

| Scenario | Spec anchors |
|----------|----------------|
| V1 | US1-A1, FR-005, SC-005 |
| V2 | US1-A2/A5, FR-002/015, SC-007 |
| V3 | US1-A3, FR-003, SC-001 |
| V4 | US2, FR-004, SC-002 |
| V5 | US2-A2/A3, FR-004/013 |
| V6 | FR-011, SC-003 |
| V7 | US3, FR-006, SC-004 |
| V8 | FR-012 |
| V9 | FR-007/008, SC-006 |
