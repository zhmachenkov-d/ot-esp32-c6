---
title: 'Host-testable OTA abort failed policy'
type: 'feature'
created: '2026-10-01'
status: 'done'
route: 'dispatch'
review_loop_iteration: 0
baseline_commit: 'c9e1db98b1c21069db702e31391e391cc8082828'
context:
  - '{project-root}/_bmad-output/specs/spec-ota-cancel-neq-failed/SPEC.md'
  - '{project-root}/_bmad-output/forge/deferred-one-slice/forged-idea.md'
---

<frozen-after-approval reason="human-owned intent — do not modify unless human renegotiates">

## Intent

**Problem:** CAP-1 abort→failed policy is not yet a pure, host-testable function; cancel-driven download abort today cannot be proven to preserve `prior_failed` without linking the HTTPS OTA task.

**Approach:** Add `ota_failed_after_download_abort(prior_failed, cancel_caused_abort)` in the existing OTA pure-helper surface and cover cancel / error / sticky prior-failed with Unity. No download-task wiring (story 2).

## Boundaries & Constraints

**Always:**
- Signature semantics: if `cancel_caused_abort` then return `prior_failed`; else return `true`.
- `cancel_caused_abort` means **entry-cause** into the abort path (this abort was cancel-driven), not “cancel was ever set earlier in the session.”
- Declare and implement with other host-visible pure OTA helpers (compiled under `HOST_TEST=1`).
- Host Unity covers at least four cases: cancel+prior false → false; cancel+prior true → true; error+prior false → true; error+prior true → true.

**Never:**
- Change `fail_abort` / `s_failed` assignment in the download task (story 2).
- Touch `status_led`, fail-safe, install-reject (`ota_update_handle_install` reject path), MQTT publishers, or HTTPS OTA task code.
- Put the HTTPS download task under `HOST_TEST`.
- Merge tag>stub or GPIO8 docs work into this story.

## I/O & Edge-Case Matrix

| Scenario | Input / State | Expected Output / Behavior | Error Handling |
|----------|--------------|---------------------------|----------------|
| Cancel, clean prior | `prior_failed=false`, `cancel_caused_abort=true` | returns `false` | N/A |
| Cancel, sticky prior | `prior_failed=true`, `cancel_caused_abort=true` | returns `true` | sticky preserved |
| Real error, clean prior | `prior_failed=false`, `cancel_caused_abort=false` | returns `true` | marks failed |
| Real error, sticky prior | `prior_failed=true`, `cancel_caused_abort=false` | returns `true` | stays failed |

</frozen-after-approval>

## Code Map

- `firmware/main/ota_update.h` -- declare `bool ota_failed_after_download_abort(bool prior_failed, bool cancel_caused_abort);` with other always-on pure helpers (before `#ifndef HOST_TEST`).
- `firmware/main/ota_update.c` -- implement in the pure section (before `#ifndef HOST_TEST` ~line 384); do **not** edit the download-task abort/fail shared tail (~753–760, `s_failed = true` at 757) or the separate install-reject `s_failed = true` (~828).
- `firmware/tests/host/test_ota_update.c` -- add Unity cases for the I/O matrix; register with existing `RUN_TEST` list (already links `otc6_host` → `ota_update.c`).
- `firmware/tests/host/CMakeLists.txt` -- no change required (`test_ota_update` already covers this TU).
- Parent contract: `_bmad-output/specs/spec-ota-cancel-neq-failed/SPEC.md` CAP-1 + pure-helper constraint; story 2 owns wiring.
- Do **not** change: `status_led*.c`, `main.c` fail-safe cancel call, install-reject path, release CI / GPIO8 docs.

## Tasks & Acceptance

**Execution:**
- [x] `firmware/main/ota_update.h` -- declare pure abort→failed helper -- host + device share one policy API.
- [x] `firmware/main/ota_update.c` -- implement helper in pure section only -- no device abort wiring.
- [x] `firmware/tests/host/test_ota_update.c` -- unit-test I/O matrix rows -- host-proof CAP-1 policy.

**Acceptance Criteria:**
- Given `prior_failed=false` and `cancel_caused_abort=true`, when the helper runs, then it returns false.
- Given `prior_failed=true` and `cancel_caused_abort=true`, when the helper runs, then it returns true (sticky prior).
- Given `prior_failed=false` and `cancel_caused_abort=false`, when the helper runs, then it returns true.
- Given `prior_failed=true` and `cancel_caused_abort=false`, when the helper runs, then it returns true.
- Given `cd firmware/tests/host && ./run.sh`, when tests run, then all host tests including the new cases pass.
- Given this story's diff, when reviewed, then no edits to download-task abort/fail tail, install-reject path, LED, or fail-safe.

## Implementation Notes

- Helper implemented in pure section of `ota_update.c` before `#ifndef HOST_TEST`; four Unity asserts in `test_failed_after_download_abort`; `./run.sh` 13/13 pass. Download-task wiring deferred to story 2.

## Spec Change Log

## Review Triage Log

- blind-hunter: Implementation Notes glued to Spec Change Log heading (`…story 2.## Spec Change Log`) — `high` — verified: file had no blank line before `## Spec Change Log`, so heading would not render; route **patch** (blank line restored).
- blind-hunter: empty Spec Change Log / Triage Log while status in-review — `false` — empty change log is normal until a bad_spec loop; triage log is being filled this pass.
- blind-hunter: missing Design Notes section — `low` — rejected: no everyday harm; sibling Design Notes are optional when approach is straightforward.
- blind-hunter: header cites opaque “CAP-1” without SPEC path — `low` — rejected: other pure helpers in the same header omit SPEC paths; CAP-1 is clear from story context.
- blind-hunter: header omits “in the session” from entry-cause phrase — `low` — verified: comment ends at “earlier.” vs frozen Always text; route **patch** (restore full phrase).
- blind-hunter: return value not spelled as value for live failed flag — `false` — comment already says “abort → failed-flag policy” and names keep/set semantics for prior_failed.
- blind-hunter: Verification omits `idf.py build` — `low` — rejected: story intent is host-proof of pure helper; device compile risk for a bool return is negligible for everyday use.
- blind-hunter: no dated verification evidence in Notes — `false` — Notes already record `./run.sh` 13/13 pass; Verification section is the command contract, not a log.
- blind-hunter: story 2 handoff underspecified — `false` — `stories.yaml` id `"2"` and Code Map already defer wiring; no missing artifact for this story’s scope.
- blind-hunter: production still sets `s_failed=true` until story 2 undocumented — `false` — Intent/Never/Implementation Notes already state no download-task wiring in this story.
- blind-hunter: positional `(false,true)` test args invite order mistakes — `low` — rejected: four asserts with comments naming each case; unlikely everyday confusion worth new locals.
- blind-hunter: I/O matrix Error Handling column asymmetric on cancel — `false` — rejected: fix would edit frozen matrix (build-spec edit forbidden); behavior already covered by Expected column.
- edge-case-hunter: no findings — n/a.
- verification-gap: no gaps — n/a.

## Verification

**Commands:**
- `cd firmware/tests/host && ./run.sh` -- expected: all host Unity tests pass, including new abort-policy cases
