---
title: 'Failsafe task stack for MQTT rediscovery'
type: 'bugfix'
created: '2026-10-02'
status: 'done'
route: 'oneshot'
review_loop_iteration: 0
context: []
---

<frozen-after-approval reason="human-owned intent — do not modify unless human renegotiates">

## Intent

**Problem:** After a long fail-safe (MQTT down), reconnect arms MQTT session debounce; `mqtt_session_tick` then runs HA discovery on the `failsafe` task, whose 3072-word stack overflows in `snprintf` (HIL OTA case 4 retry: Stack protection fault → panic reboot).

**Approach:** Raise the `failsafe` FreeRTOS task stack so rediscovery after link recovery fits, without moving discovery onto another task in this change.

</frozen-after-approval>

## Implementation Notes

- Raised `failsafe` task stack then, on human patch request: moved HA rediscovery to dedicated `mqtt_sess` (8192) and dropped `mqtt_session_tick` from `failsafe_task`; `failsafe` now 4096 (margin above old 3072 panic floor).
- `CONFIG_ESP_MAIN_TASK_STACK_SIZE=8192` in `firmware/sdkconfig.defaults` (boot wait no longer runs discovery on main after session task starts early).
- Host `./run.sh` + `idf.py build` re-verified after patches.
- Deferred HIL rediscovery retest still applies on-device.

## Spec Change Log

## Review Triage Log

- blind-hunter: HIL `v10_ota.md` on this branch lacks stack-fault note cited by Intent/comment — `false` for blocking this PR — evidence is committed on `docs/hil-ota-case4-retry`; dropped stale `context` pointer; comment no longer requires that file.
- blind-hunter: “~1 KiB JSON frames” understates nested discovery buffers — `low` — **patch**: comment now says ~2 KiB+ nested locals + snprintf.
- blind-hunter: stack units / permanent RAM cost undocumented vs heap budget — `low` — rejected for extra docs; comment now says “8192 bytes”; +5 KiB is within free-heap budget log.
- blind-hunter: boot `mqtt_session_tick` still on `app_main` (`CONFIG_ESP_MAIN_TASK_STACK_SIZE=3584`) — `defer` — boot path reaches operational in field; overflow math (~3400 peak) fits 3584 but is tight; not introduced by this change.
- blind-hunter: no HIL rediscovery retest / high-water mark in Notes — `defer` — host+build cannot catch on-device stack fault; needs device retest after flash.
- blind-hunter: empty Spec Change Log / status still in-progress mid-oneshot — `false` — finalize sets `done` and fills triage.
