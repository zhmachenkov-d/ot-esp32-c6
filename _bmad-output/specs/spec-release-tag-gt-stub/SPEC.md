---
id: SPEC-release-tag-gt-stub
companions:
  - ../../forge/deferred-one-slice/forged-idea.md
sources: []
---

> **Canonical contract.** This SPEC and the files in `companions:` are the complete, preservation-validated contract for what to build, test, and validate. Source documents listed in frontmatter are for traceability — consult them only if you need narrative rationale or prose color this contract intentionally omits.

# Release tag > APP_FW_VERSION stub

## Why

**Pain to solve:** a Release tag whose SemVer is less than or equal to the local `APP_FW_VERSION` stub in `firmware/main/app_config.h` can publish assets that devices already flashing that stub will not treat as newer — a silent non-update. This is slice **2/3** of the deferred triage in the adopted companion (after cancel≠failed, before GPIO8 docs).

## Capabilities

- **CAP-1**
  - **intent:** The release gate accepts only tags whose version is strictly newer than the checked-in `APP_FW_VERSION` stub, so a tag push cannot publish an update that stub-flashed devices ignore.
  - **success:** With stub `X.Y.Z` in `app_config.h`, `check-release-tag.sh` rejects `vX.Y.Z` and any lower SemVer tag, accepts a strictly greater tag that still passes existing shape (and ancestry when requested); `--self-test` fails the job if those cases regress; `release.yml` runs the check before the IDF build.

## Constraints

- Extend existing `.github/scripts/check-release-tag.sh` (and its `release.yml` invocation) — do not invent a parallel gate script.
- Compare tag version without the leading `v` to the string-literal stub default for `APP_FW_VERSION` in `firmware/main/app_config.h`; both sides are strict `X.Y.Z` (same shape as the existing tag regex).
- `--self-test` (already run from host-tests CI) must cover accept-newer / reject-equal / reject-older against a fixture stub.
- Own PR/slice — never merge with cancel≠failed or GPIO8 docs into one deferred-cleanup change.

## Non-goals

- Automating stub bump in `app_config.h` after a successful release (surviving crack stays manual).
- OTA cancel≠failed abort policy (slice 1); GPIO8 bootstrap docs (slice 3).
- Making host-tests a tag-push quality gate; changing tag-shape or main-ancestry rules beyond adding the stub compare.

## Success signal

Pushing a tag ≤ the stub fails the Release workflow at the tag-check step with a clear error; pushing a strictly newer valid tag on `main` proceeds to build. `bash .github/scripts/check-release-tag.sh --self-test` stays green on every host-tests CI run and encodes the >/=/< stub cases.

## Assumptions

- `check-release-tag.sh` already validates tag shape and optional main ancestry; `release.yml` already calls it before build; local stub remains under `#ifndef APP_FW_VERSION` in `app_config.h`.
