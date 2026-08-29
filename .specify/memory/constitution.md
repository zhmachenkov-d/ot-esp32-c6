<!--
Sync Impact Report
- Version change: (none / template placeholders) → 1.0.0
- Modified principles:
  - [PRINCIPLE_1_NAME] → I. Code Quality
  - [PRINCIPLE_2_NAME] → II. Testing Standards
  - [PRINCIPLE_3_NAME] → III. User Experience Consistency
  - [PRINCIPLE_4_NAME] → IV. Performance Requirements
  - [PRINCIPLE_5_NAME] → V. Simplicity & Explicit Trade-offs
- Added sections:
  - Embedded & Integration Constraints
  - Development Workflow & Quality Gates
  - Governance (concrete rules)
- Removed sections: none (scaffold placeholders replaced)
- Follow-up TODOs: none
-->

# ot-esp32-c6 Constitution

## Core Principles

### I. Code Quality

All production code MUST be clear, modular, and maintainable under embedded
resource limits.

- Names, types, and module boundaries MUST reflect domain concepts (OpenTherm
  Data IDs, Zigbee endpoints/clusters, poll tiers, spillover routing) rather
  than opaque abbreviations or leaky hardware details at call sites.
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

**Rationale**: Firmware that bridges OpenTherm and Zigbee is long-lived and
hard to debug in the field. Readable, domain-aligned code reduces regressions
and keeps agent and human changes surgical.

### II. Testing Standards

Behavior that matters MUST be covered by automated tests before it is treated
as done.

- New behavior and bug fixes MUST include tests that would fail without the
  change (unit, integration, or host-side protocol/mapping tests as appropriate).
- Protocol mappings, setpoint/temperature conversions, catalog/discovery
  classification, and ZCL attribute encoding MUST have deterministic tests with
  explicit fixtures for valid, invalid, and boundary values.
- Integration or hardware-in-the-loop checks MUST cover cross-layer contracts
  when changing OpenTherm poll scheduling, Zigbee join/control paths, NVS
  persistence, or multi-endpoint routing.
- Flaky tests MUST NOT be merged; tests MUST be deterministic given the same
  inputs and simulated time/hardware stubs.
- "Works on my board" alone is insufficient acceptance; CI or documented
  host-test suites MUST pass for the change set.

**Rationale**: Timing, encoding, and multi-channel routing bugs are expensive
to catch on-device. Tests encode the contract so refactors stay safe.

### III. User Experience Consistency

Operator- and home-automation-facing behavior MUST be predictable and uniform
across channels and releases.

- Thermostat channels that share a capability MUST expose the same ZCL
  semantics, units, and invalid/unavailable sentinels unless the spec
  documents a deliberate exception.
- User-visible state (setpoints, local temperature, availability, catalog
  status) MUST update through documented routes; silent drops, partial
  updates, or channel-specific quirks without documentation are forbidden.
- Error and unavailable conditions MUST surface consistently (same sentinel
  values, same cluster/attribute patterns) so controllers and dashboards
  behave the same across endpoints.
- Breaking changes to Zigbee attribute meaning, endpoint layout, or command
  behavior MUST be versioned and called out in release notes; silent semantic
  drift is forbidden.
- Diagnostics and logs that operators or developers rely on MUST use stable
  tags and actionable messages, not one-off formats per module.

**Rationale**: The "user" of this firmware is often a Zigbee controller or
HVAC integrator. Inconsistent attributes or sentinels look like product bugs
even when the firmware "works."

### IV. Performance Requirements

Real-time and resource budgets are part of the product contract, not optional
polish.

- OpenTherm poll work MUST respect documented tier budgets and per-tick
  multi-read limits; a change that starves fast-tier reads or blocks the
  Zigbee stack MUST NOT merge without a measured justification and spec update.
- Hot paths (poll engine, ZCL callbacks, mapping) MUST avoid unbounded loops,
  blocking I/O on critical tasks, and heap churn unless explicitly approved.
- Flash, RAM, and stack use MUST stay within project budgets; regressions that
  threaten headroom MUST be detected in review or CI size checks where available.
- Latency-sensitive user actions (setpoint write → boiler command path) MUST
  meet the latency expectations stated in the feature spec; if unspecified,
  the implementer MUST propose measurable targets before coding.
- Performance-affecting changes SHOULD include before/after evidence (timing
  logs, poll occupancy, or size delta) in the PR.

**Rationale**: Missed poll deadlines and stack stalls cause wrong temperatures,
missed setpoints, and unstable mesh behavior—user-visible failures.

### V. Simplicity & Explicit Trade-offs

Prefer the simplest design that meets the spec; record justified complexity.

- YAGNI applies: do not build alternate backends, feature flags, or layers for
  hypothetical future products.
- When a more complex design is required (e.g., tiered polling, spillover
  cluster), the rationale MUST appear in the spec, ADR, or PR description.
- Dependencies and SDK surface area MUST be minimized; new libraries require
  a clear ownership and test story.

**Rationale**: Embedded systems punish accidental complexity with size, timing,
and debug cost.

## Embedded & Integration Constraints

- Target platform guidance (ESP32-C6, ESP-IDF, OpenTherm adapter wiring, Zigbee
  endpoint model) in `knowledge/` is normative for domain facts; code MUST NOT
  contradict documented Data ID, GPIO, or cluster contracts without updating
  that knowledge in the same change.
- Secrets and local credentials MUST NOT be committed; use `.env` / example
  templates only.
- Manufacturer-specific and spillover encodings MUST remain interoperable with
  the documented attribute layout; ad-hoc packing is forbidden.

## Development Workflow & Quality Gates

- Specs, plans, and tasks under Spec-Driven Development MUST align with this
  constitution; conflicts are resolved by amending the constitution or changing
  the feature artifacts—never by silent deviation.
- PRs MUST demonstrate: constitution compliance, relevant tests, and no
  unexplained complexity or UX inconsistency across channels.
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

**Version**: 1.0.0 | **Ratified**: 2026-08-29 | **Last Amended**: 2026-08-29
