---
id: SPEC-gpio8-bootstrap-docs
companions:
  - ../../forge/deferred-one-slice/forged-idea.md
sources: []
---

> **Canonical contract.** This SPEC and the files in `companions:` are the complete, preservation-validated contract for what to build, test, and validate. Source documents listed in frontmatter are for traceability — consult them only if you need narrative rationale or prose color this contract intentionally omits.

# GPIO8 bootstrap/strapping docs

## Why

**Pain to solve:** the WeAct Mini onboard WS2812 status LED uses ESP32-C6 **GPIO8**, a boot strapping pin (boot mode with GPIO9; also ROM UART print). That constraint is not recorded next to the firmware LED pin definition, so a future early-init change or pin remap can break boot without anyone noticing. This is slice **3/3** of the deferred triage in the adopted companion (after cancel≠failed and tag>stub).

## Capabilities

- **CAP-1**
  - **intent:** Maintainers can see that the status LED GPIO is an ESP32-C6 strapping/bootstrap pin before they change early init or remapping, so boot-mode constraints stay visible at the point of change.
  - **success:** A short note in firmware docs and/or a comment near `APP_STATUS_LED_GPIO` / the status-LED header states that GPIO8 is an ESP32-C6 strapping pin (boot mode with GPIO9) and that early drive or remaps must respect that.

## Constraints

- Docs chore only — short note; no pin remap, no `status_led_init` reorder, no new host/runtime checks.
- Place the note in firmware docs and/or a comment near the status-LED GPIO definition (`app_config.h` / `status_led` header) — forge accepted either or both.
- Own PR/slice — never merge with cancel≠failed or tag>stub into one deferred-cleanup change.

## Non-goals

- OTA cancel≠failed abort policy (slice 1); release tag > `APP_FW_VERSION` stub gate (slice 2).
- SoftAP `status_led_init` placement auto-check; formal exclusive predicates in `bring-up-ladder.md`.
- Changing which GPIO the LED uses, or changing when `status_led_init` runs.

## Success signal

A reader looking at the LED pin definition or firmware docs learns GPIO8 is an ESP32-C6 bootstrap/strapping pin without digging `knowledge/` or Espressif datasheets; no firmware behavior change is required for this slice.

## Assumptions

- WeAct Mini onboard WS2812 is on GPIO8; `APP_STATUS_LED_GPIO` is already `8` in `firmware/main/app_config.h`.
- Espressif boot mode is controlled by GPIO8/GPIO9 (SPI boot default vs download boot).
