---
date: 2026-09-12
verdict: accepted
criteria: declared
headless: false
---

# Retrospective: Status LED bring-up ladder

## Epic summary

- **Epic:** stories-mode folder `_bmad-output/specs/spec-status-led-colors/` (Status LED bring-up ladder / SPEC-status-led-colors).
- **Confirmed by:** Denys (interactive).
- **Stories completed** (`stories.yaml` order; all frontmatter `status: done`):
  1. WS2812 bring-up on IO8 — `stories/1-ws2812-bring-up-on-io8.md`
  2. Host-testable bring-up ladder evaluator — `stories/2-host-testable-bring-up-ladder-evaluator.md`
  3. Live status LED bind in firmware — `stories/3-live-status-led-bind-in-firmware.md`
- **pending_stories:** none.
- **Declared acceptance:** SPEC CAP-1/CAP-2 + Success signal; per-story AC in story artifacts. Phase 4 uses **declared** criteria.

### Diff ranges (stories mode)

| Story | Baseline (recorded) | Range end | Range | Notes |
|-------|---------------------|-----------|-------|-------|
| 1 | `ffdf1f1536ac…` | next baseline `9b627c3b7f99…` | `ffdf1f15..9b627c3b` | 1 commit: `9b627c3` feat(led) WS2812 |
| 2 | `9b627c3b7f99…` | next baseline `cf4c6c385217…` | `9b627c3b..cf4c6c38` | 1 commit: `cf4c6c3` feat(led) ladder evaluator |
| 3 | `cf4c6c385217…` | `HEAD` (**inferred**) | `cf4c6c38..HEAD` | Non-merge: `40f3e20` bind; merge: `9bf53f3` PR #18 (`merge_files` restates — not summed into `files`) |

Primary firmware commits: `9b627c3`, `cf4c6c3`, `40f3e20`. Aggregate review diff: `ffdf1f15..40f3e20` (`/tmp/retro-status-led-epic.diff`).

### Evidence inventory

| Item | Status | Path / note |
|------|--------|-------------|
| Epic spec | Available | `SPEC.md` + `bring-up-ladder.md` |
| Story artifacts | Available | `stories.yaml` + three `stories/*.md` |
| Diff / commits | Available | Ranges above |
| Sprint status | **Missing** | Stories mode — no `sprint-status.yaml` |
| Previous retrospective | **Missing** | None under `_bmad-output/` |
| Session logs | **Missing** | Process-lesson analysis skipped |
| Deferred work | Available | `_bmad-output/implementation-artifacts/deferred-work.md` |
| Forge brief | Available | `_bmad-output/forge/status-led-colors/forged-idea.md` |

### Narrowed scope (missing evidence)

- **Process-lesson analysis:** skipped — no session logs.
- **Previous-retro follow-through:** N/A — no prior retro.
- **Sprint-status update (Phase 5):** skipped (stories mode).
- **Device LED walk this retro:** not observed — monitor log in terminal 6 has no LED color/pattern evidence; HIL success signal unchecked here.

### Phase status

- [x] Phase 1 — Gather
- [x] Phase 2 — Analyze
- [x] Phase 3 — Team Discussion (skipped; default)
- [x] Phase 4 — Decide
- [x] Phase 5 — Finalize

## Findings

### Aggregate views

#### Architecture delta

- **New leaf modules:** `status_led.c` / `status_led.h` (WS2812 + blink timer) and `status_led_ladder.c` / `status_led_ladder.h` (pure eval). Bind stays private in `main.c` (`status_led_bind_tick`).
- **Thin OTA surface:** `ota_update_failed()` over live `s_failed` (`ota_update.h` / `ota_update.c:845-848`).
- **Layering:** bind → eval → driver; no cycles; eval has no firmware statics. Matches story split intent.
- **Source:** `git_evidence` story ranges; `firmware/main/main.c:44-59`; commits `9b627c3`, `cf4c6c3`, `40f3e20`.

#### God-class / size growth

| Path | Net (non-merge, by story `files`) | Absolute lines now |
|------|-----------------------------------|--------------------|
| `status_led.c` | +208 (story 1) | 208 |
| `test_status_led_ladder.c` | +220 (story 2) | 220 |
| `main.c` | +8 / +28 (stories 1+3) | 325 |
| `status_led_ladder.c` | +43 (story 2) | 43 |
| `ota_update.c` | +5 (story 3) | 856 (pre-existing; epic did not grow a god class) |

