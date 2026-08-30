# Feature Specification: OpenTherm Wi‑Fi MQTT Gateway

**Feature Branch**: `features/001-wifi-mqtt-opentherm`

**Created**: 2026-08-29

**Status**: Ready for implementation

**Input**: User description: "thin OT master on ESP32-C6, Wi‑Fi MQTT + Discovery, small entity set, OT cadence + fail-safe; Zigbee and Thread out of scope." (Entity-set scope later expanded in Clarifications: all boiler-supported readable/writable OpenTherm Data IDs.)

**Assessment handoff**: `.specify/assessments/opentherm-gateway/decision.md` (go — Option C)

## Clarifications

### Session 2026-08-29

- Q: How should the operator configure Wi‑Fi and MQTT broker credentials on the gateway? → A: SoftAP / captive-portal web UI for Wi‑Fi + MQTT settings
- Q: When Home Assistant sends a CH water setpoint outside the allowed min/max, should the gateway reject it or clamp it? → A: Reject — do not apply; keep last accepted setpoint; observable failure/unchanged reflected value
- Q: Where do the allowed CH water setpoint min and max values come from? → A: Prefer boiler OpenTherm capability/limit Data IDs when available; otherwise SoftAP-configured min/max seeded from firmware defaults
- Q: Which OpenTherm Data IDs should the initial small entity set use for boiler water temperature and CH water setpoint? → A: Temp = ID 25 (`Tboiler`); setpoint = ID 1 (`Control setpoint`) write + read-back of related status/setpoint IDs as needed for climate state *(superseded for catalog breadth by full supported-ID exposure below; IDs 25 and 1 remain canonical examples within that catalog)*
- Q: Besides MQTT device availability, what boiler “status” must Home Assistant see in the initial entity set? → A: MQTT availability + explicit boiler-link health entity; no extra flame/fault/CH-active sensors in v1 *(superseded: flame/fault/CH-active and other status bits appear when the boiler supports the corresponding readable Data IDs)*
- Q: After first commissioning, how should the operator re-open the SoftAP captive portal to change Wi‑Fi or MQTT settings? → A: Physical button long-press (or equivalent) forces SoftAP / clears credentials into provisioning
- Q: When should the dedicated boiler-link health entity report unhealthy versus healthy? → A: Unhealthy after N consecutive failed OT exchanges (or equivalent short failure window); healthy after one successful exchange
- Q: What should v1 expose in Home Assistant for OpenTherm values and commands? → A: All boiler-supported readable OT Data IDs as HA entities, and operator writes to all boiler-supported writable OT Data IDs (overturns fixed small entity set)
- Q: Which OpenTherm Data IDs should define the effective CH Control setpoint (ID 1) min and max when the boiler reports limits? → A: Prefer boiler max (and min if offered) CH setpoint limit Data IDs when available; else SoftAP/firmware defaults
- Q: When Home Assistant publishes an out-of-range CH Control setpoint, how must the operator observe that the gateway rejected it? → A: Last accepted reflected value plus an explicit rejection signal (MQTT event/status or diagnostic entity)

## User Scenarios & Testing *(mandatory)*

### User Story 1 - See boiler Data IDs in Home Assistant (Priority: P1)

As the home operator, I commission the gateway onto my Wi‑Fi network and MQTT broker so that every OpenTherm Data ID the boiler supports for reading appears automatically in Home Assistant without writing a custom integration.

**Why this priority**: Visibility is the minimum viable value—without live readings, control and fail-safe are meaningless.

**Independent Test**: With a boiler (or boiler simulator) and a Home Assistant instance using MQTT discovery, power an unconfigured gateway, complete SoftAP/captive-portal setup for Wi‑Fi and MQTT, and confirm gateway meta entities plus entities for each boiler-supported readable Data ID appear and update.

**Acceptance Scenarios**:

