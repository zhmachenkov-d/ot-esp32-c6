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

- Raised `failsafe` task stack 3072 → 8192 bytes in `firmware/main/main.c` (same ballpark as `ota` 8192 / `ota_poll` 6144). Comment notes nested rediscovery buffers + the reconnect overflow.
- Left discovery on `failsafe` / `mqtt_session_tick` unchanged (Intent: stack only).
- Host `./run.sh`: 13/13 pass. `idf.py build` OK (OTA size gate OK).
- Blind-hunter: clarified comment (~2 KiB+ nested locals, bytes); HIL results citation lives on `docs/hil-ota-case4-retry` (separate commit). Deferred: on-device rediscovery retest; optional main-task stack margin.

## Spec Change Log

## Review Triage Log

- blind-hunter: HIL `v10_ota.md` on this branch lacks stack-fault note cited by Intent/comment — `false` for blocking this PR — evidence is committed on `docs/hil-ota-case4-retry`; dropped stale `context` pointer; comment no longer requires that file.
- blind-hunter: “~1 KiB JSON frames” understates nested discovery buffers — `low` — **patch**: comment now says ~2 KiB+ nested locals + snprintf.
- blind-hunter: stack units / permanent RAM cost undocumented vs heap budget — `low` — rejected for extra docs; comment now says “8192 bytes”; +5 KiB is within free-heap budget log.
- blind-hunter: boot `mqtt_session_tick` still on `app_main` (`CONFIG_ESP_MAIN_TASK_STACK_SIZE=3584`) — `defer` — boot path reaches operational in field; overflow math (~3400 peak) fits 3584 but is tight; not introduced by this change.
- blind-hunter: no HIL rediscovery retest / high-water mark in Notes — `defer` — host+build cannot catch on-device stack fault; needs device retest after flash.
- blind-hunter: empty Spec Change Log / status still in-progress mid-oneshot — `false` — finalize sets `done` and fills triage.
