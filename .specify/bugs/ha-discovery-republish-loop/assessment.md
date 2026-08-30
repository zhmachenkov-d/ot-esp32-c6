# Bug Assessment: HA discovery republished every second

- **Slug**: ha-discovery-republish-loop
- **Created**: 2026-08-30
- **Source**: pasted text (Bugbot review, branch changes)
- **Verdict**: valid
- **Severity**: medium

## Report (verbatim or summarized)

Bugbot: `state_publish_task` calls `mqtt_discovery_publish_status_projections` every second; that function republishes retained `homeassistant/*/config` payloads, not just live flag state topics. Cited `firmware/main/main.c:173-177`.

## Symptom

About once per second, status-flag entities re-publish full Home Assistant MQTT discovery config JSON (via `publish_config`) in addition to `ON`/`OFF` state. Expected: periodic state updates only; discovery config once at session arm / catalog change.

## Reproduction

1. Connect MQTT broker with logging of retained/`homeassistant/` topics.
2. After link-up, observe `state_publish_task` loop calling `mqtt_discovery_publish_status_projections` each second (`main.c:173-177`).
3. Inspect `mqtt_discovery_publish_status_projections` (`mqtt_discovery.c:162-207`): each iteration publishes state **and** builds/publishes discovery config for binary sensors / switch.

## Suspected Code Paths

- `firmware/main/main.c:173-177` — periodic call from `state_publish_task`
- `firmware/main/mqtt_discovery.c:162-207` — mixes state publish with `publish_config`
- `firmware/main/mqtt_commands.c:115` — another caller of the same combined function

## Root Cause Hypothesis

One helper both updates live state and (re)announces discovery. Using it on a 1 Hz state loop floods config topics. Confidence: **high**.

## Proposed Remediation

**Preferred**: Split into `mqtt_discovery_publish_status_flag_states(...)` (state only) and keep config publish for session-arm / one-shot discovery. Call state-only from `state_publish_task`; call full discovery from the existing discovery path.

**Alternatives**:
- Gate config publish behind a “dirty” / first-publish flag inside the combined function — smaller diff, easier to regress.
- Throttle config republish (e.g. every N minutes) — still wasteful vs one-shot.

**Files likely to change**:
- `firmware/main/mqtt_discovery.c` / `.h`
- `firmware/main/main.c`
- `firmware/main/mqtt_commands.c` (if it should stay config+state or split)

**Tests to add or update**:
- Unit test: state-only path does not invoke `publish_config`; discovery path does once per call.

## Risks & Considerations

- HA may rely on retained config; ensure one-shot still uses retain and runs after MQTT connect / catalog ready.
- CH-enable switch discovery depends on `writable_ch_enable`; republish after catalog updates must still happen when writability changes.

## Open Questions

- [NEEDS CLARIFICATION: Should climate mode discovery stay on the 1 Hz path as well, or is only status_projections the concern?]
