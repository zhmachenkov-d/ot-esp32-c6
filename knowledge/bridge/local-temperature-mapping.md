---
type: Playbook
title: Local Temperature Mapping
description: Per-channel OpenTherm flow/tank temperatures to Zigbee LocalTemperature via layered routing.
tags: [bridge, temperature]
timestamp: 2026-07-02T00:00:00Z
---

Maps channel temperature Data IDs to [LocalTemperature](/zigbee/attribute-local-temperature.md) on the matching Thermostat endpoint.

| Channel | Endpoint | OT Data ID |
|---------|----------|------------|
| CH1 | 10 | 25 Tboiler |
| CH2 | 11 | 31 TflowCH2 |
| DHW | 12 | 26 Tdhw |

## Data flow (CH1 example)

```
ot_poll_task (fast tier, time-budgeted)
  → opentherm_read_id(25, &raw, &status)
  → ot_poll_promote(25) on change
  → ot_zcl_route_apply_read(25, raw, status)
  → zb_ot_bridge_report_local_temperature(10, centi)
```

Reports only when value or validity changes and Zigbee network is joined.

## Encoding

| Layer | Format | Example (45.5°C) |
|-------|--------|------------------|
| OpenTherm | f8.8 | per [OpenTherm Data Encoding](/opentherm/opentherm-data-encoding.md) |
| Firmware | centi-int16 | 4550 |
| Zigbee on-air | int16, 0.01°C units | 4550 |

`DATA_INVALID` maps to ZCL invalid sentinel `0x8000`.

## Semantics

**Boiler flow or tank temperature**, not room air. Label climate entities accordingly in Home Assistant. Channel-mapped IDs do not appear on discovery or spillover endpoints.

# Citations

[1] `main/ot_poll.c` — `poll_and_route_id`
[2] `main/ot_zcl_route.c` — `report_thermostat_channel_read`
[3] `docs/thermostats_data_ids_mapping.md` — channel table
