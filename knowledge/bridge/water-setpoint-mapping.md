---
type: Playbook
title: Water Setpoint Mapping
description: Zigbee OccupiedHeatingSetpoint per Thermostat channel to OpenTherm setpoint Data IDs.
tags: [bridge, setpoint]
timestamp: 2026-07-02T00:00:00Z
---

Maps [OccupiedHeatingSetpoint](/zigbee/attribute-occupied-heating-setpoint.md) writes on each Thermostat endpoint to the channel's OpenTherm setpoint Data ID.

| Endpoint | Channel | OT Data ID |
|----------|---------|------------|
| 10 | CH1 | 1 TSet |
| 11 | CH2 | 8 TsetCH2 |
| 12 | DHW | 56 TdhwSet |

## Data flow (CH1 example)

```
Coordinator Write Attributes (ep 10, 0x0012)
  → ot_bridge_on_heating_setpoint(10, temp_centi_c)
  → s_water_setpoint_c = temp_centi_c / 100.0
  → opentherm_set_boiler_temperature_id(1, °C)
```

## Encoding

| Layer | Format | Example (60°C) |
|-------|--------|----------------|
| Zigbee on-air | int16, 0.01°C units | 6000 |
| Firmware float | °C | 60.0 |
| OpenTherm | f8.8 | per [OpenTherm Data Encoding](/opentherm/opentherm-data-encoding.md) |

## Timing

Setpoint writes trigger `ot_bridge_apply_outputs()` immediately: status (ID 0) then sequential setpoint writes for IDs 1, 8, 56 with **≥120 ms** gaps per [OpenTherm Frame Format](/opentherm/opentherm-frame-format.md).

## Semantics

**Water / DHW temperature targets** — not room air setpoints. Document in Home Assistant climate entity configuration.

# Citations

[1] `main/ot_bridge.c` — `ot_bridge_on_heating_setpoint`, `ot_bridge_apply_outputs`
[2] [Endpoint Layout](/bridge/endpoint-layout.md) — fixed thermostat endpoints 10–12
