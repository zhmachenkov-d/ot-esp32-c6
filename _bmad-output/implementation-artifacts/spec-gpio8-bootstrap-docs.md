---
title: 'GPIO8 bootstrap/strapping docs'
type: 'chore'
created: '2026-10-01'
status: 'done'
route: 'oneshot'
review_loop_iteration: 0
context:
  - '{project-root}/_bmad-output/specs/spec-gpio8-bootstrap-docs/SPEC.md'
  - '{project-root}/_bmad-output/forge/deferred-one-slice/forged-idea.md'
---

<frozen-after-approval reason="human-owned intent — do not modify unless human renegotiates">

## Intent

**Problem:** The WeAct Mini onboard WS2812 status LED uses ESP32-C6 GPIO8 (boot strapping with GPIO9; also ROM UART print), but that constraint is not recorded next to the LED pin definition or in firmware docs, so a future early-init change or pin remap can break boot unnoticed.

**Approach:** Add a short strapping/bootstrap note at the pin definition and in firmware docs so maintainers see the constraint before changing early init or remapping — docs/comments only; no behavior, pin, or init-order changes.

</frozen-after-approval>

## Implementation Notes

- Placement: comment on `APP_STATUS_LED_GPIO` in `app_config.h` (point of pin change) plus short `## Status LED` in `firmware/README.md` — forge/SPEC allow either or both; chose both for definition + docs discoverability.
- Wording mirrors SPEC: GPIO8 strapping, boot mode with GPIO9, ROM UART print, early drive/remaps must respect — no boot-mode table or CHIP_PU timing (short note only).
- Untouched: pin value `8`, `status_led_*`, `status_led_init` placement, SoftAP GPIO9 docs (deferred).

## Review Triage Log

- “early drive” timing / CHIP_PU hold window missing — **low** rejected (SPEC CAP success is short strapping note; full Espressif timing is knowledge/datasheet territory).
- Boot-mode level table missing — **low** rejected (same; short note constraint).
- SoftAP GPIO9 strapping undocumented — **medium** → defer (out of LED GPIO8 SPEC scope; note mentions GPIO9 without documenting SoftAP button define).
- No note in `status_led.h` — **low** rejected (CAP satisfied via `app_config.h` + README; third copy increases drift).
- “ROM UART print” oversimplifies eFuses — **false** (mirrors SPEC wording; eFuse nuance is out of short-note scope).
- No pointer to `knowledge/esp32/…` boot table — **low** rejected (SPEC success is learn without digging knowledge; optional deepen beyond chore).
- README and header duplicate phrasing — **false** (SPEC/forge explicitly allow both placements).
- Impl artifact still `in-progress` mid-oneshot — **false** (finalize sets `done`).
