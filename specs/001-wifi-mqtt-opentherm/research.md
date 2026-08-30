# Research: OpenTherm Wi‑Fi MQTT Gateway

**Feature**: `001-wifi-mqtt-opentherm`  
**Date**: 2026-08-30  
**Spec**: [spec.md](./spec.md)

All Technical Context unknowns and integration choices resolved below. Sources: feature clarifications, `.specify/assessments/opentherm-gateway/*`, `knowledge/opentherm/*`, `knowledge/bridge/*` (OT patterns only), ESP-IDF / HA MQTT public docs, prior-art gateways (OTGW-firmware, OTGateway).

---

## 1. OpenTherm stack on ESP-IDF / ESP32-C6

**Decision**: Use `sazanof/opentherm` (^1.0.3 / 1.0.7, MIT, IDF ≥ 5.2) as the primary OpenTherm master component; pin defaults **GPIO in=2 / out=3** on WeAct ESP32-C6 Mini (keeps USB Serial/JTAG on GPIO12/13 free). Treat ESP32-C6 as a **first validation milestone**: if framing/interrupts fail on C6, **stop treating T008 as done** and execute the Melnyk-port task (T008b): port Melnyk master timing onto ESP-IDF GPIO+`esp_timer` (same adapter pins) rather than pulling Arduino core. Do not proceed to user-story OT work on a broken master stack.

**Rationale**: Native IDF dependency surface (driver + esp_timer only); already documented in OKF; helpers cover Status / TSet / Tboiler and broad `MSG_ID_*` set needed for full-catalog probing.

**Alternatives considered**:
- Melnyk Arduino library only — well proven, but Arduino-on-IDF tax for a greenfield IDF app.
- Custom bit-bang from scratch — highest risk; only if both libraries fail on C6.
- Dual Zigbee+OT stack from historical bridge — out of scope (FR-007).

---

## 2. Poll scheduling vs full Data ID catalog

**Decision**: Reuse the documented **fast / slow / promoted** tier model from `knowledge/bridge/poll-tiers.md` (≈1 s fast tick, ≈60 s slow, 120 ms inter-frame gaps, time-budgeted multi-read per tick). Keepalive = master Status (ID 0) + dedicated ID 0 read every fast tick. Writable commands preempt into the next OT slot without dropping the ≥1 Hz keepalive. Promotion on write or value change applies to MQTT-driven updates (replace Zigbee write trigger).

**Rationale**: Spec allows tiered schedules for non-keepalive IDs (FR-011) while forbidding omission of supported IDs (FR-010). Existing knowledge already sizes 5–7 reads/tick under 1 s + 120 ms gaps.

**Alternatives considered**:
- Poll every supported ID every second — starves under large catalogs.
- OTGW-style TCP serial side channel — out of scope; MQTT Discovery only (FR-009).

---

## 3. Data ID support discovery

**Decision**: Probe IDs **0–127** with classification matching bridge playbook: `READ-ACK` / `DATA-INVALID` → **available** (readable expose); `UNKNOWN-DATAID` → unsupported (omit live entity). Persist catalog in NVS; on boot re-validate cache; expose only available IDs.

**Writable classification (normative for FR-004/015/SC-007)**:

1. Start from IDs with `support=available` after read probe.
2. Mark `writable=true` only when **both** hold:
   - The OpenTherm message directory / ID class marks the ID as master-controllable (write-data / write-flag class, **or** ID 0 master Status flags), **and**
   - Either (a) the ID is in the **known write-safe set** (v1: **ID 0** Status master flags including CH enable, **ID 1** Control setpoint, plus any other IDs explicitly listed in `ot_catalog` fixtures as write-safe), **or** (b) a **safe write probe** succeeds: master write of the last-read raw value (or documented no-op bit pattern) returns ACK—never probe with invented setpoints or enable flips.
3. **ID 0 special case (normative)**: Catalog `writable=true` for ID 0 means HA may command **master Status bits** (at least CH enable). The firmware applies those bits on the mandatory Status **`READ-DATA(id=0, master_status, …)`** exchange—**never** `WRITE-DATA(id=0)` (matches `knowledge/opentherm/data-id-0-status.md`). Safe-probe for ID 0 is N/A; treat as write-safe by fixture when Status ACK is observed.
4. If directory says non-writable (and not ID 0), or write probe fails / is skipped outside the known set → `writable=false` (read entity only when available).
5. Never fabricate HA write controls for unsupported IDs.

**Rationale**: FR-015 + existing `discovery-catalog` knowledge; SC-007 needs a known set S on boiler/simulator.

