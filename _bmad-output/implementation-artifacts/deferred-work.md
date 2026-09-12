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
- source_spec: /workspaces/ot-esp32-c6/_bmad-output/implementation-artifacts/spec-firmware-ota-release-ci.md
  summary: Document/enforce that next Release tag must be newer than devices flashing the local APP_FW_VERSION stub
  evidence: Blind-hunter — tagging ≤ stub yields a Release devices may not treat as an update; forge already named local stub mismatch crack
- source_spec: /workspaces/ot-esp32-c6/_bmad-output/implementation-artifacts/spec-firmware-ota-release-ci.md
  summary: Ancestry/tag-gate PR harness reclaimed via check-release-tag.sh --self-test (party 2B)
  evidence: Extracted tag shape + merge-base ancestry; host-tests runs self-test; release.yml calls script
- source_spec: `_bmad-output/specs/spec-status-led-colors/stories/1-ws2812-bring-up-on-io8.md`
  summary: SoftAP early-return status_led_init placement has no automated check against main.c sequencing
  evidence: verification-gap — host tests do not link main.c; moving init below SoftAP return would leave SoftAP boots dark while ./run.sh stays green
- source_spec: `_bmad-output/specs/spec-status-led-colors/stories/1-ws2812-bring-up-on-io8.md`
  summary: bring-up-ladder.md needs exclusive boolean predicates over bind flags (story 2)
  evidence: informal else/highest-wins rungs can overlap; story 1 excludes ladder evaluation
- source_spec: `_bmad-output/specs/spec-status-led-colors/stories/1-ws2812-bring-up-on-io8.md`
  summary: Document ESP32-C6 GPIO8 bootstrap/strapping constraint for status LED pin
  evidence: onboard WS2812 uses a bootstrap pin; future early-init or pin changes need that recorded
