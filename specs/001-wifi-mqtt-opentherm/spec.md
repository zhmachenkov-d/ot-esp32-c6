# Feature Specification: OpenTherm Wi‑Fi MQTT Gateway

**Feature Branch**: `features/001-wifi-mqtt-opentherm`

**Created**: 2026-08-29

**Status**: Draft

**Input**: User description: "thin OT master on ESP32-C6, Wi‑Fi MQTT + Discovery, small entity set, OT cadence + fail-safe; Zigbee and Thread out of scope."

**Assessment handoff**: `.specify/assessments/opentherm-gateway/decision.md` (go — Option C)

## User Scenarios & Testing *(mandatory)*

### User Story 1 - See boiler status in Home Assistant (Priority: P1)

As the home operator, I commission the gateway onto my Wi‑Fi network and MQTT broker so that boiler status and key temperatures appear automatically in Home Assistant without writing a custom integration.

**Why this priority**: Visibility is the minimum viable value—without live readings, control and fail-safe are meaningless.

**Independent Test**: With a boiler (or boiler simulator) and a Home Assistant instance using MQTT discovery, power the gateway, complete network/broker setup, and confirm status plus in-scope sensors appear and update.

**Acceptance Scenarios**:

1. **Given** the gateway is powered and joined to Wi‑Fi with a reachable MQTT broker, **When** Home Assistant is configured for MQTT discovery, **Then** the gateway’s in-scope entities appear without manual YAML entity definitions.
2. **Given** the OpenTherm link to the boiler is healthy, **When** the boiler reports status and boiler water temperature, **Then** those values become visible on the corresponding Home Assistant entities within the freshness bound in Success Criteria.
3. **Given** entities already exist from a prior session, **When** the gateway reconnects after a power cycle, **Then** the same logical entities are rediscovered or resume updating without the operator recreating them by hand.

---

### User Story 2 - Change heating setpoint from Home Assistant (Priority: P1)

As the home operator, I adjust the central-heating water setpoint (or equivalent climate control) from Home Assistant and the boiler receives that demand over OpenTherm.

**Why this priority**: Two-way control is the other half of the stated product job; monitoring alone is incomplete.

**Independent Test**: From Home Assistant, change the in-scope climate/setpoint control and verify the gateway issues the corresponding OpenTherm master command and reflected status matches within the command bound.

**Acceptance Scenarios**:

1. **Given** Home Assistant shows the gateway climate/setpoint entity as available, **When** the operator sets a new allowed CH water setpoint, **Then** the gateway commands the boiler accordingly and the entity reflects the accepted value within the command round-trip bound.
2. **Given** the operator sets a setpoint outside the allowed range, **When** the command is published, **Then** the gateway rejects or clamps per documented limits and does not leave Home Assistant and the boiler disagreeing silently.
3. **Given** OpenTherm communication is temporarily failing, **When** the operator issues a setpoint change, **Then** Home Assistant can observe that the command did not take effect (unavailable, error, or unchanged reflected status)—not a false success.

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

- Boiler does not acknowledge or support an in-scope Data ID: entity shows unavailable/unknown; other IDs continue.
- MQTT broker accepts connection but Home Assistant discovery is disabled: gateway still publishes state on its topics; operator must enable discovery (documented expectation).
- Rapid setpoint changes from HA: gateway serializes OpenTherm traffic so keepalive/status polling is not starved.
- Wi‑Fi connected but broker unreachable (DNS/timeout): treated as MQTT unavailable → fail-safe.
- Partial OpenTherm adapter/wiring fault: no false “success” on commands; status reflects link unhealthy.
- Broker sends retained or birth/LWT storms on reconnect: gateway recovers without locking the boiler into an unintended setpoint spiral (see Assumptions).

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**: The gateway MUST act as OpenTherm **master** toward a single connected boiler (or compatible slave) and maintain master keepalive / status communication at least once per second under normal and degraded HA-link conditions.
- **FR-002**: The gateway MUST expose a small, fixed initial set of Home Assistant entities via MQTT Discovery: device availability/status, boiler water temperature (sensor), and a central-heating climate/setpoint control suitable for CH water temperature demand. Domestic hot water (DHW) entities are out of scope for this release unless explicitly added later.
- **FR-003**: The gateway MUST publish updates for in-scope read values so Home Assistant shows changes within the freshness bounds in Success Criteria under normal Wi‑Fi and broker conditions.
- **FR-004**: The gateway MUST accept in-scope setpoint/climate commands from Home Assistant over MQTT and execute the corresponding OpenTherm write, then reflect success or failure in entity state.
- **FR-005**: The gateway MUST obtain Wi‑Fi connectivity and connect to an operator-configured MQTT broker on the local network; credentials MUST be configurable by the operator (not hard-coded in source).
- **FR-006**: When Wi‑Fi or MQTT is unavailable, the gateway MUST enter a documented fail-safe mode: continue OpenTherm keepalive/polling, **hold the last commanded CH setpoint** (do not clear or invent a new remote demand), and refuse to apply new remote commands until the link is healthy again.
- **FR-007**: The product MUST NOT implement Zigbee in any role (including dual-radio firmware images) for this feature.
- **FR-008**: The product MUST NOT implement OpenThread / Thread mesh / border-router behavior, and MUST NOT defer “Thread later” inside this feature’s delivery.
- **FR-009**: The gateway MUST NOT require a custom Home Assistant Core integration; MQTT Discovery (and standard MQTT entities) are the HA surface.
- **FR-010**: The gateway MUST NOT aim for feature parity with large commercial OpenTherm gateway entity catalogs; only the in-scope small entity set is required.
- **FR-011**: OpenTherm bus traffic MUST remain first-class: Home Assistant messaging MUST NOT starve the ≥1 s keepalive/status cadence.
- **FR-012**: The operator MUST be able to identify gateway online/offline (availability) distinctly from boiler link healthy/unhealthy where the platform allows.
- **FR-013**: Setpoint commands outside configured min/max MUST be rejected or clamped with observable feedback (no silent ignore).

