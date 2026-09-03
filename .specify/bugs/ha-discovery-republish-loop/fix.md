# Bug Fix: HA discovery republished every second

- **Slug**: ha-discovery-republish-loop
- **Fixed**: 2026-08-30
- **Assessment**: ./assessment.md
- **Status**: applied

## Summary

Split status-flag publishing into a state-only API used by the 1 Hz loop and write-accept path, and kept full discovery (state + retained `homeassistant/*/config`) on the one-shot `mqtt_discovery_publish_all` path.

## Changes

| File | Change | Notes |
|------|--------|-------|
| `firmware/main/mqtt_discovery.h` | modified | Added `mqtt_discovery_publish_status_flag_states`; documented projections as discovery-only |
| `firmware/main/mqtt_discovery.c` | modified | Extracted state-only publisher; projections calls it then publishes config |
| `firmware/main/main.c` | modified | `state_publish_task` uses state-only |
| `firmware/main/mqtt_commands.c` | modified | Accepted ID-0 write uses state-only (no config republish) |
| `firmware/tests/host/stubs/mqtt_ha_stub.c` | modified | Records published topics for host asserts |
| `firmware/tests/host/test_status_projections.c` | modified | Pins state-only vs full-projections publish sets |

## Diff Highlights (optional)

```c
/* 1 Hz loop — state only */
mqtt_discovery_publish_status_flag_states(s_cfg.device_id,
                                          ot_poll_get_master_status_flags(),
                                          ot_poll_get_slave_status_flags());

/* One-shot discovery still calls full projections (state + config) */
mqtt_discovery_publish_status_projections(...);
```

## Tests Added or Updated

- `firmware/tests/host/test_status_projections.c::test_status_flag_states_no_discovery_config` — state-only publishes five `status_flag/` topics and zero `homeassistant/` config topics
- `firmware/tests/host/test_status_projections.c::test_status_projections_publishes_discovery_config` — full path publishes five state + five discovery configs (4 binary_sensor + 1 switch)

## Local Verification

- Commands run: `firmware/tests/host/./run.sh` → 9/9 passed
- Manual checks: none (host coverage matches the assessment’s unit-test contract)

## Deviations from Assessment

- Also switched `mqtt_commands.c` accepted-ID-0 path to state-only; assessment listed it as optional. Config does not change on write accept, so republishing discovery there was the same wasteful pattern.
- Climate mode on the 1 Hz path left unchanged (open question in assessment; not part of preferred remediation).

## Follow-ups

- Run `/speckit-bug-test slug=ha-discovery-republish-loop`.
- Confirm after MQTT connect / catalog ready that status-flag discovery still appears once on the broker (retained config from `mqtt_discovery_publish_all`).
- Optionally decide whether climate discovery (vs mode state) needs the same treatment if Bugbot flags it later.