1. **Given** the gateway has no saved network credentials (or is in provisioning mode), **When** the operator connects to the gateway SoftAP and submits Wi‑Fi plus MQTT broker settings via the captive-portal web UI, **Then** the gateway joins the operator Wi‑Fi, connects to the broker, and leaves provisioning mode without requiring serial/console or compile-time credentials.
2. **Given** the gateway is powered and joined to Wi‑Fi with a reachable MQTT broker, **When** Home Assistant is configured for MQTT discovery, **Then** MQTT device availability, boiler-link health, and one discovered entity per boiler-supported readable OpenTherm Data ID appear without manual YAML entity definitions.
3. **Given** the OpenTherm link to the boiler is healthy, **When** the boiler successfully returns a supported readable Data ID, **Then** that value becomes visible on the corresponding Home Assistant entity within the freshness bound in Success Criteria.
4. **Given** entities already exist from a prior session, **When** the gateway reconnects after a power cycle, **Then** the same logical entities are rediscovered or resume updating without the operator recreating them by hand.
5. **Given** the boiler does not support a particular Data ID, **When** discovery/classification completes, **Then** the gateway does not present that ID as a live readable entity (unsupported IDs are omitted or marked unavailable per documented discovery rules—not fabricated values).

---

### User Story 2 - Change writable OpenTherm Data IDs from Home Assistant (Priority: P1)

As the home operator, I change any OpenTherm Data ID the boiler supports for writing (including CH control setpoint and Status flags such as CH enable) from Home Assistant and the boiler receives the corresponding master write.

**Why this priority**: Two-way control is the other half of the stated product job; monitoring alone is incomplete.

**Independent Test**: From Home Assistant, write an in-range value to a boiler-supported writable Data ID (at least ID 1 Control setpoint and a Status/CH-enable path when supported) and verify the gateway issues the OpenTherm master write and reflected state matches within the command bound.

**Acceptance Scenarios**:

1. **Given** Home Assistant shows a writable entity for a boiler-supported Data ID as available, **When** the operator publishes an allowed value, **Then** the gateway commands the boiler accordingly and the entity reflects the accepted value within the command round-trip bound.
2. **Given** the operator sets a CH water setpoint outside the allowed range, **When** the command is published, **Then** the gateway **rejects** it (does not write an out-of-range value to the boiler), keeps the last accepted setpoint on the reflected entity, and publishes an **explicit rejection signal** (MQTT event/status or diagnostic entity)—not a silently clamped value and not HA-only display of the out-of-range number.
3. **Given** OpenTherm communication is temporarily failing, **When** the operator issues a write to any writable Data ID, **Then** Home Assistant can observe that the command did not take effect (unavailable, error, or unchanged reflected status)—not a false success.
4. **Given** the boiler supports Status (Data ID 0) writes that include CH enable, **When** the operator disables central heating via the corresponding HA control, **Then** the gateway clears CH enable on the OpenTherm master Status exchange (not only by zeroing the setpoint).

---

### User Story 3 - Heating stays defined when Home Assistant is unreachable (Priority: P2)

As a household occupant, if Wi‑Fi or the MQTT broker drops, the boiler must not enter an undefined heating state; the gateway keeps the OpenTherm master link alive and applies the documented fail-safe policy until connectivity returns.

**Why this priority**: Heating safety and comfort under outage are mandatory for a control device, but depend on a working monitor/control path first.

**Independent Test**: Establish normal operation, then disconnect Wi‑Fi or stop the broker; observe OpenTherm keepalive continues and heating follows the fail-safe policy; restore connectivity and confirm remote control resumes cleanly.

**Acceptance Scenarios**:

1. **Given** normal HA control is working, **When** Wi‑Fi or MQTT becomes unavailable, **Then** the gateway continues OpenTherm master keepalive/polling at the required cadence and applies the fail-safe heating policy with no undefined demand.
2. **Given** the gateway is in fail-safe due to link loss, **When** connectivity returns, **Then** entities become available again and new Home Assistant commands are accepted under normal rules.
3. **Given** fail-safe is active, **When** Home Assistant still has stale UI state, **Then** the gateway does not invent heat demand from outdated remote commands received after the outage window without a fresh operator action after recovery (or documents if last-will/retained messages are honored—see Assumptions).

---

### Edge Cases