No new god-class from this epic; `main.c` remains the orchestration hub (~325 lines).

#### Duplication map

- No duplicate ladder mapping — single `status_led_ladder_eval`; host tests drive it with constructed inputs.
- `ota_update_failed` mirrors `ota_update_in_progress` pattern (`#ifndef HOST_TEST`).

#### Pattern divergence

- HOST_TEST stubs for LED driver match existing host-test conventions (`test_status_led.c`).
- SoftAP-path tick at 1 s vs failsafe 200 ms — intentional reuse of `ota_confirm_tick_task`; story 3 review already accepted “within one tick of that path.”

#### Spec-to-implementation reconciliation

| Spec claim | As-built | Disposition |
|------------|----------|-------------|
| CAP-1 live recompute, no high-water | Eval is pure; bind samples each tick | Met (with boot-window caveat below) |
| SoftAP > critical > ladder | `status_led_ladder.c:17-23` | Met |
| Bind named flags only | `main.c:46-55` | Met |
| Colors/patterns in companion | RGB/pattern match `bring-up-ladder.md` | Met |
| OTA `s_failed` clear policy unchanged | Bind live flag; cancel does not clear | Matches SPEC; sticky-red after cancel is accepted residual |
| Success signal (device walk) | Story 3 lists manual checks; this retro did not re-observe | Open item |

### Diff-scope review (`bmad-review` lenses)

Lenses: adversarial, edge-case-hunter, verification-gap on `ffdf1f15..40f3e20`. Findings below are **re-checked against primary sources**; subagent-only claims that did not hold were dropped (e.g. NULL-input crash as a realistic bind path; claim that session_ready stays true indefinitely while MQTT down — `failsafe_task` clears it at `main.c:165-168`).

#### Cross-story / CAP gaps

1. **STA/catalog boot window without bind ticks**  
   - **Source:** `main.c` — previously `wifi_sta_start()` used one 30 s `WaitBits` and catalog discover had no bind; LED stayed interim white until MQTT wait loops.  
   - **Lenses:** adversarial, edge-case (claim).  
   - **Instance:** fix now — **done** (human approved A1): STA wait polls 100 ms with `status_led_bind_tick`; bind before STA, before/after catalog.  
   - **Prevent next:** story AC should name *every* blocking bring-up wait, not only SoftAP early-return and MQTT loops.

2. **Live bind wiring untested by host suite**  
   - **Source:** `status_led_bind_tick` only in `main.c`; `test_status_led_ladder.c` builds inputs manually; `test_status_led.c` covers `phase_on`/init only.  
   - **Lenses:** verification-gap.  
   - **Instance:** defer (already in `deferred-work.md` SoftAP sequencing / main.c linkage gap).  
   - **Prevent next:** done_checkpoint stories that only wire `main.c` need an explicit HIL or extracted bind-builder test in the story template.

3. **SoftAP init/bind sequencing not CI-guarded**  
   - **Source:** `main.c:238-252`; deferred-work entry from story 1.  
   - **Lenses:** verification-gap, adversarial.  
   - **Instance:** defer (tracked).  
   - **Prevent next:** same as (2).

4. **OTA cancel → sticky `s_failed` → critical LED after fail-safe clears**  
   - **Source:** `failsafe_task` calls `ota_update_cancel()` at `main.c:150`; `ota_update_cancel` at `ota_update.c:850-854` does not clear `s_failed`; bind uses `ota_update_failed()` at `main.c:50`. SPEC binds live flag as-is; deferred-work story-3 entry.  
   - **Lenses:** adversarial, edge-case, verification-gap.  
   - **Instance:** accept as-is for this epic (SPEC). Track as separate OTA-policy work.  
   - **Prevent next:** forge/spec OTA clear policy when fail-safe aborts download.

5. **RGB update skipped while blink off-phase**  
   - **Source:** `status_led_set_rgb` applies only if `s_lit` (`status_led.c:152-154`); pattern changes force refresh (`:157-167`). Color-only change on blinking rung can lag until next on half-cycle.  
   - **Lenses:** adversarial, edge-case.  
   - **Instance:** defer.  
   - **Prevent next:** CAP wording “within one evaluation tick” should say *logical* rung vs *visible* hue for blink off-phase.

