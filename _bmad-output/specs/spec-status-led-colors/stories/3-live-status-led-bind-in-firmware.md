---
title: 'Live status LED bind in firmware'
type: 'feature'
created: '2026-09-12'
status: 'done'
route: 'dispatch'
review_loop_iteration: 0
baseline_commit: 'cf4c6c38521716ee3afa104bec0055a16789a603'
context:
  - '{project-root}/_bmad-output/specs/spec-status-led-colors/SPEC.md'
  - '{project-root}/_bmad-output/specs/spec-status-led-colors/bring-up-ladder.md'
  - '{project-root}/_bmad-output/specs/spec-status-led-colors/stories/2-host-testable-bring-up-ladder-evaluator.md'
---

<frozen-after-approval reason="human-owned intent — do not modify unless human renegotiates">

## Intent

**Problem:** The WS2812 and ladder evaluator exist, but the LED still shows interim dim white — operators cannot see live SoftAP / critical / bring-up progress (CAP-1/CAP-2 end-to-end).

**Approach:** Each evaluation tick, sample existing SoftAP / fail-safe / `PROVISION_BOOT_RUN_NO_MQTT` / OTA failed and ladder bind flags, run `status_led_ladder_eval`, and apply RGB+pattern via `status_led_set_*`. No separate LED-only dashboard task.

## Boundaries & Constraints

**Always:**
- Sample → `status_led_ladder_eval` → `status_led_set_rgb` + `status_led_set_pattern` every tick; live recompute (no high-water).
- SoftAP source: `provision_is_active()`; critical: `failsafe_is_active`, sticky boot `PROVISION_BOOT_RUN_NO_MQTT`, OTA live failed via new thin `ota_update_failed()` (or equivalent getter over existing `s_failed`).
- Ladder binds: STA+IP (`s_wifi_up && s_got_ip`), `mqtt_ha_connected()`, `s_mqtt_session_ready`, `ot_poll_boiler_link_healthy()`.
- Call the same bind helper from the operational path (`failsafe_task`) **and** the SoftAP path (`ota_confirm_tick_task` or equivalent that still runs after SoftAP early return).
- Sticky `PROVISION_BOOT_RUN_NO_MQTT` for process life when boot selects that action (re-provision → restart clears).
- Replace interim white default once bind runs; driver dim/timing unchanged.

**Never:**
- A dedicated “live health dashboard” FreeRTOS task whose only job is LED policy.
- Change ladder RGB/pattern semantics, story-1 driver GPIO/timer, OT GPIOs 2/3, SoftAP GPIO 9, or fail-safe/MQTT/OTA **policy**.
- Treat sticky post-rollback / pending-verify OTA as critical (v1).
- Invent a parallel health model for the LED.

## I/O & Edge-Case Matrix

| Scenario | Input / State | Expected Output / Behavior | Error Handling |
|----------|--------------|---------------------------|----------------|
| SoftAP live | SoftAP active (commissioning) | Magenta slow blink within one tick | SoftAP beats critical |
| Critical live | SoftAP off; fail-safe or boot-no-MQTT or OTA failed | Red fast blink | OR of three |
| Ladder live | SoftAP/critical off; flags at each rung | Matching color/pattern from ladder | Uses story-2 eval |
| Regress | Higher rung then flag drops | LED drops to new rung (no sticky) | Live sample each tick |
| SoftAP boot path | Credentials missing; SoftAP early return | Bind still runs (SoftAP path tick) | Not only `failsafe_task` |
| Interim replaced | After first successful bind tick | No longer stuck on dim white | Init may still flash white briefly |

</frozen-after-approval>

## Code Map

- `firmware/main/main.c` -- primary: sticky boot-no-MQTT; `status_led_bind_tick()` (or inline helper) fill `status_led_ladder_input_t` → eval → `status_led_set_*`; call from `failsafe_task` (~112–152) and SoftAP-path tick (~188–196 / early return).
- `firmware/main/ota_update.h` + `ota_update.c` -- add thin `ota_update_failed()` reading live `s_failed` (mirror `ota_update_in_progress`).
- `firmware/main/status_led_ladder.h` -- reuse eval/input/result; do not change pure logic.
- `firmware/main/status_led.h` -- reuse `set_rgb` / `set_pattern`; do not change driver refresh.
- `firmware/main/provision_softap.h` -- `provision_is_active()` for SoftAP; `PROVISION_BOOT_RUN_NO_MQTT` at boot (`provision_boot_action` ~`main.c:248–255`).
- Public already: `failsafe_is_active`, `mqtt_ha_connected`, `ot_poll_boiler_link_healthy`.
- Host: update OTA stub if host tests compile against new getter; ladder unit tests stay authoritative for mapping.
- Do **not** change: ladder palette/order, LED GPIO/timer dim, OT/SoftAP pin policy, OTA clear/set policy for `s_failed`.