- Boiler does not acknowledge or support a Data ID: that ID is omitted or shows unavailable/unknown; other supported IDs continue.
- Boiler does not report CH setpoint limit/capability Data IDs: gateway uses SoftAP-configured min/max (firmware-default seeded) for rejection bounds on Control setpoint (ID 1) and for any HA climate min/max advertising tied to that setpoint. When boiler max-limit (e.g. ID 57) is present, that value drives effective max for rejection; min uses boiler min-limit if offered else SoftAP/firmware min.
- MQTT broker accepts connection but Home Assistant discovery is disabled: gateway still publishes state on its topics; operator must enable discovery (documented expectation).
- Rapid or bursty writes from HA across many Data IDs: gateway serializes OpenTherm traffic so keepalive/status polling is not starved; slower-tier readable IDs may refresh less often than keepalive while still meeting SC-001 after each successful read.
- Out-of-range CH setpoint write: reflected setpoint stays at last accepted value; explicit rejection signal is published; boiler is not written.
- Wi‑Fi connected but broker unreachable (DNS/timeout): treated as MQTT unavailable → fail-safe.
- Partial OpenTherm adapter/wiring fault: no false “success” on commands; boiler-link health reflects unhealthy after the consecutive-failure threshold.
- Broker sends retained or birth/LWT storms on reconnect: gateway recovers without locking the boiler into an unintended write spiral across writable Data IDs (see Assumptions).
- Operator needs to change Wi‑Fi/MQTT after commissioning (including after bad credentials): long-press physical button (or equivalent) forces SoftAP provisioning and clears saved credentials; sustained join/broker failure alone does not open SoftAP in v1.
- Transient single OpenTherm exchange failure: boiler-link health stays healthy until the consecutive-failure threshold is met; after threshold, health is unhealthy and Data ID entities may show unavailable until exchanges succeed again.
- Very large supported-ID catalogs: all supported readable/writable IDs remain required; poll scheduling may tier reads, but omitting a supported ID from HA exposure is not an allowed optimization.

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**: The gateway MUST act as OpenTherm **master** toward a single connected boiler (or compatible slave) and maintain master keepalive / status communication at least once per second under normal and degraded HA-link conditions.
- **FR-002**: The gateway MUST expose via MQTT Discovery: MQTT device availability; an explicit **boiler-link healthy/unhealthy** entity; **one Home Assistant entity for every OpenTherm Data ID the connected boiler supports for reading**; and **a writable Home Assistant control for every OpenTherm Data ID the connected boiler supports for writing**. Unsupported Data IDs MUST NOT be presented as live fabricated values. Convenience mappings (e.g. climate UI for Control setpoint ID 1 / `TSet`, sensor for ID 25 / `Tboiler`) MAY be used where they improve HA UX, but MUST NOT replace or hide the requirement to cover the full supported readable/writable set.
- **FR-003**: The gateway MUST publish updates for supported readable Data ID values so Home Assistant shows changes within the freshness bounds in Success Criteria under normal Wi‑Fi and broker conditions (measured from successful OpenTherm read of that ID).
- **FR-004**: The gateway MUST accept commands from Home Assistant over MQTT for every boiler-supported writable Data ID, execute the corresponding OpenTherm write, and reflect success or failure in entity state.
- **FR-005**: The gateway MUST obtain Wi‑Fi connectivity and connect to an operator-configured MQTT broker on the local network; credentials MUST be configurable by the operator (not hard-coded in source) via a **SoftAP / captive-portal web UI** that collects Wi‑Fi and MQTT broker settings. After first commissioning, re-entry to SoftAP provisioning MUST be via a **physical button long-press (or board-equivalent input)** that forces SoftAP mode and clears saved Wi‑Fi/MQTT credentials into provisioning; v1 MUST NOT rely on auto SoftAP-on-failure alone or on MQTT/LAN-only reconfiguration for credential recovery.
- **FR-006**: When Wi‑Fi or MQTT is unavailable, the gateway MUST enter a documented fail-safe mode: continue OpenTherm keepalive/polling, **hold the last commanded CH setpoint** (do not clear or invent a new remote demand), and refuse to apply **any** new remote Data ID writes until the link is healthy again.
- **FR-007**: The product MUST NOT implement Zigbee in any role (including dual-radio firmware images) for this feature.
- **FR-008**: The product MUST NOT implement OpenThread / Thread mesh / border-router behavior, and MUST NOT defer “Thread later” inside this feature’s delivery.
- **FR-009**: The gateway MUST NOT require a custom Home Assistant Core integration; MQTT Discovery (and standard MQTT entities) are the HA surface.
- **FR-010**: The gateway MUST expose the **full set of Data IDs supported by the connected boiler** (readable and writable as applicable). Matching a particular commercial gateway’s UI/layout is not required; omitting boiler-supported readable or writable IDs from HA exposure is not allowed.
- **FR-011**: OpenTherm bus traffic MUST remain first-class: Home Assistant messaging MUST NOT starve the ≥1 s keepalive/status cadence. Polling of the full supported readable set MAY use tiered/slower schedules for non-keepalive IDs.
- **FR-012**: The operator MUST be able to identify gateway online/offline (MQTT availability) distinctly from boiler link healthy/unhealthy via a dedicated boiler-link health entity (not only via sensor/climate unavailable). Boiler-link MUST report **unhealthy** after a documented consecutive OpenTherm exchange failure threshold (default: **3 consecutive failed** keepalive/status or in-scope exchanges at the ≥1 Hz cadence, or an equivalent short failure window), and MUST report **healthy** again after **one successful** keepalive/status exchange—not on a single isolated failure before the unhealthy threshold, and not solely by mirroring Data ID entity unavailable.
- **FR-013**: CH Control setpoint (Data ID 1) commands outside the effective min/max MUST be **rejected** (not clamped): the gateway MUST NOT write an out-of-range value to the boiler, MUST retain the last accepted setpoint on the reflected entity, and MUST provide **observable feedback** as both (1) unchanged last-accepted reflected value and (2) an **explicit rejection signal** (MQTT event/status topic or a dedicated diagnostic entity)—no silent ignore, silent clamp, or HA-only display of the rejected out-of-range value. Effective **max** MUST prefer the boiler-reported CH setpoint upper-limit Data ID when available (commonly **ID 57** / max CH water setpoint limit, or the boiler’s equivalent supported max-limit ID). Effective **min** MUST prefer a boiler-reported CH setpoint lower-limit Data ID when the boiler offers one; otherwise min MUST use SoftAP-configured / firmware-default min. When no boiler max-limit ID is available, both bounds MUST use SoftAP-configured values seeded from firmware defaults. Boiler limit IDs remain readable entities per FR-002; they are also inputs to rejection bounds—not display-only.
- **FR-014**: The SoftAP / captive-portal UI MUST allow the operator to set CH setpoint min/max used as the fallback when boiler limit Data IDs are unavailable (and MUST persist those settings across reboot).
- **FR-015**: The gateway MUST determine which Data IDs the boiler supports (discovery/classification) and use that result to drive MQTT Discovery entity creation for reads and writes.

