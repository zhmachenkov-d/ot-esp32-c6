# Data Model: OpenTherm Wi‑Fi MQTT Gateway

**Feature**: `001-wifi-mqtt-opentherm`  
**Date**: 2026-08-30  
**Spec**: [spec.md](./spec.md) | **Research**: [research.md](./research.md)

## Entities

### GatewayDevice

Single embedded controller instance.

| Field | Type | Notes |
|-------|------|-------|
| `device_id` | string | Stable id derived from MAC (e.g. `aabbccddeeff`) |
| `fw_version` | string | Reported in MQTT `device` / `origin` blocks |
| `wifi_ssid` | string | NVS; cleared on re-provision |
| `mqtt_host` | string | NVS |
| `mqtt_port` | uint16 | NVS; default 1883 |
| `mqtt_username` / `mqtt_password` | string | NVS; optional |
| `mqtt_tls` | bool | NVS; default false |
| `ch_min_c` / `ch_max_c` | float | SoftAP fallback bounds; seeded 10.0 / 90.0 |
| `provisioning` | bool | SoftAP active |
| `mqtt_link` | enum | `up` \| `down` |
| `failsafe` | bool | True when Wi‑Fi or MQTT unavailable per FR-006 |
| `last_accepted_ch_setpoint_c` | float \| null | Held across fail-safe |

**Validation**: MQTT host non-empty when leaving SoftAP; `ch_min_c` < `ch_max_c`; credentials never logged in plaintext.

**Relationships**: Owns one `BoilerLink`, one `SupportedCatalog`, many `DataIdEntity` projections, one `SetpointBounds`.

---

### BoilerLink

OpenTherm master↔slave health, distinct from MQTT availability.

| Field | Type | Notes |
|-------|------|-------|
| `state` | enum | `healthy` \| `unhealthy` |
| `consecutive_failures` | uint | Incremented on failed OT exchange |
| `failure_threshold` | uint | Default **3** |
| `last_success_ms` | timestamp | Monotonic |

**Transitions**:
- `healthy` → `unhealthy` when `consecutive_failures >= failure_threshold`
- `unhealthy` → `healthy` after one successful keepalive/status exchange; reset failure counter
- Single isolated failure does not flip to unhealthy

**HA projection**: `binary_sensor` boiler-link health (see contracts).

---

### SupportedDataId

One OpenTherm Data ID classified for this boiler.

| Field | Type | Notes |
|-------|------|-------|
| `id` | uint8 | 0–127 |
| `support` | enum | `available` \| `unsupported` |
| `readable` | bool | `support=available` → usable as read entity |
| `writable` | bool | Master-write capable for this boiler (see classification below) |
| `last_raw` | uint16 \| null | Last OT data payload |
| `last_status` | enum | ACK / DATA_INVALID / etc. |
| `poll_tier` | enum | `fast` \| `slow` \| `promoted` |
| `ha_component` | enum | `sensor` \| `binary_sensor` \| `number` \| `switch` \| … |
| `unit` / `device_class` | string \| null | HA hints where known |

**Validation**: Unsupported IDs MUST NOT appear as live fabricated HA entities. Catalog from discovery/validation only.

**Writable classification** (must match research §3):

1. `writable` requires `support=available`.
2. OpenTherm directory/class must allow master write for that ID.
3. Then either ID ∈ known write-safe set (v1 minimum: **0**, **1**, plus fixture-listed IDs) **or** safe write-probe ACK (echo last-read raw / documented no-op—never invent values).
4. Otherwise `writable=false` (readable-only when available).

**Relationships**: Many comprise `SupportedCatalog`; each maps to zero or one live `DataIdEntity` (omit if unsupported).

---

### SupportedCatalog

| Field | Type | Notes |
|-------|------|-------|
| `version` | uint | NVS schema version |
| `ids` | SupportedDataId[] | Sorted |
| `validated` | bool | Boot re-probe done |
| `updated_at` | timestamp | |

**Transitions**: empty → discovering → validated; NVS load → validate → maybe rewrite NVS if set changed.

---

### SetpointBounds

Effective CH Control setpoint (ID 1) limits.

| Field | Type | Notes |
|-------|------|-------|
| `min_c` | float | Prefer boiler min-limit ID; else GatewayDevice.ch_min_c |
| `max_c` | float | Prefer boiler max-limit ID (e.g. 57); else ch_max_c |
| `source_min` | enum | `boiler` \| `config` |
| `source_max` | enum | `boiler` \| `config` |

**Validation**: Commands with `value < min_c` or `value > max_c` → **Reject** (no OT write).

---

### WritableCommand

Inbound HA → gateway write intent.

| Field | Type | Notes |
|-------|------|-------|
| `data_id` | uint8 | Must be catalog-writable |
| `value` | typed | Decoded per ID encoding |
| `received_at` | timestamp | |
| `outcome` | enum | `accepted` \| `rejected_range` \| `rejected_failsafe` \| `rejected_link` \| `ot_failed` |

**Rules**:
- If `failsafe` or boiler-link unhealthy policy requires: do not claim success; reflect failure/unchanged
- ID 1: range check against `SetpointBounds` before OT write
- Serialize onto OT bus; must not starve keepalive

---

### SetpointRejectionSignal

Explicit operator-visible rejection (FR-013).

| Field | Type | Notes |
|-------|------|-------|
| `data_id` | uint8 | Typically 1 |
| `attempted_value` | float | |
| `min_c` / `max_c` | float | Bounds at reject time |
| `reason` | string | e.g. `out_of_range` |
| `at` | timestamp | |

---

### FailSafeState

| Field | Type | Notes |
|-------|------|-------|
| `active` | bool | |
| `entered_at` | timestamp | |
| `held_ch_setpoint_c` | float | Last accepted |
| `remote_writes_allowed` | bool | False while active |

**Transitions**:
- Link loss (Wi‑Fi STA disconnect/lost-IP or MQTT disconnect/error) starts entry timer → `active` within 10 s if still down (SC-004)
- Link recovery (Wi‑Fi+MQTT healthy) + debounce → inactive; optional single retained ID 1 apply; then accept live writes

---

## State overview

```text
[Provisioning SoftAP]
        │ credentials saved
        ▼
[STA + MQTT connecting] ──loss──► [FailSafe: OT alive, hold CH, refuse writes]
        │ up                              │
        ▼                                 │ recover
[Operational: discover/poll OT] ◄─────────┘
        │
        ├─► Catalog validated → MQTT Discovery publish
        ├─► Reads → sensors update (SC-001)
        └─► Writes → validate → OT → reflect (SC-002) or Reject signal
```

## Encoding notes

- Temperatures: OpenTherm f8.8 ↔ °C float for MQTT JSON/state strings (see `knowledge/opentherm` data encoding).
- Status flags (ID 0): bitfields mapped to binary_sensor / switch projections without losing the underlying ID 0 entity when supported.
- Convenience `climate` (optional) is additive UX only; per-ID entities remain required.
