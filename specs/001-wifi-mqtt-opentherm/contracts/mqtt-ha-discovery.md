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
| Birth | `online` (publish on connect, retained recommended) — **except** when fail-safe is already **active** (then birth/`status` MUST be `offline`) |
| LWT | `offline` (broker-side on unclean disconnect; see fail-safe row — **application** policy is Option A) |
| Entity field | `availability_topic` → above; `payload_available`=`online`; `payload_not_available`=`offline` |

Boiler-link health MUST NOT reuse this topic as its only signal.

**Fail-safe Option A (normative)**: On Wi‑Fi/MQTT link loss, start the **10 000 ms** entry timer. While the timer is running and fail-safe is **not** yet active, **application availability MUST be treated as `online`** for write-allowance purposes; if the client can reconnect before expiry, publish retained birth `online` and keep accepting remote writes. When fail-safe **becomes active**, publish retained `offline` on `…/status` and refuse remote writes. Broker LWT may transiently show `offline` during the disconnect itself; that MUST NOT by itself enter fail-safe or permanently refuse writes before timer expiry.

---

## Unavailable / invalid value sentinels

Consistent operator-visible behavior when a value cannot be shown (constitution III):

| Condition | MQTT / HA behavior |
|-----------|-------------------|
| Device MQTT `offline` (LWT / disconnect) | Entities using `availability_topic` become unavailable via HA availability — do not invent fresh OT values |
| Boiler-link `unhealthy` | Keep publishing `boiler_link=unhealthy`; for Data ID state topics that have no valid last sample **or** whose last exchange failed after unhealthy threshold: publish state payload **empty string** (HA treats as unknown/unavailable). Do **not** publish fabricated numeric zeros |
| `DATA-INVALID` / no valid decode yet | Same empty-string state on `ot/<N>/state` until a valid ACK sample exists; discovery entity may remain configured |
| Fail-safe **active** (entry timer expired; Wi‑Fi or MQTT still down) | Application MQTT availability is `offline` (retained publish + LWT); OT keepalive continues locally; commands → `rejected_failsafe` on `ot/<N>/rejection`; reflected states stay last accepted (or empty if never accepted)—not false success |
| Fail-safe **entry timer** running (link loss detected, not yet active) | Application availability remains **`online`** (Option A); remote writes still allowed; cancel timer if Wi‑Fi+MQTT recover before expiry (FR-006) |

Do not use topic-specific one-off unavailable encodings. Empty string is the v1 sentinel for “no valid value” on per-ID state topics.

---

## Boiler-link health

**Discovery** (example):

- Topic: `homeassistant/binary_sensor/otc6_<device_id>_boiler_link/config`
- Payload keys: `name`=Boiler link, `unique_id`=`otc6_<device_id>_boiler_link`, `state_topic`=`otc6/<device_id>/boiler_link`, `payload_on`=`healthy`, `payload_off`=`unhealthy`, `device_class`=`connectivity` (or none), shared `device` + `availability_topic`

**State**: `healthy` | `unhealthy` per data-model threshold (default **3 consecutive failed keepalive/status** exchanges only; tiered catalog reads do not count).

---

## Per Data ID entities

Default component choice follows the **Default HA component map** in `data-model.md` (encoding class → HA type). Summary:

For each **available** readable ID `N`:

| HA type | When |
|---------|------|
| `sensor` | Continuous numeric **or** whole-ID flag8/bitfield raw |
| `binary_sensor` | Additive only for documented flag projections (e.g. Status bits)—not a substitute for omitting the per-ID entity |

- Discovery: `homeassistant/<component>/otc6_<device_id>_ot_<N>/config`
- `unique_id`: `otc6_<device_id>_ot_<N>`
- `state_topic`: `otc6/<device_id>/ot/<N>/state`
- `object_id` / `name`: include Data ID and short OT name when known (e.g. `OT 25 Tboiler`)
- Omit discovery entirely for unsupported IDs

For each **available writable** ID `N`:

| HA type | When |
|---------|------|
| `number` | Continuous numeric writable, or raw flag8 write |
| `switch` | Documented boolean / single-bit enable writable |

- `command_topic`: `otc6/<device_id>/ot/<N>/set`
- `state_topic`: `otc6/<device_id>/ot/<N>/state` (reflected accepted value)
- For ID 1 `number`/`climate`: advertise `min`/`max` from effective `SetpointBounds`

### Optional convenience climate (additive)

- May map CH water setpoint (ID 1) + related status for UX
- MUST NOT suppress per-ID entities for the same Data IDs

---

## Command rejection signal (all writable Data IDs)

**Topic** (status): `otc6/<device_id>/ot/<N>/rejection`  
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

`reason` values (v1): `out_of_range` (ID 1 / range-checked IDs), `rejected_failsafe`, `ot_failed`. Fields `min`/`max` apply when `reason=out_of_range`; otherwise they MAY be omitted.

Optional: also discover `event` or diagnostic `binary_sensor` that toggles/pulses on reject so the operator sees an entity, not only a topic. **Not required for FR-004/FR-013** — the rejection status topic alone is the v1 bar.

**Invariant**: On reject/failure, `otc6/<device_id>/ot/<N>/state` remains last accepted value (not the attempted rejected value; empty string if never accepted).

---

## Command acceptance rules

| Condition | Broker / HA observation |
|-----------|-------------------------|
| In-range writable, fail-safe inactive | OT write **attempted**; state updates ≤2 s on success (SC-002) |
| ID 1 (or range-checked ID) out of range | No OT write; state unchanged; `outcome=rejected_range`; publish `ot/<N>/rejection` with `reason=out_of_range` |
| Fail-safe active | No remote writes applied (`outcome=rejected_failsafe`); application availability `offline`; publish `ot/<N>/rejection`; state unchanged — not false success |
| Boiler-link unhealthy | **Still attempt** OT write; on failure → `ot_failed` + `ot/<N>/rejection`; boiler-link may stay/become unhealthy per keepalive/status threshold |
| OT exchange failure (any link state) | No false success; reflected state unchanged; `outcome=ot_failed` + `ot/<N>/rejection` |

---

## Retained writes on reconnect

- After **2 s** link-up debounce (Wi‑Fi STA + MQTT both healthy continuously): apply **at most one** retained message on `ot/<1>/set` if present
- Do not auto-apply retained messages for other `ot/<N>/set` topics