**Alternatives considered**:
- Static compiled allow-list — rejects FR-010 full supported set.
- Fabricate entities for unknown IDs — forbidden by FR-002.

---

## 4. Home Assistant MQTT Discovery shape

**Decision**: Use classic **per-entity MQTT Discovery** under prefix `homeassistant` with a shared `device` block (identifiers = gateway unique id from MAC). Topic root for state/command: `otc6/<device_id>/...` (distinct from commercial OTGW naming). Prefer component types:

| Kind | HA component | Examples |
|------|--------------|----------|
| Gateway online | LWT + `availability_topic` on all entities | `otc6/<id>/status` → `online`/`offline` |
| Boiler-link health | `binary_sensor` | healthy / unhealthy |
| Readable continuous | `sensor` | ID 25 Tboiler, pressures, modulation, u8/u16 codes |
| Readable flag8 / bitfield (whole ID) | `sensor` (raw) **plus** additive `binary_sensor` for documented bits | ID 0 Status raw entity always; flame/fault/CH-active projections when supported |
| Writable numeric | `number` (min/max/step) | generic writable floats/ints; raw flag8 write |
| Writable boolean / enable | `switch` | CH enable via ID 0 master Status flags on Status exchange (not WRITE-DATA) |
| CH water climate UX | optional convenience `climate` for ID 1 + related status | MUST NOT replace per-ID entities (FR-002) |
| CH setpoint reject | status topic `otc6/<id>/ot/1/rejection` (required); optional `event` / diagnostic `binary_sensor` | topic alone satisfies FR-013; entity is additive UX |

Normative encoding→component defaults: `data-model.md` Default HA component map; contract summary in `contracts/mqtt-ha-discovery.md`.

Publish retained discovery configs after catalog validation; re-publish on reconnect so HA restart recovers entities (HA MQTT discovery guidance).

**Rationale**: Proven by OTGW-firmware / OTGateway; works without custom HA integration (FR-009). Device-based discovery (HA 2024.6+) is a possible later optimization for large catalogs—not required for v1 correctness.

**Alternatives considered**:
- Custom HA Core integration — forbidden (FR-009).
- Manual YAML-only entities — fails SC-005 auto-discovery path.
- Device-discovery-only — newer; keep classic first for broader HA versions.

---

## 5. CH Control setpoint bounds and rejection

**Decision**:
- Effective **max**: boiler CH max-limit Data ID when available (commonly **ID 57** / Max CH water setpoint); else SoftAP/NVS value seeded from firmware default **90.0 °C**.
- Effective **min**: SoftAP/NVS seeded from firmware default **10.0 °C**. **v1 assumes no standard OpenTherm Data ID for CH Control setpoint lower limit**; use a boiler min-limit ID only if it is present in the catalog **and** listed in write-safe/bound fixtures (none required at ship). Do not invent a min-limit ID.
- Out-of-range MQTT write: **reject** — no OT write, keep last-accepted reflected setpoint, publish explicit rejection on status topic `otc6/<id>/ot/1/rejection` (reason `out_of_range` + attempted value). Optional HA diagnostic/`event` discovery may mirror that signal later.
- SoftAP UI persists operator fallback min/max (FR-014).

**Rationale**: Matches FR-013 clarifications; ID 57 is the documented max-limit in project knowledge; no project-canonical min-limit ID exists, so SoftAP min is the default lower bound.

**Alternatives considered**: Silent clamp (rejected in clarify); HA-only min/max without reject signal (rejected).

---

## 6. Fail-safe and retained MQTT writes

**Decision**: Wi‑Fi STA disconnect / lost-IP **or** MQTT client disconnect/error starts a fail-safe **entry timer** of **10 000 ms** (default; `app_config`); if the combined link stays unhealthy when the timer expires, enter fail-safe (SC-004) — continue OT keepalive/polling; **hold last commanded CH setpoint** (ID 1); refuse **all** remote Data ID writes until link healthy (FR-006). **Writes remain allowed until fail-safe becomes active.** While fail-safe is active, MQTT availability is **`offline`** (LWT / disconnect)—not `online` with write refusal only. Cancel the timer if Wi‑Fi and MQTT recover before expiry. On recovery: require **2 s** continuous Wi‑Fi STA + MQTT healthy (**link-up debounce**); then **apply at most one retained CH setpoint (ID 1)** if present, then follow live commands; ignore retained storms for other writables (do not apply retained non-ID-1 commands automatically).

