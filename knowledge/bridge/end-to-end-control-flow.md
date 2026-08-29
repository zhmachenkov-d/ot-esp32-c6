---
type: Playbook
title: End-to-End Control Flow
description: Zigbee join, multi-channel Thermostat callbacks, tiered OpenTherm poll engine, and layered ZCL routing.
tags: [bridge, control-flow]
timestamp: 2026-07-02T00:00:00Z
---

How the **Bridge** firmware connects Zigbee cluster commands to OpenTherm master traffic and exposes boiler readings on Zigbee.

## Startup sequence

1. NVS + Zigbee stack init (`zb_ot_bridge`)
2. `ot_catalog_init()` — load or prepare discovery catalog
3. `ot_bridge_init()` — default CH on, water setpoints 60°C; `ot_poll_init()`
4. OpenTherm master init on GPIO12/13 (configurable via menuconfig)
5. `ot_catalog_start()` — discovery or boot validation (background)
6. `zb_ot_bridge_start()` — register endpoints (ep 10–12 thermostats, ep 20 spillover, ep 21–29 discovery), defer join until OT precheck
7. `ot_bridge_start()` — `ot_poll` task after catalog validated + OT precheck done

## Runtime paths

### Inbound (Zigbee → boiler)

| ZCL input | Handler | OpenTherm action |
|-----------|---------|------------------|
| OccupiedHeatingSetpoint write (ep 10/11/12) | `ot_bridge_on_heating_setpoint` | ID 1 / 8 / 56 WRITE |
| SystemMode Heat / Off (ep 10/11/12) | `ot_bridge_on_system_mode` | ID 0 enable bit for that channel |
| MaxHeatSetpointLimit write (ep 10 only) | `ot_zcl_route_apply_write` | ID 57 WRITE |
| Discovery endpoint / spillover write | `ot_zcl_route_apply_write` | Master-writable ID WRITE + `ot_poll_promote` |

### Outbound (boiler → Zigbee)

| OpenTherm read | Handler | ZCL output |
|----------------|---------|------------|
| All available routed IDs | `ot_poll_task` → `ot_zcl_route_apply_read` | Thermostat channels, discovery endpoints, or spillover |
| ID 0 keepalive | `opentherm_send_status_keepalive` | Maintains OT link (CH + DHW + CH2 master flags) |
| ID 0 slave READ | `poll_and_route_id(0)` every fast tick | ThermostatRunningState on ep 10, 11, 12 |

Layered routing details: [Endpoint Layout](/bridge/endpoint-layout.md), [Spillover Encoding](/bridge/spillover-encoding.md), [Water Setpoint Mapping](/bridge/water-setpoint-mapping.md), [Local Temperature Mapping](/bridge/local-temperature-mapping.md).

## Roles

| Component | Role |
|-----------|------|
| ESP32-C6 | OpenTherm **master** + Zigbee **End Device** (3× Thermostat + sensors + spillover) |
| Boiler | OpenTherm **slave** |
| Coordinator | ZCL client (Home Assistant / ZHA / Zigbee2MQTT) |

## Verification

1. Serial: `Joined network successfully`
2. Coordinator discovers device (`OT-ZB-Bridge-v2`, ep 10–12 + ep 20 + dynamic discovery)
3. Serial: ~1 Hz Tboiler / LocalTemperature updates on CH1
4. Setpoint change from HA on each climate entity updates the matching OT ID
5. Spillover entities visible for OEM/diagnostic IDs (e.g. ID 7)

# Citations

[1] `main/ot_bridge.c` — Thermostat write callbacks
[2] `main/ot_poll.c` — poll task
[3] `main/zb_ot_bridge.c` — Zigbee device model
[4] `main/ot_zcl_route.c` — layered routing
[5] [OpenTherm Protocol](/opentherm/opentherm-protocol.md)
