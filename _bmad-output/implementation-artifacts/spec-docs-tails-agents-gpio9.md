---
title: 'Docs tails: AGENTS tag>stub + GPIO9 strapping'
type: 'chore'
created: '2026-10-01'
status: 'done'
route: 'oneshot'
review_loop_iteration: 0
context:
  - '{project-root}/_bmad-output/forge/docs-tails-agents-gpio9/forged-idea.md'
---

<frozen-after-approval reason="human-owned intent — do not modify unless human renegotiates">

## Intent

**Problem:** Two deferred docs tails remain: (A) `AGENTS.md` Policy Release guidance still omits that tag `vX.Y.Z` must be strictly greater than the local `APP_FW_VERSION` stub (gate already lives in `firmware/README.md` + CI); (B) SoftAP button `APP_SOFTAP_BUTTON_GPIO` (GPIO9) has no strapping note at its define, while the GPIO8 LED comment only names GPIO9 as the boot-mode partner.

**Approach:** One small chore: extend the Policy Release bullet in `AGENTS.md` with the tag>stub rule and a pointer to `firmware/README.md`; add a short strapping/early-drive comment only at `APP_SOFTAP_BUTTON_GPIO` in `app_config.h` (not a README SoftAP line, not WeAct remap-binding wording). Do not copy-paste the GPIO8 LED comment.

</frozen-after-approval>

## Implementation Notes

- A: Extended Policy Release bullet in `AGENTS.md` (inside `bmad:context`) with tag **strictly greater** than `APP_FW_VERSION` stub; pointer kept as `firmware/README.md` for details.
- B: Comment only above `APP_SOFTAP_BUTTON_GPIO` — GPIO9 strapping / boot mode with GPIO8 / early drive or boot-time levels; avoided LED-comment copy-paste (no ROM UART; no remap wording; SoftAP button site).
- Untouched: `firmware/README.md`, SoftAP README, pin value `9`, GPIO8 LED comment, CI gate.

## Review Triage Log

- Blind-hunter: Policy parenthetical after publish clause can mis-aim README pointer — **medium** → defer (AGENTS.md review patches deferred; see deferred-work.md).
- Blind-hunter: Policy omits stub path / strip-`v` — **false**; Running bullet names `app_config.h`; README has full gate; forge wanted short Policy + pointer.
- Blind-hunter: Running bullet does not restate tag>stub — **false**; forge locked Item A to Policy Release only (Running-only placement rejected).
- Blind-hunter: Policy does not say numeric SemVer — **false**; compare semantics live in `firmware/README.md` / `check-release-tag.sh` by design.
- Blind-hunter: “Early drive” misfits SoftAP input — **false**; forge locked wording “early drive / boot levels”.
- Blind-hunter: Comment lacks SPI-boot GPIO9-high table — **false**; forge short note only (same depth as GPIO8 LED comment; no boot-mode table).
