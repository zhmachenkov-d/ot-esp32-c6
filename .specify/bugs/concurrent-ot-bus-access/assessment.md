# Bug Assessment: Concurrent OpenTherm bus access

- **Slug**: concurrent-ot-bus-access
- **Created**: 2026-08-30
- **Source**: pasted text (Bugbot review, branch changes)
- **Verdict**: valid
- **Severity**: high

## Report (verbatim or summarized)

Bugbot: `ot_poll_start` runs the keepalive/write task while `app_main` calls `ot_catalog_discover`, which invokes `ot_poll_exchange` directly with no bus lock—two tasks can drive `esp_ot_send_request` at once. Cited `firmware/main/main.c:198-234`.

## Symptom

During cold start (and re-validate), catalog discovery exchanges can interleave with the poll task’s keepalive/write exchanges on the same OpenTherm master interface. Expected: a single owner of the bus at a time so frames are not corrupted or mis-attributed.

## Reproduction

1. Boot with Wi‑Fi/MQTT credentials present so `app_main` reaches catalog load/discover after `ot_poll_start()`.
2. Note `ot_poll_start()` at `main.c:198` then `ot_catalog_discover()` at `main.c:231`/`234` on the `app_main` task.
3. Confirm `ot_catalog_discover` → `ot_poll_exchange` → `esp_ot_send_request` with no mutex; poll task also calls `ot_poll_exchange` from its loop.

## Suspected Code Paths

- `firmware/main/main.c:195-235` — starts poll task, then discovers on `app_main`
- `firmware/main/ot_catalog.c:253-288` — discovery loop uses `ot_poll_exchange`
- `firmware/main/ot_poll.c:106+` — `ot_poll_exchange` / poll task loop with no bus lock

## Root Cause Hypothesis

`ot_poll_exchange` is a shared, unsynchronized entry point used by both the FreeRTOS poll task and `app_main` during discovery. Confidence: **high** for the race; **medium** for field symptoms (depends on timing and boiler timing tolerance).

## Proposed Remediation

**Preferred**: Add a bus mutex (or equivalent) inside `ot_poll_exchange` so all callers serialize. Optionally pause the poll task (or skip its loop) while discovery runs, but locking at the exchange boundary covers MQTT write commands too.

**Alternatives**:
- Run discovery before `ot_poll_start`, then start the poll task — simpler for boot path only; leaves later concurrent callers unprotected.
- Queue discovery work onto the poll task — cleaner ownership, more refactor.

**Files likely to change**:
- `firmware/main/ot_poll.c` / `ot_poll.h`
- Possibly `firmware/main/main.c` if discovery ordering changes
- Host stubs/tests if exchange locking needs mocking

**Tests to add or update**:
- Host test or documented concurrency invariant: nested/parallel `ot_poll_exchange` calls serialize (mock lock or single-threaded assertion of acquisition order).

## Risks & Considerations

- Lock held across OpenTherm RTT (tens of ms) can delay MQTT command path; keep critical section to one exchange.
- Deadlock if a caller holds the lock and waits on another path that also exchanges — avoid nested waits.

## Open Questions

- [NEEDS CLARIFICATION: Should discovery also suppress keepalive/status writes for the duration, or is mutex-only sufficient?]
