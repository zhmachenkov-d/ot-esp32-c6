---
title: 'Firmware release CI + host-tests CI'
type: 'feature'
created: '2026-09-09'
status: 'done'
route: 'dispatch'
review_loop_iteration: 0
baseline_commit: '1cff388620546e626cec8c4c0f35591432d8be1a'
context:
  - '{project-root}/_bmad-output/forge/firmware-ota-release-ci/forged-idea.md'
  - '{project-root}/firmware/README.md'
  - '{project-root}/AGENTS.md'
---

<frozen-after-approval reason="human-owned intent — do not modify unless human renegotiates">

## Intent

**Problem:** Device dual-slot OTA already polls GitHub Releases `manifest.json` and installs HTTPS assets, but release assets are still manual, and nothing runs host unit tests on GitHub Actions. Tagging `vX.Y.Z` does not reliably produce matching assets; `main` can rot without a continuous test signal.

**Approach:** Ship forge “wide done” in one PR as two independent workflows: (1) tag-push release CI — ESP-IDF `esp32c6` build with size gate, tag-injected version, draft→upload `otc6_gateway.bin` + generated `manifest.json`→publish; (2) host-tests CI on every PR to `main` and every push to `main`. Host tests are **not** a release/tag gate.

## Boundaries & Constraints

**Always:**
- Release trigger only on push of strict SemVer tags `vMAJOR.MINOR.PATCH` (no prerelease/suffix channel).
- Tag version wins: built `APP_FW_VERSION` and manifest `version` both equal the tag without the leading `v`.
- Publish safety: draft Release → upload both assets → publish only when uploads succeed.
- Generated `manifest.json` always includes: `manifest_version` (1), `firmware_id` (`otc6_gateway`), `version`, `url` (https asset URL for `otc6_gateway.bin`), `sha256` (64 lowercase hex), `size`, `release_url` (tag page). No hand-edited Release JSON.
- Reuse existing CMake OTA size gate; fail the release job if the build/size gate fails.
- Release and host-tests CI image: `espressif/idf:release-v5.4` (same floating train as `.devcontainer`). Tagged commit must be reachable from `origin/main`.
- Host-tests workflow: every pull_request targeting `main` and every push to `main`; run `firmware/tests/host/./run.sh` (needs `IDF_PATH`). No path filters required for done.
- Document: local/dev `APP_FW_VERSION` is a stub; tag-push release is **not** quality-gated by host tests (continuous signal on `main` only).
- Document IDF drift explicitly: CI/devcontainer track floating `release-v5.4`; a local `dependencies.lock` pin (e.g. 5.4.4) may differ — accepted, not a silent surprise.

**Never:**
- Change device OTA parse/download/HA Install behavior (`ota_update.c` / `.h`), partition layout, or `OTA_MANIFEST_URL` / `OTA_FIRMWARE_ID` semantics.
- Add prerelease/suffixed-tag OTA channels, release-event triggers, publish-before-assets, or make host tests a `needs:`/gate on the release workflow.
- Invent Zigbee paths or release branches.

## I/O & Edge-Case Matrix

| Scenario | Input / State | Expected Output / Behavior | Error Handling |
|----------|--------------|---------------------------|----------------|
| Release happy path | Push tag `vX.Y.Z` on `origin/main` | Draft → bin + manifest uploaded → published; versions match | N/A |
| Size gate fail | `.bin` exceeds CMake limit | Release job fails; no published incomplete release | Stop before publish |
| Bad tag shape | Not strict `vMAJOR.MINOR.PATCH` | Release workflow does not run / fails validation | No Release |
| Tag not on main | Tag commit not reachable from `origin/main` | Job fails before draft/publish | No `/latest` update |
| Partial upload | Asset upload fails after draft | Stays draft; job fails | Do not publish |
| Host tests green | PR→`main` or push→`main` | Host-tests workflow runs `./run.sh` and passes | N/A |
| Host tests red | Same triggers, failing tests | Workflow fails; does **not** block or cancel an in-flight tag release | Report failure only |

</frozen-after-approval>

## Code Map

