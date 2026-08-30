# Research: OpenTherm Wi‑Fi MQTT Gateway

**Feature**: `001-wifi-mqtt-opentherm`  
**Date**: 2026-08-30  
**Spec**: [spec.md](./spec.md)

All Technical Context unknowns and integration choices resolved below. Sources: feature clarifications, `.specify/assessments/opentherm-gateway/*`, `knowledge/opentherm/*`, `knowledge/bridge/*` (OT patterns only), ESP-IDF / HA MQTT public docs, prior-art gateways (OTGW-firmware, OTGateway).

---

## 1. OpenTherm stack on ESP-IDF / ESP32-C6

**Decision**: Use `sazanof/opentherm` (^1.0.3 / 1.0.7, MIT, IDF ≥ 5.2) as the primary OpenTherm master component; pin defaults **GPIO in=2 / out=3** on WeAct ESP32-C6 Mini (keeps USB Serial/JTAG on GPIO12/13 free). Treat ESP32-C6 as a **first validation milestone**: if framing/interrupts fail on C6, port Melnyk master timing onto ESP-IDF GPIO+`esp_timer` (same adapter pins) rather than pulling Arduino core.

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

**Decision**: Probe IDs **0–127** with classification matching bridge playbook: `READ-ACK` / `DATA-INVALID` → **available** (expose); `UNKNOWN-DATAID` → unsupported (omit live entity). Persist catalog in NVS; on boot re-validate cache; expose only available IDs. Writable capability: treat available IDs that the OT spec marks as master-writeable **and** that accept a write probe (or are known write-capable when read-ack’d for that ID class) as writable controls—implementation detail in tasks: prefer write-class from OT message directory + safe write probe where needed without inventing values for unsupported IDs.

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
| Readable continuous | `sensor` | ID 25 Tboiler, pressures, modulation |
| Readable flags | `binary_sensor` | flame, fault, CH active bits from ID 0 (when supported) |
| Writable numeric | `number` (min/max/step) | generic writable floats/ints |
| Writable boolean / enable | `switch` | CH enable path via Status write when supported |
| CH water climate UX | optional convenience `climate` for ID 1 + related status | MUST NOT replace per-ID entities (FR-002) |
| CH setpoint reject | `event` or `binary_sensor` diagnostic + status topic | explicit rejection signal (FR-013) |

Publish retained discovery configs after catalog validation; re-publish on reconnect so HA restart recovers entities (HA MQTT discovery guidance).

**Rationale**: Proven by OTGW-firmware / OTGateway; works without custom HA integration (FR-009). Device-based discovery (HA 2024.6+) is a possible later optimization for large catalogs—not required for v1 correctness.

**Alternatives considered**:
- Custom HA Core integration — forbidden (FR-009).
- Manual YAML-only entities — fails SC-005 auto-discovery path.
- Device-discovery-only — newer; keep classic first for broader HA versions.

---

## 5. CH Control setpoint bounds and rejection

**Decision**:
- Effective **max**: boiler CH max-limit Data ID when available (commonly **ID 57**); else SoftAP/NVS value seeded from firmware default **90.0 °C**.
- Effective **min**: boiler CH min-limit ID when offered; else SoftAP/NVS seeded from firmware default **10.0 °C**.
- Out-of-range MQTT write: **reject** — no OT write, keep last-accepted reflected setpoint, publish explicit rejection (MQTT status/event payload with reason `out_of_range` + attempted value).
- SoftAP UI persists operator fallback min/max (FR-014).

**Rationale**: Matches FR-013 clarifications; defaults are conventional CH water bounds and overridable.

**Alternatives considered**: Silent clamp (rejected in clarify); HA-only min/max without reject signal (rejected).

---

## 6. Fail-safe and retained MQTT writes

**Decision**: Within **10 s** of Wi‑Fi or MQTT unavailability (SC-004): enter fail-safe — continue OT keepalive/polling; **hold last commanded CH setpoint** (ID 1); refuse **all** remote Data ID writes until link healthy (FR-006). On recovery: debounce link-up; **apply at most one retained CH setpoint (ID 1)** if present, then follow live commands; ignore retained storms for other writables (do not apply retained non-ID-1 commands automatically).

**Rationale**: Spec Assumptions default; aligns with OTGateway-style emergency thinking without inventing new demand.

**Alternatives considered**: Demand-off on outage (rejected by clarify hold-last policy); apply all retained writables (spiral risk).

---

## 7. SoftAP / captive portal and re-provision

**Decision**: First boot (no credentials) and button-triggered re-provision: start SoftAP (e.g. `OTC6-XXXX`), DNS captive redirect to local HTTP UI (`esp_http_server`) collecting Wi‑Fi SSID/password, MQTT host/port/user/password, optional TLS toggle (off by default for trusted LAN), and CH min/max fallbacks. Persist to NVS; switch to STA; stop SoftAP. **Re-entry**: WeAct user button **GPIO9**, long-press **≥5 s** clears Wi‑Fi/MQTT credentials and forces SoftAP (FR-005). Sustained join/broker failure alone does **not** open SoftAP in v1.

**Rationale**: Spec clarifications; board has SW2 on IO9; 5 s reduces accidental wipe.

**Alternatives considered**: BLE provisioning, serial-only config, auto SoftAP-on-failure — all out for v1 operator UX.

---

## 8. MQTT client / broker assumptions

**Decision**: ESP-MQTT over TCP to operator LAN broker (IPv4). Auth: username/password when configured. TLS optional (v1 default off on trusted LAN). QoS 0 or 1 for state; discovery configs retained; LWT `offline` on broker disconnect.

**Rationale**: Spec Assumptions; ESP-IDF documented path; avoids Thread/NAT64.

**Alternatives considered**: MQTT over Thread / OTBR — killed in assessment; WebSocket-only — unnecessary on LAN.

---

## 9. Boiler-link health threshold

**Decision**: Unhealthy after **3 consecutive failed** OT exchanges at ≥1 Hz keepalive/status cadence (or equivalent ~3 s window); healthy again after **successful exchange(s)** (spec default FR-012). Distinct from MQTT LWT availability.

**Rationale**: Clarification session default; avoids flapping on single glitch.

---

## 10. Project / toolchain

**Decision**: New `firmware/` ESP-IDF 5.4 project, `idf.py set-target esp32c6`, WeAct ESP32-C6 Mini (ESP32-C6FH4). Component manager for `sazanof/opentherm` and `espressif/mqtt`. Host tests under `firmware/tests/host`.

**Rationale**: Matches project hardware default and `knowledge/esp-idf/esp32-c6-get-started.md`; empty production tree today.

**Alternatives considered**: Arduino framework — weaker fit for SoftAP/MQTT/NVS modularity already assumed in IDF knowledge; PlatformIO-only — optional later, not required.

---

## 11. Constitution Zigbee framing

**Decision**: Do not implement Zigbee. Treat constitution Zigbee/ZCL wording as **governance debt**: amend constitution in a follow-up to Wi‑Fi MQTT product language (already noted in spec Assumptions and assessment decision). Until then, apply principles I–V mapped to MQTT entities and OT poll budgets.

**Rationale**: Spec FR-007/008 and go decision Option C.

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
| SoftAP / button | SoftAP portal; GPIO9 ≥5 s |
| Fail-safe / retained | Hold last CH; refuse writes; ≤1 retained ID 1 after debounce |
| CH default min/max | 10.0 / 90.0 °C |
| Topic namespace | `otc6/<device_id>/` |
| Zigbee/Thread | Absent from design |
