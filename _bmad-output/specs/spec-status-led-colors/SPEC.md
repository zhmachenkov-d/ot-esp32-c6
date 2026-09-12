---
id: SPEC-status-led-colors
companions:
  - bring-up-ladder.md
sources:
  - ../../forge/status-led-colors/forged-idea.md
---

> **Canonical contract.** This SPEC and the files in `companions:` are the complete, preservation-validated contract for what to build, test, and validate. Source documents listed in frontmatter are for traceability — consult them only if you need narrative rationale or prose color this contract intentionally omits.

# Status LED bring-up ladder

## Why

**Pain to solve:** during OTC6 bring-up and field recovery, an operator cannot tell how far the gateway got (Wi‑Fi, MQTT, OpenTherm session) without serial logs or a working broker. The WeAct Mini onboard WS2812 should answer that live, so progress and stuck rungs are visible at a glance.

## Capabilities

- **CAP-1**
  - **intent:** The device shows how far bring-up has progressed on the status LED by recomputing the active rung every tick from current subsystem state.
  - **success:** Changing SoftAP / STA+IP / MQTT / session / OT health flags changes the LED within one evaluation tick to the matching rung in `bring-up-ladder.md`; reverting a higher rung drops the LED to the new current rung (no sticky high-water).

- **CAP-2**
  - **intent:** SoftAP commissioning and critical fault conditions preempt all lower ladder rungs, with SoftAP winning over critical when both apply.
  - **success:** With SoftAP active, LED is magenta slow blink even if a critical condition is also true; with SoftAP off and any critical condition true, LED is red fast blink regardless of lower-rung flags.

## Constraints

- Hardware target is WeAct ESP32-C6 Mini WS2812 on **IO8**; drive dim at roughly **10–20%**; slow blink **1 Hz**, fast blink **4 Hz**.
- Priority order is SoftAP → critical → ladder rungs; critical when any of: fail-safe active, `PROVISION_BOOT_RUN_NO_MQTT`, OTA live `s_failed`.
- Rung selection binds only to existing signals: SoftAP active, STA+IP, `mqtt_ha_connected()`, `s_mqtt_session_ready`, `ot_poll_boiler_link_healthy()` — no parallel health model for the LED.
- Color and pattern per rung (and else/highest-wins order among non-override rungs) are defined in `bring-up-ladder.md` and must not diverge in implementation.
- v1 accepts that magenta/purple/blue are hue-adjacent for some colorblind users; blink vs solid is the required differentiation — no separate colorblind palette in this slice.

## Non-goals

- A separate “live health dashboard” job whose only purpose is driving the LED.
- Treating sticky post-rollback or pending-verify OTA states as critical in v1.
- A high-water mark that keeps green (or any higher rung) after first achievement.

## Success signal

On a device, walking SoftAP → Wi‑Fi → MQTT connect → session ready → OT healthy produces the companion’s sequence (magenta → orange/yellow/cyan as applicable → blue/purple → green), SoftAP and critical preempt correctly, and dropping OT or MQTT regresses the LED without sticking on a prior higher rung.

## Assumptions

- The named bind flags already exist in firmware and are the authoritative inputs for rung evaluation.
- OTA live `s_failed` already clears on successful manifest apply, new OTA attempt start, OTA success, and cold boot — bind the live flag; no separate clear-path work for the LED.