- `firmware/main/app_config.h` — `#ifndef` around `APP_FW_VERSION` for CI inject; leave `OTA_*` alone.
- `firmware/CMakeLists.txt` + `firmware/cmake/check_ota_size.cmake` — reuse size gate; do not weaken.
- `firmware/main/ota_update.c` / `firmware/tests/host/test_ota_update.c` — contract reference only; do not change parse rules.
- `firmware/tests/host/run.sh` — host-tests entrypoint for CI.
- `firmware/README.md` — tag→CI assets + stub caveat + note that host tests are continuous on `main`, not a tag gate.
- `.devcontainer/*` — CI image aligned to `espressif/idf:release-v5.4`.
- **New** `.github/workflows/release.yml` — tag trigger, main-ancestry, build, draft→assets→publish.
- **New** `.github/workflows/host-tests.yml` (name flexible) — PR + push to `main`; `./run.sh`.
- **New** `.github/scripts/generate-ota-manifest.sh` (or equiv.) — emit manifest JSON.
- Do not touch `firmware/flash.sh`.

## Tasks & Acceptance

**Execution:**
- [x] `firmware/main/app_config.h` -- `#ifndef` wrap `APP_FW_VERSION`; keep local stub default -- tag-wins inject
- [x] `.github/scripts/generate-ota-manifest.sh` (or equiv.) -- Emit v1 manifest with required + always-on fields -- no hand-edited JSON
- [x] `.github/workflows/release.yml` -- Strict `vX.Y.Z` on `origin/main`: IDF `release-v5.4` build, inject version, draft→assets→publish -- release assets
- [x] `.github/workflows/host-tests.yml` -- On PR to `main` and push to `main`, run `firmware/tests/host/./run.sh` with IDF available -- continuous signal (not a release `needs:`)
- [x] `firmware/README.md` (+ `AGENTS.md` one-liner if needed) -- Document both workflows, stub version caveat, non-gating of tags by host tests, and floating `release-v5.4` vs possible local lockfile drift -- operator-honest / agent-honest
- [x] Manifest generator coverage -- Cheap self-test / check of required fields + sha256 shape -- catch bad generator

**Acceptance Criteria:**
- Given tag `vX.Y.Z` on `origin/main`, when release workflow succeeds, then published Release has `otc6_gateway.bin` + `manifest.json` and embedded version equals `X.Y.Z`.
- Given those assets, when `manifest.json` is inspected, then it has `manifest_version: 1`, `firmware_id: "otc6_gateway"`, matching `version`, https `url` to the bin, 64-char lowercase `sha256`, numeric `size`, and `release_url` for the tag.
- Given size-gate failure, when release workflow runs, then job fails and no published Release lacks both final assets.
- Given non-strict tag or tag not on `main`, when pushed, then no OTA `/latest` publish from this release workflow.
- Given a PR to `main` or push to `main`, when host-tests workflow runs, then it executes `firmware/tests/host/./run.sh` and fails the check on test failure.
- Given a failing host-tests run, when a tag release workflow is considered, then host tests are not a `needs:` dependency of release (tag may still publish — accepted crack).
- Given local builds without CI injection, when `APP_FW_VERSION` is used, then stub default remains and README states it may not match any Release.

## Implementation Notes

- Version inject: `#ifndef` in `app_config.h` + `target_compile_definitions` in `firmware/main/CMakeLists.txt` for `idf.py -DAPP_FW_VERSION=X.Y.Z`. Local verify: build with `9.9.9` embedded in `otc6_gateway.bin`; size gate OK.
- Release workflow split: `build` job in `espressif/idf:release-v5.4` (no `gh` in image) → artifact → `publish` on `ubuntu-latest` with `gh` for draft→upload→publish. Host-tests is not a `needs:` of release.
- Manifest generator `--self-test` runs in host-tests workflow and before upload in publish job.
- Matrix coverage: host green via `./run.sh` (passed); manifest shape via generator self-test (passed); tag/main/size/draft-order covered by release.yml steps (not executable without a real tag push).
- Review patches: post-build `grep` for injected version in bin; draft reuse + `--clobber` upload; `size > 0`; strict `owner/name` REPO; full `https://github.com/...` URL asserts in validate/self-test; README CMake-cache note for local `-DAPP_FW_VERSION`.