### Key Entities

- **Gateway device**: Single embedded controller at the boiler; owns Wi‑Fi client, MQTT client, and OpenTherm master roles.
- **Boiler (OpenTherm slave)**: Heating appliance on the OpenTherm link; source of readable Data IDs; sink of writable Data ID commands.
- **Home Assistant (via MQTT)**: Automation hub that discovers and displays entities and issues Data ID write commands.
- **MQTT broker**: Operator-provided message broker on the LAN path between gateway and Home Assistant.
- **Supported Data ID**: An OpenTherm Data ID classified as available on the connected boiler for read and/or write; each such ID maps to HA entity exposure per FR-002.
- **Boiler-link health**: Dedicated HA entity reflecting OpenTherm master↔slave link healthy vs unhealthy, distinct from MQTT device availability. Unhealthy after documented consecutive OT exchange failures (default 3); healthy after one successful keepalive/status exchange.
- **Writable command**: A Home Assistant control change mapped to an OpenTherm master write of a boiler-supported writable Data ID (including ID 1 Control setpoint and Status/CH-enable when supported).
- **Setpoint bounds**: Effective CH water setpoint min/max for Data ID 1 — prefer boiler max-limit Data ID (commonly ID 57) and boiler min-limit ID when offered; otherwise SoftAP-persisted operator values seeded from firmware defaults.
- **Fail-safe state**: Gateway mode while Wi‑Fi or MQTT is down; continues OT keepalive, holds the last commanded CH setpoint, and refuses new remote Data ID writes until the link recovers.

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: Under normal Wi‑Fi and broker conditions, a supported readable Data ID value appears or updates in Home Assistant within **5 seconds** of a successful OpenTherm read of that value.
- **SC-002**: Under normal conditions, a write to a supported writable Data ID from Home Assistant results in an OpenTherm command attempt and a reflected entity update within **2 seconds**.
- **SC-003**: While Home Assistant traffic is present, OpenTherm master keepalive/status continues at **≥ 1 cycle per second** (no starvation).
- **SC-004**: Within **10 seconds** of Wi‑Fi or MQTT loss, the gateway is in fail-safe (per FR-006) with no undefined boiler demand.
- **SC-005**: An operator with a working MQTT-capable Home Assistant can go from powered unconfigured gateway → SoftAP/captive-portal Wi‑Fi+MQTT setup → visible entities for gateway meta state and boiler-supported Data IDs **without** installing a custom HA integration, on the first commissioning attempt in a documented setup path.
- **SC-006**: Out-of-scope transports are absent: no Zigbee join/control path and no Thread/OpenThread commissioning or mesh role in the delivered feature.
- **SC-007**: On a boiler (or simulator) with a known supported-ID set S, every ID in S that is readable has a corresponding HA read entity, and every ID in S that is writable has a corresponding HA write control (spot-check plus documented discovery coverage is acceptable evidence).

