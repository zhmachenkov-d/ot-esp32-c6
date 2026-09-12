---
title: 'Host-testable bring-up ladder evaluator'
type: 'feature'
created: '2026-09-12'
status: 'done'
route: 'dispatch'
review_loop_iteration: 0
baseline_commit: '9b627c3b7f99e107873e63b6c08aa30ff4d6cb3d'
context:
  - '{project-root}/_bmad-output/specs/spec-status-led-colors/SPEC.md'
  - '{project-root}/_bmad-output/specs/spec-status-led-colors/bring-up-ladder.md'
  - '{project-root}/_bmad-output/specs/spec-status-led-colors/stories/1-ws2812-bring-up-on-io8.md'
---

<frozen-after-approval reason="human-owned intent — do not modify unless human renegotiates">

## Intent

**Problem:** CAP-1/CAP-2 ladder logic (SoftAP → critical → else/highest-wins rungs) is not yet encoded as a pure, host-testable mapping; story 3 cannot bind flags to the LED without a trusted evaluator.

**Approach:** Add a pure evaluator that maps SoftAP, critical, and bind-flag inputs to pre-dim RGB + `status_led_pattern_t` per `bring-up-ladder.md`, covered by host Unity tests. No GPIO, timer, or firmware tick wiring.

## Boundaries & Constraints

**Always:**
- Priority: SoftAP → critical → ladder rungs; SoftAP beats critical.
- Critical when any: fail-safe active, `PROVISION_BOOT_RUN_NO_MQTT`, OTA live failed.
- Ladder else/highest-wins (evaluate top rung first): green → purple → blue → cyan → yellow → orange, conditions as in `bring-up-ladder.md`.
- Inputs are a plain bool struct (caller-supplied); do not read file-static state from `main.c` / `ota_update.c`.
- Output uses existing `status_led_pattern_t` (SOLID/SLOW/FAST) and logical RGB before driver dim scale.
- Named-color RGB (pre-dim): magenta `(255,0,255)`, red `(255,0,0)`, orange `(255,96,0)`, yellow `(255,200,0)`, cyan `(0,200,200)`, blue `(0,64,255)`, purple `(160,0,255)`, green `(0,200,0)`.

**Never:**
- Call `status_led_set_*`, init/timer/GPIO, or change story-1 driver behavior.
- Wire SoftAP / MQTT / OT / fail-safe / OTA sampling (story 3).
- Invent a parallel health model or diverge from `bring-up-ladder.md`.
- High-water mark or sticky rung after first achievement.

## I/O & Edge-Case Matrix

| Scenario | Input / State | Expected Output / Behavior | Error Handling |
|----------|--------------|---------------------------|----------------|
| SoftAP wins | SoftAP on; critical also true | Magenta + SLOW | SoftAP preempts critical |
| Critical only | SoftAP off; any critical true | Red + FAST | OR of three critical inputs |
| Green top | Session ready + OT healthy; SoftAP/critical off | Green + SOLID | Highest rung wins |
| Purple | Session ready + OT unhealthy | Purple + SOLID | |
| Blue (OT-only) | Session not ready + OT healthy | Blue + SOLID | Wins over orange even if Wi‑Fi down |
| Cyan | MQTT up + session not ready + OT down | Cyan + SLOW | |
| Yellow | STA+IP + MQTT not connected | Yellow + SLOW | |
| Orange | No STA+IP | Orange + SLOW | Lowest rung |
| Live recompute | Same evaluator called again with lower flags | New lower rung (no sticky) | Pure fn; no memory |

</frozen-after-approval>

## Code Map

- `firmware/main/status_led.h` -- reuse `status_led_pattern_t` only; do not change driver API/behavior.
- `firmware/main/status_led_ladder.h` + `status_led_ladder.c` -- **new** pure evaluator: input struct → RGB + pattern.
- `_bmad-output/specs/spec-status-led-colors/bring-up-ladder.md` -- canonical rung order and colors; implement exactly.
- `firmware/tests/host/test_status_led_ladder.c` -- Unity coverage of I/O matrix rows.
- `firmware/tests/host/CMakeLists.txt` -- register pure executable like `test_status_led` (link ladder `.c` only, no ESP stubs).
- Bind-flag **sources for story 3** (do not wire here): SoftAP `s_softap_active` (`ota_update.c`); STA+IP `s_wifi_up && s_got_ip` (`main.c`); `mqtt_ha_connected()`; `s_mqtt_session_ready` (`main.c`); `ot_poll_boiler_link_healthy()`; `failsafe_is_active()`; `PROVISION_BOOT_RUN_NO_MQTT` via boot/provision path; OTA `s_failed` (`ota_update.c`).
- Do **not** change: `status_led.c` hardware path, OT GPIOs 2/3, SoftAP GPIO 9, MQTT/OTA/fail-safe policy, `main.c` tick placement.

