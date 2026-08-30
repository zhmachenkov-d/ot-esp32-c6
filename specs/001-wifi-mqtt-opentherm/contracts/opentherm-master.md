# Contract: OpenTherm Master Runtime

**Feature**: `001-wifi-mqtt-opentherm`  
**Date**: 2026-08-30  
**Consumers**: Firmware poll/command modules; boiler (slave) via TTL adapter

## Physical / GPIO

| Signal | Default GPIO | Notes |
|--------|--------------|-------|
| OT adapter in (IRQ) | 12 | Interrupt-capable |
| OT adapter out | 13 | |
| Role | Master | Single boiler slave |

Adapter required (GPIO cannot drive OT bus levels). See `knowledge/bridge/opentherm-gpio-wiring.md`.

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
| READ-ACK | available |
| DATA-INVALID | available (value may be unavailable) |
| UNKNOWN-DATAID | unsupported — no live HA entity |

Probe domain: Data IDs 0–127. Persist + boot-validate in NVS.

## Command path

1. Validate writable + bounds (ID 1) + not fail-safe  
2. Enqueue serialized OT write  
3. Reflect success/failure to MQTT state within SC-002 when attempt completes  
4. Never report success if OT write did not occur

## Fail-safe OT behavior

While fail-safe active: continue keepalive/polling; hold last accepted CH setpoint on the wire per master Status/TSet policy; ignore new remote writes.
