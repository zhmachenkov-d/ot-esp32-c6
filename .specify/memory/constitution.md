<!--
Sync Impact Report
- Version change: 1.0.0 → 2.0.0
- Modified principles:
  - I. Code Quality — domain names: Zigbee endpoints/clusters → MQTT Discovery / HA entity roles
  - II. Testing Standards — ZCL encoding / Zigbee join paths → MQTT discovery payloads / SoftAP / fail-safe
  - III. User Experience Consistency — ZCL/Zigbee controller framing → MQTT Discovery / Home Assistant entities
  - IV. Performance Requirements — Zigbee stack / ZCL callbacks → MQTT client / HA traffic vs OT poll
  - V. Simplicity — unchanged intent; example wording no longer implies spillover-cluster product path
- Added sections: none
- Removed / redefined:
  - Embedded constraints: Zigbee endpoint / spillover / cluster packing → OT GPIO + MQTT topic contracts;
    historical Zigbee knowledge remains reference-only
- Follow-up TODOs: none (closes governance mismatch for Wi‑Fi MQTT feature direction)
-->

# ot-esp32-c6 Constitution

## Core Principles

### I. Code Quality

All production code MUST be clear, modular, and maintainable under embedded
resource limits.

- Names, types, and module boundaries MUST reflect domain concepts (OpenTherm
  Data IDs, MQTT Discovery / Home Assistant entity roles, poll tiers, fail-safe
  and link-health states) rather than opaque abbreviations or leaky hardware
  details at call sites.
- Each translation unit MUST have a single primary reason to change; shared
  protocol or mapping logic MUST live in one place, not duplicated across
  channels or layers.
- Public APIs MUST document ownership, lifetime, units, and error returns;
  undocumented side effects are forbidden.
- Speculative abstraction (`void*` contexts, unused `#ifdef` matrices, unused
  configurability) MUST NOT ship. Prefer the smallest design that satisfies
  the current spec.
- Code review MUST treat unexplained complexity, mysterious names, duplicated
  logic, and shotgun edits as defects to fix or justify in the PR.

**Rationale**: Firmware that bridges OpenTherm to Home Assistant over Wi‑Fi MQTT
is long-lived and hard to debug in the field. Readable, domain-aligned code
reduces regressions and keeps agent and human changes surgical.

### II. Testing Standards

Behavior that matters MUST be covered by automated tests before it is treated
as done.

- New behavior and bug fixes MUST include tests that would fail without the
  change (unit, integration, or host-side protocol/mapping tests as appropriate).
- Protocol mappings, setpoint/temperature conversions, catalog/discovery
  classification, MQTT Discovery payload builders, and command reject / fail-safe
  outcomes MUST have deterministic tests with explicit fixtures for valid,
  invalid, and boundary values.
- Integration or hardware-in-the-loop checks MUST cover cross-layer contracts
  when changing OpenTherm poll scheduling, SoftAP commissioning, MQTT
  connect/discovery/command paths, NVS persistence, or fail-safe transitions.
- Flaky tests MUST NOT be merged; tests MUST be deterministic given the same
  inputs and simulated time/hardware stubs.
- "Works on my board" alone is insufficient acceptance; CI or documented
  host-test suites MUST pass for the change set.

**Rationale**: Timing, encoding, and multi-channel routing bugs are expensive
to catch on-device. Tests encode the contract so refactors stay safe.

### III. User Experience Consistency

Operator- and home-automation-facing behavior MUST be predictable and uniform
across releases.

- Capabilities that share meaning MUST expose the same MQTT Discovery semantics,
  units, and invalid/unavailable sentinels unless the spec documents a
  deliberate exception.
- User-visible state (setpoints, temperatures, MQTT availability, boiler-link
  health, catalog coverage) MUST update through documented routes; silent drops,
  partial updates, or topic-specific quirks without documentation are forbidden.
- Error and unavailable conditions MUST surface consistently (same sentinels,
  same availability vs boiler-link separation) so Home Assistant dashboards
  behave predictably.
- Breaking changes to MQTT topic meaning, entity unique_id layout, or command
  behavior MUST be versioned and called out in release notes; silent semantic
  drift is forbidden.
