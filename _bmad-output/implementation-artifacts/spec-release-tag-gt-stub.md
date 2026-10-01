---
title: 'Release tag > APP_FW_VERSION stub'
type: 'feature'
created: '2026-10-01'
status: 'done'
route: 'oneshot'
review_loop_iteration: 0
context:
  - '{project-root}/_bmad-output/specs/spec-release-tag-gt-stub/SPEC.md'
  - '{project-root}/_bmad-output/forge/deferred-one-slice/forged-idea.md'
---

<frozen-after-approval reason="human-owned intent — do not modify unless human renegotiates">

## Intent

**Problem:** A Release tag whose SemVer is ≤ the checked-in `APP_FW_VERSION` stub can publish assets that stub-flashed devices ignore as not newer — a silent non-update.

**Approach:** Extend `.github/scripts/check-release-tag.sh` so the release gate rejects tags that are not strictly greater than the string-literal stub in `firmware/main/app_config.h`; keep `--self-test` covering accept-newer / reject-equal / reject-older against a fixture stub; leave `release.yml` calling the same script before the IDF build.

</frozen-after-approval>

## Implementation Notes

- Extended `.github/scripts/check-release-tag.sh`: parse first `#define APP_FW_VERSION "X.Y.Z"` stub from header (default `firmware/main/app_config.h`), numeric `semver_gt`, reject tag ≤ stub; optional `--stub-header` for fixtures.
- `--self-test` covers accept-newer / reject-equal / reject-older (fixture stub `1.2.3`), `semver_gt` unit cases, CLI `--stub-header` path, and ancestry; expected-failure stderr silenced.
- `release.yml` step renamed to mention stub-newer; host-tests still runs `--self-test` unchanged.
- Documented >stub gate and “don’t bump stub before tagging” in `firmware/README.md`.
- Verified: `--self-test` passes (clean stdout); live stub `0.2.2` accepts `v0.2.3`, rejects `v0.2.2` / `v0.2.1`.
- Review patches applied; AGENTS.md stub-rule lag deferred.

## Review Triage Log

- Ancestry expected-fail stderr noise in `--self-test` — **medium** → patch (redirect like stub reject cases).
- Parser comment said `#ifndef` but code takes first `#define` — **low** → patch (comment only); claim that wrong define is read is **false** for current single-define header.
- No self-test for stub-parser error paths — **low** rejected (SPEC requires >/ = / < only; rare path).
- No assert on exact reject error text — **low** rejected (exit code sufficient).
- `release.yml` step title omitted stub-newer — **medium** → patch (rename step).
- README missing stub-bump-before-tag pitfall — **medium** → patch (one sentence).
- Recovery paragraph silent on ≤stub tag-check fail — **low** rejected (covered by new stub-order sentence; draft recovery is unrelated).
- `AGENTS.md` lags stub-newer rule — **medium** → defer (AGENTS.md edits are defer-class).
- Spec `status: in-progress` while notes look done — **false** (finalize sets `done`).
- `--stub-header` unused in self-test — **low** → patch (CLI drive via fresh process).
- Bash `semver_gt` vs firmware `ota_semver_cmp` — **false** (CI bash gate cannot link C; same X.Y.Z numeric order).
- Artifact “documented” overstated vs AGENTS — **false** as product bug; AGENTS deferred separately.
