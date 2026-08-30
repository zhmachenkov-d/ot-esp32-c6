# Contract: Home Assistant MQTT Discovery & Runtime Topics

**Feature**: `001-wifi-mqtt-opentherm`  
**Date**: 2026-08-30  
**Consumers**: Home Assistant MQTT integration (discovery enabled); operator broker

Discovery prefix: `homeassistant` (HA default).  
State/command root: `otc6/<device_id>/` where `<device_id>` is stable (MAC-derived hex).

## Device identity (shared `device` block)

```json
{
  "identifiers": ["otc6_<device_id>"],
  "manufacturer": "ot-esp32-c6",
  "model": "WeAct ESP32-C6 Mini OpenTherm Gateway",
  "name": "OpenTherm Gateway",
  "sw_version": "<fw_version>"
}
```

## Availability (MQTT device online)

| Item | Value |
|------|-------|
| Topic | `otc6/<device_id>/status` |
| Birth | `online` (publish on connect, retained recommended) |
| LWT | `offline` |
| Entity field | `availability_topic` → above; `payload_available`=`online`; `payload_not_available`=`offline` |

Boiler-link health MUST NOT reuse this topic as its only signal.

---

## Boiler-link health

**Discovery** (example):

- Topic: `homeassistant/binary_sensor/otc6_<device_id>_boiler_link/config`
- Payload keys: `name`=Boiler link, `unique_id`=`otc6_<device_id>_boiler_link`, `state_topic`=`otc6/<device_id>/boiler_link`, `payload_on`=`healthy`, `payload_off`=`unhealthy`, `device_class`=`connectivity` (or none), shared `device` + `availability_topic`

**State**: `healthy` | `unhealthy` per data-model threshold (default 3 consecutive OT failures).

---

## Per Data ID entities

For each **available** readable ID `N`:

| HA type | When |
|---------|------|
| `sensor` | Continuous / enumerated non-boolean |
| `binary_sensor` | Flag / boolean readable |

- Discovery: `homeassistant/<component>/otc6_<device_id>_ot_<N>/config`
- `unique_id`: `otc6_<device_id>_ot_<N>`
- `state_topic`: `otc6/<device_id>/ot/<N>/state`
- `object_id` / `name`: include Data ID and short OT name when known (e.g. `OT 25 Tboiler`)
- Omit discovery entirely for unsupported IDs

For each **available writable** ID `N`:

| HA type | When |
|---------|------|
| `number` | Numeric writable |
| `switch` | Boolean / enable writable |

- `command_topic`: `otc6/<device_id>/ot/<N>/set`
- `state_topic`: `otc6/<device_id>/ot/<N>/state` (reflected accepted value)
- For ID 1 `number`/`climate`: advertise `min`/`max` from effective `SetpointBounds`

### Optional convenience climate (additive)

- May map CH water setpoint (ID 1) + related status for UX
- MUST NOT suppress per-ID entities for the same Data IDs

---

## CH setpoint rejection signal

**Topic** (status): `otc6/<device_id>/ot/1/rejection`  
**Payload** (JSON example):

```json
{
  "reason": "out_of_range",
  "attempted": 120.0,
  "min": 10.0,
  "max": 90.0,
  "ts": 0
}
```

Optional: also discover `event` or diagnostic `binary_sensor` that toggles/pulses on reject so the operator sees an entity, not only a topic. **Not required for FR-013** — the rejection status topic alone is the v1 bar.

**Invariant**: On reject, `otc6/<device_id>/ot/1/state` remains last accepted value (not the attempted out-of-range value).

---

## Command acceptance rules

| Condition | Broker / HA observation |
|-----------|-------------------------|
| In-range writable, link OK | OT write attempted; state updates ≤2 s (SC-002) |
| ID 1 out of range | No OT write; state unchanged; rejection payload published |
| Fail-safe active | No remote writes applied; state unchanged / unavailable — not false success |
| Boiler OT failure | No false success; boiler-link may go unhealthy after threshold |

---

## Retained writes on reconnect

- After link-up debounce: apply **at most one** retained message on `ot/<1>/set` if present
- Do not auto-apply retained messages for other `ot/<N>/set` topics