### Key Entities

- **Gateway device**: Single embedded controller at the boiler; owns Wi‑Fi client, MQTT client, and OpenTherm master roles.
- **Boiler (OpenTherm slave)**: Heating appliance on the OpenTherm link; source of status and temperatures; sink of CH setpoint commands.
- **Home Assistant (via MQTT)**: Automation hub that discovers and displays entities and issues climate/setpoint commands.
- **MQTT broker**: Operator-provided message broker on the LAN path between gateway and Home Assistant.
- **In-scope reading**: A boiler value mapped to a Home Assistant sensor or climate attribute (initially status-related flags as needed for climate, boiler water temperature).
- **In-scope command**: A Home Assistant climate/setpoint change mapped to an OpenTherm master write (initially CH water setpoint).
- **Fail-safe state**: Gateway mode while Wi‑Fi or MQTT is down; continues OT keepalive and holds the last commanded CH setpoint until the link recovers.

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: Under normal Wi‑Fi and broker conditions, in-scope sensor values appear or update in Home Assistant within **5 seconds** of a successful OpenTherm read of that value.
- **SC-002**: Under normal conditions, an in-scope setpoint change from Home Assistant results in an OpenTherm command attempt and a reflected entity update within **2 seconds**.
- **SC-003**: While Home Assistant traffic is present, OpenTherm master keepalive/status continues at **≥ 1 cycle per second** (no starvation).
- **SC-004**: Within **10 seconds** of Wi‑Fi or MQTT loss, the gateway is in fail-safe (per FR-006) with no undefined boiler demand.
- **SC-005**: An operator with a working MQTT-capable Home Assistant can go from powered gateway + configured Wi‑Fi/broker to visible in-scope entities **without** installing a custom HA integration, on the first commissioning attempt in a documented setup path.
- **SC-006**: Out-of-scope transports are absent: no Zigbee join/control path and no Thread/OpenThread commissioning or mesh role in the delivered feature.

## Assumptions

- Target hardware is the project’s **WeAct ESP32-C6-A** (ESP32-C6-class) board with a suitable OpenTherm level adapter; Wi‑Fi is available at the boiler install site.
- Initial entity set is **CH-focused and small** (availability, boiler water temperature, CH climate/setpoint)—not full Data ID discovery catalogs or OTGW-scale entity counts.
- MQTT Discovery prefix and climate/sensor patterns follow Home Assistant’s standard expectations; operator uses a LAN broker (e.g. HA Mosquitto add-on) reachable over IPv4 Wi‑Fi.
- TLS for MQTT is optional for a trusted LAN in v1; username/password (or equivalent) are supported when the broker requires them.
- On reconnect, retained MQTT setpoint messages are either ignored until a fresh post-recovery command or applied once under a documented rule—default: **apply at most one retained setpoint only after link-up debounce**, then follow live commands (refine in plan if needed).
- Project constitution language that still frames the product as OpenTherm↔Zigbee will be amended as a governance follow-up so it matches this Wi‑Fi MQTT direction; this feature does not ship Zigbee.
- Existing Zigbee-oriented knowledge/playbooks in the repo remain historical/reference only for this feature; they are not a delivery dependency.
- Single boiler, single gateway; multi-boiler and multi-zone orchestration are out of scope.
- Numeric freshness/command bounds in SC-001/SC-002 may be tightened later but are the acceptance floor for this release.