## Tasks & Acceptance

**Execution:**
- [x] `firmware/main/ota_update.h` + `ota_update.c` -- expose live failed getter -- critical bind without digging statics.
- [x] `firmware/main/main.c` -- sticky boot-no-MQTT; sample+eval+drive helper; call from failsafe and SoftAP ticks -- CAP-1/CAP-2 e2e both boot paths.
- [x] Host stubs/tests as needed for new OTA getter -- keep `./run.sh` green.
- [x] `idf.py build` -- app links bind path.

**Acceptance Criteria:**
- Given SoftAP active on device, when bind ticks, then LED is magenta slow blink (even if a critical flag is also true).
- Given SoftAP off and any critical true, when bind ticks, then LED is red fast blink.
- Given SoftAP/critical off and ladder flags change, when bind ticks, then LED follows story-2 mapping within one tick and regresses without high-water.
- Given SoftAP early return (no credentials), when SoftAP path tick runs, then LED still updates (not only via `failsafe_task`).
- Given host tests + esp32c6 build, when run, then both succeed.

## Implementation Notes

- `ota_update_failed()` mirrors `ota_update_in_progress()` over live `s_failed` (device API only; host stub not required).
- `status_led_bind_tick()` samples SoftAP / fail-safe / sticky boot-no-MQTT / OTA failed / STA+IP / MQTT / session / OT → `status_led_ladder_eval` → `status_led_set_*`; called from `failsafe_task` (200 ms) and `ota_confirm_tick_task` (SoftAP early-return path, 1 s).
- Also called once before SoftAP early return, inside `app_main` MQTT wait loops, and once before `failsafe_task` create so bring-up colors are not stuck on interim white until task loops.
- Sticky `s_provision_boot_run_no_mqtt` set once at boot when `PROVISION_BOOT_RUN_NO_MQTT`; process restart after re-provision clears.
- Verified: `./run.sh` 13/13; `idf.py build` (esp32c6) OK; ELF links `status_led_bind_tick` / `ota_update_failed` / `status_led_ladder_eval`.
- Review: deferred OTA-cancel→`s_failed` sticky critical after fail-safe clear (bind live flag as-is).

## Spec Change Log

## Review Triage Log

- medium → patch — bind only after `failsafe_task` starts; `app_main` STA/MQTT wait leaves interim white through core bring-up CAP-1 should show — call bind during waits and before SoftAP early return.
- defer — fail-safe `ota_update_cancel` may leave `s_failed` true after OTA abort; pre-existing OTA fail semantics; SPEC binds live `s_failed` as-is (no LED/OTA policy change).
- low (rejected) — SoftAP-path 1 s vs operational 200 ms; AC is “within one tick” of that path; Implementation Notes already state 1 s.
- false — host stub for `ota_update_failed`; device-only getter under `#ifndef HOST_TEST` (same as `ota_update_in_progress`); task intentionally N/A.
- low (rejected) — `ota_confirm_tick_task` comment framing; SoftAP early-return create site is correct.
- low (rejected) — no new HIL file; Verification Manual checks already list device walks; everyday CI is host+build (same as story 1).
- false — empty Spec Change Log / Triage at review start expected before this pass.
- false — Code Map line anchors drift; fix would edit this build’s spec (rejected).
- medium → patch (verification-gap) — live bind unobserved by host tests — accepted as HIL/manual; elevating to new HIL file rejected as above; wiring patches below reduce white-window risk.
- low (rejected) (verification-gap) — sticky boot-no-MQTT only via main; ladder host test already covers critical input; boot sticky is process wiring covered by Manual checks.

## Design Notes

Prefer `provision_is_active()` over OTA’s private SoftAP mirror. Sticky boot-no-MQTT avoids re-reading NVS every 200 ms; process restart after re-provision is the clear path. Piggyback existing ticks — SPEC forbids an LED-only dashboard task. Story-2 tests remain the mapping oracle; story 3 focuses on wiring + SoftAP-path coverage.

## Verification

**Commands:**
- `cd firmware/tests/host && ./run.sh` -- expected: all host tests pass.
- `cd firmware && idf.py build` -- expected: build succeeds with bind linked.

**Manual checks (if no CLI):**
- Flash WeAct Mini: SoftAP → magenta slow; STA progress through yellow/cyan/blue/purple/green per ladder; induce fail-safe or OTA fail → red fast; drop MQTT/OT → LED regresses.
