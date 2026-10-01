---
title: 'Wire download-task abort to cancel≠failed'
type: 'feature'
created: '2026-10-01'
status: 'done'
route: 'oneshot'
review_loop_iteration: 0
baseline_commit: 'e76ade24dc8905e4e6e2c6d06f5ba9dd505d8d9c'
context:
  - '{project-root}/_bmad-output/specs/spec-ota-cancel-neq-failed/SPEC.md'
  - '{project-root}/_bmad-output/forge/deferred-one-slice/forged-idea.md'
  - '{project-root}/_bmad-output/specs/spec-ota-cancel-neq-failed/stories/1-host-testable-ota-abort-failed-policy.md'
---

<frozen-after-approval reason="human-owned intent — do not modify unless human renegotiates">

## Intent

**Problem:** The download-task shared abort tail still always sets `s_failed = true`, so a fail-safe cancel of an in-flight OTA leaves a false failed flag for LED/MQTT after fail-safe clears.

**Approach:** Classify abort entry-cause (cancel vs real error) in the download task, feed `prior_failed` and that cause into `ota_failed_after_download_abort`, and assign `s_failed` from the helper so cancel keeps prior while begin/perform/incomplete/sha256/finish errors still mark failed.

</frozen-after-approval>

## Implementation Notes

- Entry-cause: local `cancel_caused_abort` set true only on the `s_cancel` → `fail_abort` branch; not read from `s_cancel` at the shared tail (avoids concurrent cancel after an error entry).
- Shared `fail_abort`/`fail` tail: `s_failed = ota_failed_after_download_abort(s_failed, cancel_caused_abort)` — covers cancel + begin/perform/incomplete/sha256/finish (and pre-begin slot/size) error paths.
- Left install-reject `s_failed = true`, LED, fail-safe, MQTT publishers untouched.
- Note: `ota_task` still clears `s_failed` at entry (pre-existing); so during a normal download abort, `prior_failed` at the helper call is false. Sticky-prior is still correct if ever true; host-proven in story 1.
- Host: `cd firmware/tests/host && ./run.sh` — 13/13 pass after wiring.

## Review Triage Log

- blind-hunter: story missing full dispatch sections / Boundaries / Tasks / Code Map / Verification despite checkpoints — `false` — oneshot route intentionally keeps only Intent + Implementation Notes.
- blind-hunter: frozen Intent-only omits Always/Never — `false` — oneshot deletes Boundaries when route gate is clean.
- blind-hunter: no Given/When/Then AC mapping CAP-1 — `false` — oneshot; Intent states CAP-1 wiring outcome.
- blind-hunter: no Verification / HIL / idf.py named — `false` for oneshot template; host run recorded in Notes. HIL gap deferred separately.
- blind-hunter: status in-progress while Notes look finished — `false` — mid-oneshot workflow; status set to done at finalize.
- blind-hunter: prior always false because task clears s_failed at entry — `low` — rejected as bug (pre-existing clear-on-new-attempt); noted in Implementation Notes.
- blind-hunter: cancel not sampled during sha256/finish — `defer` — pre-existing poll window; not introduced by this wiring.
- blind-hunter: HIL v10_ota.md lacks fail-safe cancel≠failed case — `defer` — on-device success signal still manual; checklist update is separate.
- blind-hunter: Code Map absent — `false` — oneshot.
- blind-hunter: Unicode curly quotes in new C comment — `low` — patched to ASCII.
