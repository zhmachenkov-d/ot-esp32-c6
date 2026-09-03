# Bug Verification: HA discovery republished every second

- **Slug**: ha-discovery-republish-loop
- **Tested**: 2026-08-30
- **Assessment**: ./assessment.md
- **Fix**: ./fix.md
- **Result**: partial

## Summary

Host unit tests and static call-site checks confirm the 1 Hz path no longer publishes discovery config, and the one-shot discovery path still does. The assessment’s MQTT-broker reproduction on device was not run, so end-to-end retained-config behavior remains unconfirmed in this environment.

## Checks Performed

| Check | Command / Action | Result | Notes |
|-------|------------------|--------|-------|
| Reproduction (post-fix) | MQTT broker logging of retained `homeassistant/` topics after link-up | skipped | Requires device + broker; not available in this host workspace |
| Static call-site audit | Grep for `mqtt_discovery_publish_status_*` | pass | `main.c` / `mqtt_commands.c` → state-only; full projections only from `mqtt_discovery_publish_all` |
| New / updated tests | `./build/test_status_projections` | pass | Both regression pins PASS |
| Regression suite | `firmware/tests/host/./run.sh` | pass | 9/9 tests passed |
| Lint / type-check | — | skipped | No separate host lint step for this path |

## Output Excerpts

```
test_status_flag_states_no_discovery_config:PASS
test_status_projections_publishes_discovery_config:PASS
4 Tests 0 Failures 0 Ignored
OK

100% tests passed, 0 tests failed out of 9
```

Call sites (post-fix):

- `main.c` `state_publish_task` → `mqtt_discovery_publish_status_flag_states`
- `mqtt_commands.c` accepted ID-0 → `mqtt_discovery_publish_status_flag_states`
- `mqtt_discovery.c` one-shot discovery → `mqtt_discovery_publish_status_projections`

## Residual Risks

- On-device confirmation still needed: after MQTT connect / catalog ready, status-flag `homeassistant/*/config` should appear once (retained), then only `status_flag/` state traffic at 1 Hz.
- Climate mode discovery on the 1 Hz path was left unchanged (assessment open question); out of scope for this bug.

## Recommendation

Hold for a short device check against a logging MQTT broker, then close. Host evidence strongly supports the fix; do not reopen unless broker logs still show per-second `homeassistant/*/config` publishes.
