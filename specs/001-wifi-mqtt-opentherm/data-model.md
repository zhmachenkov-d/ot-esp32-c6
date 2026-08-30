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
| `failsafe` | bool | True when `FailSafeState.active` (entry timer expired while link still down)—**not** merely because Wi‑Fi/MQTT just dropped |
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
- `healthy` → `unhealthy` when `consecutive_failures >= failure_threshold` (**keepalive/status exchanges only** — tiered catalog reads do not increment)
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
2. OpenTherm directory/class must allow master control for that ID (**or** ID 0 master Status flags).
3. Then either ID ∈ known write-safe set (v1: **0**, **1**, plus IDs listed under `firmware/tests/host/fixtures/` as write-safe; OpenTherm mandatory baseline **0, 1, 3, 14, 17, 25** appears in fixtures for read coverage—writable only where directory allows) **or** safe write-probe ACK (echo last-read raw / documented no-op—never invent values). **Exception**: ID 0 is write-safe by fixture when Status ACK is observed; do not `WRITE-DATA` probe ID 0.
4. **ID 0 semantics**: `writable=true` means HA may set master Status bits (CH enable at minimum). Apply via Status **`READ-DATA(id=0)`** carrying master flags—**never** `WRITE-DATA(id=0)`.
5. Otherwise `writable=false` (readable-only when available).

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
| `min_c` | float | SoftAP/firmware `ch_min_c` in v1; override only if a boiler min-limit ID is catalog-available **and** listed under `firmware/tests/host/fixtures/` |
| `max_c` | float | Prefer boiler max-limit ID (e.g. 57); else ch_max_c |
| `source_min` | enum | `boiler` \| `config` (expect `config` unless fixture override) |
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
| `outcome` | enum | `accepted` \| `rejected_range` \| `rejected_failsafe` \| `ot_failed` |

**Rules**:
- If `failsafe` active (`remote_writes_allowed=false`): outcome `rejected_failsafe`; no OT write; publish `ot/<N>/rejection`; no false success
- Boiler-link `unhealthy` does **not** pre-reject: still attempt the OT write (after range/fail-safe checks); on exchange failure → `ot_failed` + `ot/<N>/rejection`; on success → `accepted` (may clear consecutive failures / restore healthy per BoilerLink transitions)
- ID 1: range check against `SetpointBounds` before OT write → `rejected_range` + `ot/1/rejection` if out of bounds
- Serialize onto OT bus; must not starve keepalive
- Non-success outcomes MUST publish on `otc6/<device_id>/ot/<N>/rejection` (same pattern for every writable N)

---

### SetpointRejectionSignal

Explicit operator-visible rejection/failure (FR-004 / FR-013).

| Field | Type | Notes |
|-------|------|-------|
| `data_id` | uint8 | Writable ID `N` |
| `attempted_value` | typed | |
| `min_c` / `max_c` | float \| null | Bounds at reject time when `reason=out_of_range` |
| `reason` | string | `out_of_range` \| `rejected_failsafe` \| `ot_failed` |
| `at` | timestamp | |

**Topic**: `otc6/<device_id>/ot/<N>/rejection`

---

### FailSafeState

| Field | Type | Notes |
|-------|------|-------|
| `active` | bool | |
| `entry_timer_ms` | uint | Default **10 000** (SC-004 / FR-006); from `app_config` |
| `entered_at` | timestamp | |
| `held_ch_setpoint_c` | float | Last accepted |
| `remote_writes_allowed` | bool | False while active |

**Transitions**:
- Link loss (Wi‑Fi STA disconnect/lost-IP or MQTT disconnect/error) starts entry timer (`entry_timer_ms`, default **10 000**); on expiry while still down → `active` (SC-004); cancel timer if Wi‑Fi+MQTT recover before expiry; **remote writes allowed until `active`**; **application availability stays `online` during the timer (Option A)**
- While `active`: application MQTT availability is `offline` (retained publish + LWT); `remote_writes_allowed=false`
- Link recovery: Wi‑Fi+MQTT healthy continuously for **2 s** (link-up debounce) → inactive; optional single retained ID 1 apply; then accept live writes

---

## State overview

```text
[Provisioning SoftAP]
        │ credentials saved
        ▼
[STA + MQTT connecting] ──loss──► [Entry timer 10 s; avail online; writes OK]
        │ up                              │ timer expires, still down
   ───  │                                 ▼
        │                         [Fail-safe active: OT alive, hold CH,
        │                          refuse writes, avail offline]
        │                                 │ recover + 2 s debounce
        ▼                                 │
[Operational: discover/poll OT] ◄─────────┘
        │
        ├─► Catalog validated → MQTT Discovery publish
        ├─► Reads → sensors update (SC-001)
        └─► Writes → validate → OT → reflect (SC-002) or ot/<N>/rejection
```

## Encoding notes

- Temperatures: OpenTherm f8.8 ↔ °C float for MQTT JSON/state strings (see `knowledge/opentherm` data encoding).
- Status flags (ID 0): v1 additive projections — slave **fault**, **CH active**, **DHW active**, **flame**; master **CH enable** `switch` when writable — without losing the underlying ID 0 entity when supported. Master-bit commands use the Status exchange, not `WRITE-DATA(id=0)`.
- Host fixtures baseline (OpenTherm mandatory Data IDs): **0, 1, 3, 14, 17, 25** under `firmware/tests/host/fixtures/`.
- Convenience `climate` (optional) is additive UX only; per-ID entities remain required.

### Default HA component map (v1)

Normative defaults by OpenTherm value class (directory / `knowledge/opentherm`). Fixture overrides MAY change a specific ID; omitting a supported ID is forbidden.

| OT value class | Readable HA component | Writable HA component |
|----------------|----------------------|------------------------|
| Continuous numeric (`f8.8`, `s8.8`, `u8`, `u16`, `s16`, …) | `sensor` | `number` |
| Whole-ID flag8 / bitfield (raw) | `sensor` (raw / numeric state string) | `number` (raw) unless a documented single-bit enable uses `switch` |
| Documented single-bit enable / boolean (e.g. Status CH enable) | additive `binary_sensor` when projected | `switch` |
| Status (ID 0) known flag projections | additive `binary_sensor` / `switch` for **fault, CH active, DHW active, flame, CH enable** only (v1) **in addition to** the ID 0 entity | per T021b / `knowledge/opentherm/data-id-0-status.md` |
| Optional CH `climate` (ID 1 UX) | — | additive only; MUST NOT replace per-ID entities |