**Rationale**: Spec Assumptions default; aligns with OTGateway-style emergency thinking without inventing new demand.

**Alternatives considered**: Demand-off on outage (rejected by clarify hold-last policy); apply all retained writables (spiral risk).

---

## 7. SoftAP / captive portal and re-provision

**Decision**: First boot (no credentials) and button-triggered re-provision: start SoftAP (e.g. `OTC6-XXXX`), **open** SoftAP (no WPA password in v1; physical presence on the SoftAP SSID is the access control), DNS captive redirect to local HTTP UI (`esp_http_server`) collecting Wi‑Fi SSID/password, MQTT host/port/user/password, optional TLS toggle (off by default for trusted LAN), and CH min/max fallbacks. Persist to NVS; switch to STA; stop SoftAP. **Re-entry**: WeAct user button **GPIO9**, long-press **≥5 s** clears Wi‑Fi/MQTT credentials and forces SoftAP (FR-005). Sustained join/broker failure alone does **not** open SoftAP in v1.

**Rationale**: Spec clarifications; board has SW2 on IO9; 5 s reduces accidental wipe.

**Alternatives considered**: BLE provisioning, serial-only config, auto SoftAP-on-failure — all out for v1 operator UX.

---

## 8. MQTT client / broker assumptions

**Decision**: ESP-MQTT over TCP to operator LAN broker (IPv4). Auth: username/password when configured. TLS optional (v1 default off on trusted LAN). QoS 0 or 1 for state; discovery configs retained; LWT `offline` on broker disconnect.

**Rationale**: Spec Assumptions; ESP-IDF documented path; avoids Thread/NAT64.

**Alternatives considered**: MQTT over Thread / OTBR — killed in assessment; WebSocket-only — unnecessary on LAN.

---

## 9. Boiler-link health threshold

**Decision**: Unhealthy after **3 consecutive failed** OT exchanges at ≥1 Hz keepalive/status cadence; healthy again after **one successful** keepalive/status exchange (FR-012). v1 uses consecutive count only (no separate time-window alternative). Distinct from MQTT LWT availability. **Unhealthy does not pre-reject MQTT writes** — still attempt OT (subject to fail-safe + ID 1 range); failure → `ot_failed`, not a `rejected_link` gate (FR-004).

**Rationale**: Clarification session default; avoids flapping on single glitch; keeps command path aligned with “observe non-success” rather than inventing a second refuse mode beside fail-safe.

---

## 10. Project / toolchain

**Decision**: New `firmware/` ESP-IDF 5.4 project, `idf.py set-target esp32c6`, WeAct ESP32-C6 Mini (ESP32-C6FH4). Component manager for `sazanof/opentherm` and `espressif/mqtt`. Host tests under `firmware/tests/host`.

**Rationale**: Matches project hardware default and `knowledge/esp-idf/esp32-c6-get-started.md`; empty production tree today.

**Alternatives considered**: Arduino framework — weaker fit for SoftAP/MQTT/NVS modularity already assumed in IDF knowledge; PlatformIO-only — optional later, not required.

---

## 11. Constitution Zigbee framing

**Decision**: Do not implement Zigbee or Thread. **Resolved (2026-08-30)**: constitution **v2.0.0** retargets principles I–V to Wi‑Fi MQTT / Home Assistant Discovery and OT poll budgets; Zigbee-oriented `knowledge/` playbooks remain historical/reference only. No further constitution amend is required for this feature unless product scope changes.

**Rationale**: Spec FR-007/008 and go decision Option C; governance mismatch closed by constitution MAJOR amend (see Sync Impact Report in `.specify/memory/constitution.md`).

**Alternatives considered**: Dual-radio image — explicitly out of scope.

---

## Resolved NEEDS CLARIFICATION checklist

| Item | Resolution |
|------|------------|
| Language / IDF version | C11, ESP-IDF 5.4, esp32c6 |
| Target board / OT GPIO | WeAct ESP32-C6 Mini; GPIO2 in / GPIO3 out |
| OT library | sazanof first; Melnyk-port fallback |
| Storage | NVS |
| Testing | Host unit + HIL/quickstart |
| HA entity types | Table in §4 |
| SoftAP / button | **Open** SoftAP portal; GPIO9 ≥5 s |
| Fail-safe / retained | Hold last CH; refuse writes; ≤1 retained ID 1 after **2 s** link-up debounce |
| CH default min/max | 10.0 / 90.0 °C |
| Topic namespace | `otc6/<device_id>/` |
| Zigbee/Thread | Absent from design |
