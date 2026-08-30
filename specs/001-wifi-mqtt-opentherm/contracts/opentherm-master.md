# Contract: OpenTherm Master Runtime

**Feature**: `001-wifi-mqtt-opentherm`  
**Date**: 2026-08-30  
**Consumers**: Firmware poll/command modules; boiler (slave) via TTL adapter

## Physical / GPIO

| Signal | Default GPIO | Notes |
|--------|--------------|-------|
| OT adapter in (IRQ) | 2 | WeAct ESP32-C6 Mini default; interrupt-capable |
| OT adapter out | 3 | WeAct ESP32-C6 Mini default (keeps USB Serial/JTAG on 12/13 free) |
| Role | Master | Single boiler slave |

Adapter required (GPIO cannot drive OT bus levels). See `knowledge/bridge/opentherm-gpio-wiring.md`. Historical bridge docs that cite GPIO12/13 are non-normative for this WeAct Mini feature.

## Cadence

| Obligation | Bound |
|------------|-------|
| Keepalive / status cycle | ≥ 1 per second (normal and fail-safe) |
| Inter-frame gap | ≥ 120 ms |
| Fast tick | ~1000 ms with time-budgeted extra reads |
| Slow tier | Default ~60 s for non-fast IDs |
| HA/MQTT work | Must not starve keepalive |

## Discovery classification

| Slave response | Catalog |
|----------------|---------|
| READ-ACK | available (readable) |
| DATA-INVALID | available (value may be unavailable) |
| UNKNOWN-DATAID | unsupported — no live HA entity |

**Writable**: per research §3 / data-model — directory write-class + known write-safe set or safe echo write-probe; never invent values.

Probe domain: Data IDs 0–127. Persist + boot-validate in NVS.

## Command path

1. Validate catalog-writable; if Data ID is in the **v1 range-checked set** (see `mqtt-ha-discovery.md` / FR-013), check bounds → `rejected_range` / `out_of_range` on failure; refuse remote writes **only when `FailSafeState.active`** (entry timer running ≠ active — Option A: writes still allowed during the timer)
2. Enqueue serialized OT command:
   - Most IDs: `WRITE-DATA` (or write-class frame) with encoded value  
   - **ID 0**: update pending master Status flags; apply on next Status **`READ-DATA(id=0)`** keepalive/exchange — **never** `WRITE-DATA(id=0)`  
3. Reflect success/failure to MQTT state within SC-002 when attempt completes  
4. Never report success if the OT command did not occur

## Fail-safe OT behavior

While fail-safe **active** (`FailSafeState.active`): continue keepalive/polling; hold last accepted CH setpoint on the wire per master Status/TSet policy; **refuse** new remote writes (do not apply OT write) and require MQTT `ot/<N>/rejection` with `reason=rejected_failsafe` per FR-004/FR-013 — not a silent drop. During the **entry timer** (link loss detected, not yet active), OT command path remains open for remote writes.
