---
id: SPEC-ota-cancel-neq-failed
companions:
  - ../../forge/deferred-one-slice/forged-idea.md
sources: []
---

> **Canonical contract.** This SPEC and the files in `companions:` are the complete, preservation-validated contract for what to build, test, and validate. Source documents listed in frontmatter are for traceability — consult them only if you need narrative rationale or prose color this contract intentionally omits.

# OTA cancel ≠ failed

## Why

**Pain to solve:** when fail-safe cancels an in-flight OTA download, the abort path today marks OTA `s_failed`, so after fail-safe clears the status LED stays critical red and MQTT update state can still claim `"failed":true` — a false failure. This is slice **1/3** of the deferred triage in the adopted companion (then tag>stub gate, then GPIO8 docs).

## Capabilities

- **CAP-1**
  - **intent:** A cancel-driven abort of an in-flight OTA download does not mark the update as failed, while aborts caused by real download/apply errors still do, so every consumer of the live failed flag sees the same truth.
  - **success:** After a cancel-driven download-task abort, `ota_update_failed()` is false and the published update-state JSON does not include `"failed":true` from that abort; after an abort caused by begin, perform, incomplete image, sha256 mismatch, or finish error, `ota_update_failed()` is true and the JSON includes `"failed":true`.

## Constraints

- Scope is the **download-task abort path only**; install-reject and manifest poll-cancel semantics are out of this slice.
- Single source of truth in the OTA abort path: no `status_led` module changes, no clear-on-failsafe-exit, no LED-only special case.
- Cancel must not clear a prior sticky failed flag left by a real error.
- Classify abort by **entry-cause** into the abort path (cancel vs error), not by whether cancel was ever set earlier in the session.
- Abort → failed-flag policy lives in a **host-testable pure helper** (e.g. `ota_failed_after_download_abort(prior_failed, cancel_caused_abort)`: cancel keeps `prior_failed`, error → true); the download task only supplies those two booleans into the shared abort tail — no HTTPS OTA task under `HOST_TEST`.
- Keep as its own PR/slice — never merge with tag>stub or GPIO8 docs into one deferred-cleanup change.

## Non-goals

- Release CI enforce tag SemVer strictly greater than `APP_FW_VERSION` stub (slice 2).
- GPIO8 bootstrap/strapping documentation (slice 3).
- SoftAP `status_led_init` placement auto-check; formal exclusive predicates in `bring-up-ladder.md`; one mega “deferred cleanup” PR.
- Changing install-reject failed behavior; automating stub bump after release.

## Success signal

On device: fail-safe cancels in-flight OTA, fail-safe clears, and with no other critical conditions the LED leaves critical while MQTT update state does not claim failed from that cancel; a deliberate real OTA error still yields failed flag + JSON + critical LED. Host Unity tests cover the pure abort helper for cancel vs real-error (and sticky prior-failed) cases.

## Assumptions

- Status LED already binds live `ota_update_failed()` as a critical input; fail-safe already calls `ota_update_cancel()`.
