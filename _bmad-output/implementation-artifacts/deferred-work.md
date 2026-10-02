# Deferred work

- source_spec: none
  summary: Host-tests CI workflow on every PR to main and every push to main
  evidence: Split from firmware-ota-release-ci forge intent; independently shippable from tag-driven release asset pipeline
- source_spec: /workspaces/ot-esp32-c6/_bmad-output/implementation-artifacts/spec-firmware-ota-release-ci.md
  summary: Host-tests CI reclaimed into spec-firmware-ota-release-ci (no longer deferred for this PR)
  evidence: Party review — Denys chose widen PR (forge wide-done); still not a release/tag gate
- source_spec: /workspaces/ot-esp32-c6/_bmad-output/implementation-artifacts/spec-firmware-ota-release-ci.md
  summary: PR-time executable harness for release.yml tag-shape and origin/main ancestry gates
  evidence: Verification-gap review — gates live only inline in release.yml; host-tests cannot catch regression until a real tag push
- source_spec: `_bmad-output/specs/spec-release-tag-gt-stub/SPEC.md`
  summary: Enforce Release tag SemVer strictly greater than local APP_FW_VERSION stub (check-release-tag.sh / release.yml)
  evidence: Blind-hunter — tagging ≤ stub yields a Release devices may not treat as an update; forge deferred-one-slice keep/after; stub hygiene after release still manual (surviving crack)
- source_spec: /workspaces/ot-esp32-c6/_bmad-output/implementation-artifacts/spec-firmware-ota-release-ci.md
  summary: Document/enforce that next Release tag must be newer than devices flashing the local APP_FW_VERSION stub — superseded by spec-release-tag-gt-stub
  evidence: Blind-hunter — tagging ≤ stub yields a Release devices may not treat as an update; forge already named local stub mismatch crack; tracking moved to SPEC slice 2/3
- source_spec: /workspaces/ot-esp32-c6/_bmad-output/implementation-artifacts/spec-firmware-ota-release-ci.md
  summary: Ancestry/tag-gate PR harness reclaimed via check-release-tag.sh --self-test (party 2B)
  evidence: Extracted tag shape + merge-base ancestry; host-tests runs self-test; release.yml calls script
- source_spec: `_bmad-output/specs/spec-status-led-colors/stories/1-ws2812-bring-up-on-io8.md`
  summary: SoftAP early-return status_led_init placement has no automated check against main.c sequencing
  evidence: verification-gap — host tests do not link main.c; moving init below SoftAP return would leave SoftAP boots dark while ./run.sh stays green
- source_spec: `_bmad-output/specs/spec-status-led-colors/stories/1-ws2812-bring-up-on-io8.md`
  summary: bring-up-ladder.md needs exclusive boolean predicates over bind flags (story 2)
  evidence: informal else/highest-wins rungs can overlap; story 1 excludes ladder evaluation
- source_spec: `_bmad-output/specs/spec-gpio8-bootstrap-docs/SPEC.md`
  summary: Document ESP32-C6 GPIO8 bootstrap/strapping constraint for status LED pin (firmware docs and/or comment near APP_STATUS_LED_GPIO)
  evidence: onboard WS2812 uses a bootstrap pin; future early-init or pin changes need that recorded; forge deferred-one-slice keep/last; slice 3/3
- source_spec: `_bmad-output/specs/spec-status-led-colors/stories/1-ws2812-bring-up-on-io8.md`
  summary: Document ESP32-C6 GPIO8 bootstrap/strapping for status LED — superseded by spec-gpio8-bootstrap-docs
  evidence: onboard WS2812 uses a bootstrap pin; tracking moved to SPEC slice 3/3
- source_spec: `_bmad-output/specs/spec-ota-cancel-neq-failed/SPEC.md`
  summary: OTA cancel ≠ failed — cancel-driven download abort must not set s_failed; real errors still do; host-test via pure helper ota_failed_after_download_abort
  evidence: Blind-hunter sticky critical LED after fail-safe cancel; forge deferred-one-slice keep/next; party locks CAP bounds + pure helper
- source_spec: `_bmad-output/specs/spec-status-led-colors/stories/3-live-status-led-bind-in-firmware.md`
  summary: Fail-safe ota_update_cancel may leave OTA s_failed true so LED stays critical red after fail-safe clears — superseded by spec-ota-cancel-neq-failed
  evidence: Blind-hunter — cancel aborts in-flight OTA into fail path; LED SPEC binds live s_failed as-is; clear policy moved to OTA abort-path SPEC
- source_spec: `_bmad-output/specs/spec-ota-cancel-neq-failed/stories/2-wire-download-task-abort-to-cancel-neq-failed.md`
  summary: OTA cancel sampling during sha256 verify / before finish — resolved (cancel_now + mid-sha256 poll)
  evidence: Walkthrough follow-up patch on fix/ota-cancel-neq-failed-wire
- source_spec: `_bmad-output/specs/spec-ota-cancel-neq-failed/stories/2-wire-download-task-abort-to-cancel-neq-failed.md`
  summary: HIL OTA cancel≠failed checklist case — resolved (v10_ota.md case 7; results ✓ 2026-10-02)
  evidence: Serial Starting OTA → Writing ota_1 → OTA cancelled; stayed 0.2.2; docs/hil-ota-results-3-7

- source_spec: /workspaces/ot-esp32-c6/_bmad-output/implementation-artifacts/spec-release-tag-gt-stub.md
  summary: AGENTS.md still omits Release tag must be strictly greater than APP_FW_VERSION stub
  evidence: Blind-hunter — firmware/README.md documents the gate; agent-facing AGENTS.md release guidance does not
- source_spec: /workspaces/ot-esp32-c6/_bmad-output/implementation-artifacts/spec-gpio8-bootstrap-docs.md
  summary: SoftAP button GPIO9 (APP_SOFTAP_BUTTON_GPIO) is also an ESP32-C6 strapping pin and remains undocumented at its define / SoftAP docs
  evidence: Blind-hunter — new GPIO8 note mentions boot mode with GPIO9 but does not document GPIO9 remaps; out of SPEC-gpio8-bootstrap-docs LED-only scope

- source_spec: /workspaces/ot-esp32-c6/_bmad-output/implementation-artifacts/spec-docs-tails-agents-gpio9.md
  summary: AGENTS.md Policy Release parenthetical "(details in firmware/README.md)" sits after the publish clause, so it can read as documenting asset publish rather than tag>stub compare semantics
  evidence: Blind-hunter — agents following the pointer for comparison rules may miss that the full gate (strip v, stub path, equal/older fail) lives in firmware/README.md; deferred because review patches must not edit AGENTS.md
