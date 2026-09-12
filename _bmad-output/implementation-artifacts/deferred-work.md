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