6. **No automated / HIL LED color assertions**  
   - **Source:** `firmware/tests/hil/` has no status-LED checklist rows; SPEC Success signal is device-visible.  
   - **Lenses:** verification-gap.  
   - **Instance:** defer (process + optional HIL doc).  
   - **Prevent next:** epic Success signal → checklist row required before `done`.

#### Accepted deviations (do not re-flag)

7. SoftAP 1 s vs operational 200 ms bind cadence — story 3 triage.  
8. Ladder green/purple when `mqtt_session_ready && !mqtt_connected` — encoded in `test_green_beats_yellow_when_mqtt_connected_false` / purple twin; bind clears session when MQTT down in `failsafe_task`.  
9. Blue OT-only without requiring STA/MQTT — matches companion rung 4 informal condition.  
10. Continue bind after `status_led_init` failure — logged at `main.c:240-242`; set_* no-op without strip (driver gates).

#### Dropped (no durable source / unrealistic)

- NULL `status_led_ladder_eval` — not reachable from bind.  
- Adversarial “timer critical section required” — no observed tear; not elevating without evidence.  
- Edge-case line refs to `main.c:618+` — file is 325 lines; those locations are invalid; retained only where re-verified at correct lines.

## Behavior verification

| Check | Result | Source |
|-------|--------|--------|
| Host tests `./run.sh` | **13/13 passed** including `test_status_led`, `test_status_led_ladder` | Re-run 2026-09-12 during Phase 2 |
| Device LED success-signal walk | **Not exercised this retro** | Terminal 6 flash/monitor has Wi‑Fi/OT traffic only; no LED color observation recorded |
| SoftAP / critical preempt on hardware | **Not exercised this retro** | — |

Passing host tests do not substitute for the SPEC Success signal on hardware.

## Previous-retro follow-through

No prior retrospective under `_bmad-output/` (no `RETROSPECTIVE.md`, no `epic-*-retro-*.md`, no `sprint-status.yaml` `action_items`). Nothing to follow through on — missing prior retro, not “zero open items.”

## Action items

Proposed only — not applied by this retrospective.

| ID | Action | Owner | From finding |
|----|--------|-------|--------------|
| A1 | Call `status_led_bind_tick()` during `wifi_sta_start` wait / between catalog steps (or make STA wait non-blocking with bind) so orange/yellow appear before MQTT loops | Denys — **approved; implemented** (poll STA wait + bind bracketing catalog) | Finding 1 |
| A2 | Keep deferred: SoftAP `status_led_init` sequencing + live bind untested by host — consider extracted bind-input builder test or HIL checklist row | Denys | Findings 2–3, 6 |
| A3 | Keep deferred: document ESP32-C6 GPIO8 strap constraint for status LED | Denys | deferred-work.md |
| A4 | Separate forge/spec: OTA `s_failed` clear when fail-safe cancels in-flight OTA (LED sticky critical) | Denys | Finding 4 |
| A5 | Optional: apply RGB on set even when `!s_lit`, or refresh after set_rgb | Denys | Finding 5 |

Process lessons (no session logs — from artifact trail only):

- Story 3 review correctly forced bind into MQTT wait loops; the same class of gap remained on `wifi_sta_start` — scan *all* blocking waits in `app_main` next time.
- Express forge→spec→build without `sprint-status.yaml` works; epic-level verification still needs an explicit HIL/success-signal gate.

## Acceptance verdict

**accepted** (human override)

- **Machine verdict was:** `accepted-with-open-items`.  
- **Human decision (Denys, 2026-09-12):** override to **accepted**.  
- **Criteria:** declared (SPEC CAP-1/CAP-2 + Success signal; story AC).  
- **Stories:** all `done`; `pending_stories` empty.  
- **Evidence for meet:** pure ladder + SoftAP/critical priority implemented and host-tested; live bind samples declared flags; SoftAP path creates `ota_confirm_tick_task` and binds; commits `9b627c3`/`cf4c6c3`/`40f3e20`; host 13/13.  
- **Follow-through:** A1 approved and implemented after retro (STA wait polls with bind; catalog steps bracketed). Remaining deferred items A2–A5 stay tracked outside this verdict.

## Open questions

_(Cleared — human overrode verdict to accepted and approved A1.)_

## Assumptions

_(Interactive run — omitted; epic and finished-story state confirmed with user.)_
