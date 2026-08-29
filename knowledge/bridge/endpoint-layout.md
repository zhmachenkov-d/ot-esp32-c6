---
type: Playbook
title: Endpoint Layout
description: Zigbee endpoint topology — three Thermostat channels, discovery-driven standard clusters, and spillover.
tags: [bridge, zigbee, endpoints]
timestamp: 2026-07-02T00:00:00Z
---

How the **Bridge** maps available OpenTherm Data IDs onto Zigbee endpoints.

## Fixed thermostat endpoints (10–12)

| Endpoint | ProductLabel | Clusters | OT mapping |
|----------|--------------|----------|------------|
| 10 | CH1 | Basic + Thermostat | IDs 1, 25, 57; ID 0 CH bits |
| 11 | CH2 | Basic + Thermostat | IDs 8, 31; ID 0 CH2 bits |
| 12 | DHW | Basic + Thermostat | IDs 56, 26; ID 0 DHW bits |

All three Thermostat endpoints are always registered. Unsupported channels report invalid sentinels.

ID 0 status is fanned out to all three endpoints (`ThermostatRunningState`). Channel Data IDs: [Water Setpoint Mapping](/bridge/water-setpoint-mapping.md), [Local Temperature Mapping](/bridge/local-temperature-mapping.md).

## Spillover endpoint

| Endpoint | Clusters | When created |
|----------|----------|--------------|
| 20 (configurable) | Basic + spillover `0xFC01` | Always (boot) |

`OT_SPILLOVER_ENDPOINT` defaults to 20 via menuconfig (`CONFIG_OT_SPILLOVER_ENDPOINT`).

## Discovery-driven endpoints (21–29)

For each available Data ID with a **standard cluster mapping** (`zcl_route` Analog In/Out or Multistate), `zb_add_discovery_endpoint()` creates one endpoint:

| `zcl_route` | ZCL cluster |
|-------------|-------------|
| `OT_ROUTE_ANALOG_IN` | Analog Input |
| `OT_ROUTE_ANALOG_OUT` | Analog Output |
| `OT_ROUTE_MULTISTATE` | Multistate Input |

Endpoints are allocated sequentially from `OT_DISCOVERY_EP_MIN` (21) through `OT_DISCOVERY_EP_MAX` (29).

## No-duplication rule

Thermostat-mapped IDs appear on **one** channel endpoint only. Spillover-routed IDs (e.g. ID 7) appear on endpoint 20 only. Standard-mapped IDs are not duplicated on spillover.

## Catalog-driven updates

`zb_ot_bridge_apply_catalog_update()` runs after discovery validation or `RescanCatalog`:

1. Add discovery endpoints for new available standard-mapped IDs
2. Register spillover attributes for new available spillover-routed IDs
3. Poll list rebuilt via `ot_poll_rebuild_list()`

## Device identity

| Attribute | Value |
|-----------|-------|
| Manufacturer | `esp32-c6-opentherm` |
| Model | `OT-ZB-Bridge-v2` |

# Citations

[1] `main/zb_ot_bridge.c` — endpoint factory, spillover registration
[2] `main/zb_ot_bridge.h` — endpoint constants
[3] [Water Setpoint Mapping](/bridge/water-setpoint-mapping.md) — setpoint channel table
[4] [Local Temperature Mapping](/bridge/local-temperature-mapping.md) — temperature channel table