## Spec Change Log

- 2026-09-09 party review (Denys): Reclaimed host-tests CI into this PR (forge wide-done). Still not a release/tag gate. Avoids shipping automated `/latest` while `main` has no continuous host-test signal. Prior Build split to `deferred-work.md` superseded for this work.
- 2026-09-09 party review (Denys): Keep CI on floating `espressif/idf:release-v5.4` (door 1); require README/AGENTS note that local lockfile may lag — do not pin CI to 5.4.4 in this PR.

## Review Triage Log

- blind: cmake cache keeps stale APP_FW_VERSION on later local builds — **medium** — real for local rebuilds without clean/`-U`; CI runners are fresh so release path safe. Route later as patch (README one-liner).
- blind+edge+vg: release build never asserts injected version in `.bin` — **high** — verified: workflow only `test -f` bin; inject failure ships stub vs tag manifest. **patch**
- blind+edge: `gh release create` not idempotent on re-run — **medium** — verified: second run fails if draft exists. **patch**
- blind: deferred-work.md first entry still says host-tests deferred — **false** for product code; agent-doc noise. Reject (fix is edit deferred list / agent context).
- blind: Code Map omits CMakeLists inject — **false** route: Reject — fix would edit this build's spec.
- blind+edge: size≥0 allows empty bin — **medium** — verified in validate + generator. **patch**
- blind+edge: weak REPO `owner/name` check — **medium** — verified: `*/*` allows extra segments. **patch**
- blind: CI omits title/summary — **false** — frozen Always lists required/always-on fields; title/summary not required.
- blind: README example shows title/summary vs CI — **low** — reject: unlikely everyday harm; already notes optional extras.
- blind: README missing concrete git tag/push sequence — **low** — reject: more than a trivial deletion; operators know tagging.
- blind+vg: self-test/validate weak on full github.com URL — **medium** — vg pre-verified; endswith-only. **patch**
- blind: stub 0.2.2 vs tagging ≤stub — **defer** — surviving forge crack already documented; not introduced as new silent bug beyond docs.
- edge: CMake accepts non-X.Y.Z APP_FW_VERSION — **false** for release (tag regex sets VERSION); local misuse only, no demonstrated CI path.
- vg: tag/main-ancestry gates have no PR-time harness — **medium** (unverified-at-PR) — **defer** — follow-up harness beyond this story’s chosen verification plan.

## Design Notes

**Two workflows, no coupling:** Release and host-tests must not `needs:` each other. Tag publish remains ungated by tests (surviving forge crack); host tests make the crack visible on `main` before people tag. Both jobs use `espressif/idf:release-v5.4`; document lockfile drift rather than pinning this PR.

**Version injection:** `#ifndef APP_FW_VERSION` + CI compile define from tag (strip `v`). No sed-commit of `app_config.h`.

**Manifest URLs:** From `github.repository` + tag; do not hardcode owner/repo in CI defaults.

**Golden manifest shape (illustrative):**

```json
{
  "manifest_version": 1,
  "firmware_id": "otc6_gateway",
  "version": "0.2.3",
  "url": "https://github.com/OWNER/REPO/releases/download/v0.2.3/otc6_gateway.bin",
  "sha256": "<64 lowercase hex of otc6_gateway.bin>",
  "size": 1149360,
  "release_url": "https://github.com/OWNER/REPO/releases/tag/v0.2.3"
}
```

**Publish order:** draft → upload both assets → publish. Never publish empty first.

## Verification

**Commands:**
- `bash .github/scripts/generate-ota-manifest.sh …` -- expected: required keys; lowercase 64-hex sha256
- `cd firmware && idf.py set-target esp32c6 && idf.py build` with injected version -- expected: `firmware/build/otc6_gateway.bin`; size gate OK
- `cd firmware/tests/host && ./run.sh` -- expected: pass (same entry host-tests CI uses)
- Workflow YAML / `actionlint` if available -- expected: no schema errors; no `needs:` from release→host-tests

**Manual checks:**
- Two workflow files; release has no host-test gate; `ota_update.c` / partitions / `OTA_*` unchanged except `APP_FW_VERSION` guard
