---
type: Playbook
title: Spillover Encoding
description: Manufacturer cluster 0xFC01 attribute encoding, invalid sentinels, and spillover-only routing.
tags: [bridge, spillover, encoding]
timestamp: 2026-07-02T00:00:00Z
---

How OpenTherm Data IDs without a standard ZCL mapping are exposed on the spillover cluster.

## Cluster layout

| Property | Value |
|----------|-------|
| Cluster ID | `0xFC01` (`OT_SPILLOVER_CLUSTER_ID`) |
| Endpoint | `CONFIG_OT_SPILLOVER_ENDPOINT` (default 20) |
| Attribute ID | OpenTherm Data ID (0–127) |
| Attribute type | Per OT spec table row (`ot_value_type_t`) |

Each spillover-routed available ID gets one server-side attribute. Attribute access is read-only unless the catalog marks the ID as master-writable.

## Encoding

`ot_spillover_encode()` and `ot_spillover_decode()` in `components/opentherm/ot_encoding.c` convert between OpenTherm raw values and ZCL on-air types:

| OT type | ZCL type | Notes |
|---------|----------|-------|
| f8.8 | int16 (0.01°C) | Via `ot_f88_to_centi_i16` |
| s16 | int16 | Direct |
| u8 / flag8 | uint8 | Direct |
| u16 | uint16 | Direct |
| mixed / special | uint16 raw | Opaque 16-bit payload |

## Invalid sentinel

When OpenTherm returns `DATA_INVALID`, the bridge reports the ZCL invalid sentinel from `ot_zcl_invalid_sentinel()` (typically `0x8000` for temperature types). Coordinators should treat this as "value not available".

## Spillover-only rule

`ot_zcl_route_apply_read()` routes each ID to exactly one surface:

- Standard-mapped IDs → Thermostat or discovery endpoint clusters
- Spillover-routed IDs → spillover cluster only

Standard-mapped IDs never appear as spillover attributes, preventing duplicate entities in Home Assistant.

## Writes

Writable spillover attributes accept ZCL writes, decode via `ot_spillover_decode()`, apply via `ot_zcl_route_apply_write()`, and trigger `ot_poll_promote()` on success.

# Citations

[1] `components/opentherm/ot_encoding.c` — encode/decode, invalid sentinel
[2] `main/ot_zcl_route.c` — routing dispatch
[3] `main/zb_ot_bridge.c` — spillover attribute registration and write handler
