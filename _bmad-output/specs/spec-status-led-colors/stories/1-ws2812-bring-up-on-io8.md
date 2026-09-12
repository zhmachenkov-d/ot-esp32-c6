---
title: 'WS2812 bring-up on IO8'
type: 'feature'
created: '2026-09-12'
status: 'done'
route: 'dispatch'
review_loop_iteration: 0
baseline_commit: 'ffdf1f1536ac8b0b204557e88f037fa06b5cd5e5'
context:
  - '{project-root}/_bmad-output/specs/spec-status-led-colors/SPEC.md'
---

<frozen-after-approval reason="human-owned intent — do not modify unless human renegotiates">

## Intent

**Problem:** The WeAct Mini onboard WS2812 (IO8) is unused; later bring-up-ladder stories need a dim solid / 1 Hz / 4 Hz drive primitive before any flag binding.

**Approach:** Add a small status-LED driver on GPIO 8 using `espressif/led_strip`, expose set-color + solid/slow/fast pattern APIs, init from `app_main`, and show an interim dim white solid until story 3 owns pattern selection.

## Boundaries & Constraints

**Always:**
- WS2812 on GPIO **8** (`APP_STATUS_LED_GPIO`); brightness ≈ **15%** of full (within SPEC ~10–20%).
- Patterns: **solid**, slow blink **1 Hz**, fast blink **4 Hz** (50% duty); driver owns timing so SoftAP early-return path still animates.
- After init (until story 3): dim **white** solid as temporary “alive” proof — no SoftAP/MQTT/OT/fail-safe binding.
- Prefer host-testable pure timing/phase helpers for blink on/off; hardware path stays behind thin wrappers.

**Never:**
- Bring-up ladder evaluation, SoftAP/critical priority, or subsystem flag binding (stories 2–3).
- A separate “live health dashboard” task whose only job is LED policy.
- Touch OT GPIOs 2/3, SoftAP button GPIO 9, or existing task priorities/stacks except calling into the new driver.
- Claim colorblind palette redesign; story 1 only needs white + blink primitives.

## I/O & Edge-Case Matrix

| Scenario | Input / State | Expected Output / Behavior | Error Handling |
|----------|--------------|---------------------------|----------------|
| Init OK | `status_led_init()` once | Strip ready; dim white solid | Log + return `esp_err_t` on strip create fail |
| Solid | Pattern SOLID, RGB set | Continuous dim color | Ignore redundant set |
| Slow blink | Pattern SLOW, time advances | On ~500 ms / off ~500 ms at 1 Hz | Phase from monotonic ms |
| Fast blink | Pattern FAST, time advances | On ~125 ms / off ~125 ms at 4 Hz | Same |
| SoftAP boot | Credentials missing; `app_main` returns early | LED still inits and keeps pattern (own refresh) | Must not depend on `failsafe_task` |
| Double init | Second `status_led_init()` | Idempotent success or clear error; no leak | Document chosen behavior |

</frozen-after-approval>

## Code Map

- `firmware/main/app_config.h` -- add `APP_STATUS_LED_GPIO` (8); existing OT/SoftAP pins stay.
- `firmware/main/idf_component.yml` -- add `espressif/led_strip`; keep opentherm/mqtt.
- `firmware/main/CMakeLists.txt` -- register `status_led.c`; `REQUIRES` as needed for led_strip.
- `firmware/main/main.c` -- `app_main` (~198+): call `status_led_init()` after OT init (~209) and **before** SoftAP early return (~216–220); do not put LED refresh only on `failsafe_task` (missing on SoftAP path).
- `firmware/main/status_led.c` / `status_led.h` -- **new** driver: init, set RGB, set pattern, internal refresh.
- `firmware/tests/host/` -- add pure-function tests for blink phase (1 Hz / 4 Hz duty); follow `./run.sh` patterns.
- Do **not** change: OT GPIO 2/3, SoftAP GPIO 9, OpenTherm/MQTT/OTA logic, fail-safe policy.

## Tasks & Acceptance