## Tasks & Acceptance

**Execution:**
- [x] `firmware/main/status_led_ladder.h` + `status_led_ladder.c` -- pure input-struct → RGB + pattern per ladder -- CAP-1/CAP-2 logic without I/O.
- [x] `firmware/tests/host/test_status_led_ladder.c` -- unit-test I/O matrix (SoftAP/critical preempt, each rung, no sticky) -- host-proof mapping.
- [x] `firmware/tests/host/CMakeLists.txt` -- build/register `test_status_led_ladder` -- CI via `./run.sh`.
- [x] `firmware/main/CMakeLists.txt` -- compile `status_led_ladder.c` into app -- ready for story 3 link.

**Acceptance Criteria:**
- Given SoftAP and critical both true, when evaluated, then output is magenta slow blink (SoftAP wins).
- Given SoftAP off and any critical true, when evaluated, then output is red fast blink regardless of ladder flags.
- Given SoftAP/critical off and each ladder rung’s inputs in turn, when evaluated, then color+pattern match `bring-up-ladder.md` (else/highest-wins).
- Given a prior higher-rung result, when inputs drop to a lower rung and evaluate again, then output is the new lower rung (no high-water).
- Given `cd firmware/tests/host && ./run.sh`, when tests run, then all host tests including ladder cases pass.

## Implementation Notes

- Pure `status_led_ladder_eval()`: SoftAP → critical (OR of three bools) → rungs 6→1; named pre-dim RGB from story; reuses `status_led_pattern_t` only.
- Host `test_status_led_ladder` covers SoftAP-over-critical, each critical input, all six rungs (incl. blue over orange), and no sticky high-water.
- Linked into app via `main/CMakeLists.txt`; no `status_led_set_*` / tick wiring (story 3).
- Verified: `./run.sh` 13/13; `idf.py build` (esp32c6) OK.

## Spec Change Log

## Review Triage Log

- false — NULL `in` deref; no caller passes NULL (tests and story-3 will use stack structs); unreachable for this API surface.
- false — ladder header includes `status_led.h`; approved reuse of `status_led_pattern_t` from that header; isolating the enum would change story-1 or add a second type source.
- false — cyan without `sta_got_ip`; frozen matrix and `bring-up-ladder.md` cyan row require MQTT up / session not ready / OT down only — not STA+IP.
- medium → patch — SoftAP magenta never asserted without critical flags; SoftAP could be wrongly AND’d with critical and still pass.
- medium → patch — highest-wins of blue (and green/purple) over yellow not pinned when `sta_got_ip && !mqtt_connected` also true.
- false — SoftAP preempting `provision_boot_run_no_mqtt` alone; SoftAP returns before any critical OR, so SoftAP+critical tests already cover that path.
- low (rejected) — duplicated RGB magic numbers; tests pin palette; named constants are extra surface not required by intent.
- low (rejected) — input fields undocumented vs firmware symbols; Code Map already lists story-3 binding sources.
- low (rejected) — no-sticky only green→yellow; matrix live-recompute row already covered.
- false — empty Spec Change Log / Triage at review start is expected before this pass; Implementation Notes already record verification.
- false (edge) — same NULL finding as above.
- medium → patch (verification-gap) — SoftAP-alone magenta missing; same as SoftAP-without-critical above.
- medium → patch (verification-gap) — yellow predicate overlap with higher rungs untested; same as blue/green/purple-over-yellow.

## Design Notes

Highest-wins means check rung 6 → 1 after overrides. Blue (“OT-only”) does not require Wi‑Fi; if session is not ready and OT is healthy, blue beats orange. Critical is three independent bools ORed in the input struct (`failsafe_active`, `provision_boot_run_no_mqtt`, `ota_failed`); story 3 maps live signals onto those fields.

Evaluator must be side-effect free and hold no static rung state so live recompute is automatic.

## Verification

**Commands:**
- `cd firmware/tests/host && ./run.sh` -- expected: all host tests pass including new ladder cases.
- `cd firmware && idf.py build` -- expected: app links with `status_led_ladder.c` (no behavioral LED change yet).