## Assumptions

- Target hardware is the project’s **WeAct ESP32-C6 Mini** (ESP32-C6FH4) board with a suitable OpenTherm level adapter on **GPIO2 (in) / GPIO3 (out)**; Wi‑Fi is available at the boiler install site.
- First-boot (and re-provisioning) networking uses a **SoftAP + captive-portal web UI** for both Wi‑Fi and MQTT settings; serial/console and compile-time-only credential paths are not the v1 operator UX. Re-provisioning after a successful commission is triggered by **physical button long-press** (or board-equivalent), which clears saved network credentials and re-enters SoftAP—not by auto SoftAP-on-failure alone or MQTT/LAN-only config.
- Entity scope is the **boiler-supported OpenTherm Data ID catalog** (all readable IDs visible; all writable IDs controllable), plus MQTT availability and boiler-link health—not a fixed CH-only subset. Certifying every boiler SKU’s quirks remains out of scope; exposure follows per-boiler support discovery.
- MQTT Discovery prefix and entity patterns follow Home Assistant’s standard expectations; operator uses a LAN broker (e.g. HA Mosquitto add-on) reachable over IPv4 Wi‑Fi.
- TLS for MQTT is optional for a trusted LAN in v1; username/password (or equivalent) are supported when the broker requires them.
- On reconnect, retained MQTT write messages are either ignored until a fresh post-recovery command or applied once under a documented rule—default: **apply at most one retained CH setpoint (ID 1) only after link-up debounce**, then follow live commands; retained storms for other writable IDs MUST NOT spiral unintended boiler state (refine in plan if needed).
- Project constitution describes OpenTherm↔Wi‑Fi MQTT / Home Assistant Discovery as the product surface (v2.0.0+); this feature does not ship Zigbee or Thread.
- Existing Zigbee-oriented knowledge/playbooks in the repo remain historical/reference only for this feature; they are not a delivery dependency (OpenTherm Data ID and discovery knowledge remains relevant).
- Single boiler, single gateway; multi-boiler and multi-zone orchestration are out of scope.
- Numeric freshness/command bounds in SC-001/SC-002 may be tightened later but are the acceptance floor for this release.
- Exact HA entity types per Data ID (sensor, number, switch, climate, etc.) are chosen in planning to match HA MQTT Discovery capabilities while satisfying FR-002/FR-004.