**Execution:**
- [x] `firmware/main/idf_component.yml` -- depend on `espressif/led_strip` -- RMT WS2812 backend for C6.
- [x] `firmware/main/app_config.h` -- define `APP_STATUS_LED_GPIO 8` -- single pin source of truth.
- [x] `firmware/main/status_led.h` + `status_led.c` -- init, set RGB, SOLID/SLOW/FAST patterns, driver-owned refresh (~15% scale, white default) -- story primitives.
- [x] `firmware/main/CMakeLists.txt` -- compile/link `status_led.c` -- build integration.
- [x] `firmware/main/main.c` -- `status_led_init()` before SoftAP early return -- LED alive on both boot paths.
- [x] `firmware/tests/host/` -- unit-test blink on/off phase for 1 Hz and 4 Hz -- I/O matrix without hardware.

**Acceptance Criteria:**
- Given a flashed WeAct Mini, when boot completes (STA or SoftAP path), then the status LED shows dim white solid without serial/MQTT.
- Given the driver API, when pattern is set to SLOW then FAST, then blink rate is 1 Hz then 4 Hz at ~50% duty (HIL or scope/eye).
- Given host tests, when phase helpers run across a 1 s window, then SLOW yields ~500 ms on/off and FAST ~125 ms on/off (±1 sample).
- Given SoftAP early return in `app_main`, when credentials are missing, then LED still initializes and continues refreshing.

## Implementation Notes

- Added `espressif/led_strip` ^2.5.0; `status_led` uses RMT WS2812 GRB, dim scale 38/255, `esp_timer` 40 Hz refresh (independent of `failsafe_task`).
- `status_led_init()` idempotent; called from `app_main` after OT start, before SoftAP early return.
- Host: `test_status_led` covers SOLID/SLOW/FAST phase and double-init; SoftAP/live strip rows rely on call placement + HIL (no device in CI).
- Review patches: non-fatal LED init in `app_main`; shared `s_inited` + host hw stub; `s_lit` only on HW success; clear LED if timer start fails.
- Verified post-patch: `./run.sh` 12/12; `idf.py build` (esp32c6) OK.
## Spec Change Log

## Review Triage Log

- false — memlog still shows resolved OQ as historical append-only entry; later decision supersedes; memlog must not be rewritten.
- defer — bring-up-ladder informal overlapping predicates; story 1 excludes ladder evaluation (story 2).
- medium → patch — `ESP_ERROR_CHECK(status_led_init)` aborted SoftAP on LED failure; fixed to log and continue.
- medium → patch — hollow `test_init_idempotent` under HOST_TEST stub; shared `s_inited` gate + hw stub that fails on second claim.
- medium → patch — `s_lit` advanced on failed strip ops; now only on ESP_OK.
- low (rejected) — missing HIL checklist file; Verification already lists manual checks; everyday CI path is host+build.
- false — timer/API concurrency; story 1 only calls from `app_main` before tasks; story 3 concern.
- defer — GPIO8 bootstrap/strapping awareness for future pin/early-init work.
- false — Code Map line anchors drift; fix would edit this build’s spec (rejected).
- low (rejected) — redundant-set / pattern transition host coverage; phase tests cover rates; unlikely everyday gap.
- false — LED init after `ot_poll_start` matches approved Code Map order.
- false — empty triage/change-log at in-review start is expected before this pass.
- medium → patch — timer start fail after `apply_output(true)` could leave LED lit; clear before teardown.
- low (rejected) — uint32 ms wrap ~49 days; negligible for gateway uptime class.
- low (rejected) — set_rgb/pattern before init; no callers before init in this story.
- medium → patch (verification-gap) — same hollow idempotency as above; fixed.
- defer (verification-gap) — SoftAP `app_main` call placement has no automated check; repo does not host-test `main.c` sequencing.

## Design Notes

Refresh must not live only in `failsafe_task`: that task is created only on the operational path after credentials exist (`main.c` ~286); SoftAP early return (~216–220) never starts it. Prefer a tiny dedicated refresh task or `esp_timer` started from `status_led_init`.

Dim: scale each channel by ~38/255 (≈15%). Interim color: white `(255,255,255)` pre-scale. Story 3 will replace pattern/color selection; leave a single clear `status_led_set_*` entry point.

## Verification

**Commands:**
- `cd firmware/tests/host && ./run.sh` -- expected: all host tests pass including new LED phase tests.
- `cd firmware && idf.py set-target esp32c6 && idf.py build` -- expected: build succeeds with `led_strip` resolved.

**Manual checks (if no CLI):**
- Flash WeAct Mini; confirm dim white solid on IO8 after boot (STA and SoftAP). Optionally call into set-pattern (temporary test hook or debugger) to confirm 1 Hz / 4 Hz.