- Diagnostics and logs that operators or developers rely on MUST use stable
  tags and actionable messages, not one-off formats per module.

**Rationale**: The primary "user" of this firmware is often Home Assistant via
MQTT Discovery (plus the SoftAP operator). Inconsistent entities or sentinels
look like product bugs even when the firmware "works."

### IV. Performance Requirements

Real-time and resource budgets are part of the product contract, not optional
polish.

- OpenTherm poll work MUST respect documented tier budgets and per-tick
  multi-read limits; a change that starves fast-tier reads or lets HA/MQTT work
  block the OT keepalive task MUST NOT merge without a measured justification
  and spec update.
- Hot paths (poll engine, MQTT callbacks that enqueue OT work, mapping) MUST
  avoid unbounded loops, blocking I/O on critical tasks, and heap churn unless
  explicitly approved.
- Flash, RAM, and stack use MUST stay within project budgets; regressions that
  threaten headroom MUST be detected in review or CI size checks where available.
- Latency-sensitive user actions (setpoint write → boiler command path) MUST
  meet the latency expectations stated in the feature spec; if unspecified,
  the implementer MUST propose measurable targets before coding.
- Performance-affecting changes SHOULD include before/after evidence (timing
  logs, poll occupancy, or size delta) in the PR.

**Rationale**: Missed poll deadlines and stack stalls cause wrong temperatures,
missed setpoints, and unstable heating control—user-visible failures.

### V. Simplicity & Explicit Trade-offs

Prefer the simplest design that meets the spec; record justified complexity.

- YAGNI applies: do not build alternate backends, feature flags, or layers for
  hypothetical future products (including Zigbee or Thread dual-radio paths
  unless a future spec explicitly requires them).
- When a more complex design is required (e.g., tiered polling), the rationale
  MUST appear in the spec, ADR, or PR description.
- Dependencies and SDK surface area MUST be minimized; new libraries require
  a clear ownership and test story.

**Rationale**: Embedded systems punish accidental complexity with size, timing,
and debug cost.

## Embedded & Integration Constraints

- Target platform guidance (ESP32-C6, ESP-IDF, OpenTherm adapter wiring, MQTT
  topic / Discovery contracts) in `knowledge/` and feature `contracts/` is
  normative for domain facts; code MUST NOT contradict documented Data ID, GPIO,
  or MQTT contracts without updating that documentation in the same change.
  Zigbee-oriented playbooks in `knowledge/` are historical/reference only unless
  a feature spec reinstates them.
- Secrets and local credentials MUST NOT be committed; use `.env` / example
  templates only.
- MQTT Discovery payloads and OpenTherm encodings MUST remain interoperable with
  the documented contracts; ad-hoc packing or invented Data ID values are
  forbidden.

## Development Workflow & Quality Gates

- Specs, plans, and tasks under Spec-Driven Development MUST align with this
  constitution; conflicts are resolved by amending the constitution or changing
  the feature artifacts—never by silent deviation.
- PRs MUST demonstrate: constitution compliance, relevant tests, and no
  unexplained complexity or UX inconsistency across operator/HA surfaces.
- Agents and humans MUST prefer surgical diffs: touch only what the task
  requires; do not "clean up" unrelated code in the same change.

## Governance

This constitution supersedes informal practice and conflicting local habits.
Amendments require:

1. An explicit edit to this file with semantic version bump:
   - MAJOR: remove or redefine a principle in a backward-incompatible way
   - MINOR: add a principle or materially expand guidance
   - PATCH: clarifications and non-semantic wording fixes
2. Updated **Last Amended** date (ISO `YYYY-MM-DD`)
3. A Sync Impact Report comment at the top of this file summarizing the change

Compliance:

- All reviews (human or automated) MUST verify applicable principles.
- Exceptions require written justification in the PR and, if lasting, an
  amendment or ADR.
- Runtime development guidance in `.cursor/skills/` and project docs MUST NOT
  contradict this constitution; if they do, update those guides or amend here.

**Version**: 2.0.0 | **Ratified**: 2026-08-29 | **Last Amended**: 2026-08-30
