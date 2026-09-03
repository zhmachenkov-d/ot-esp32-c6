# Bug Fix: Concurrent OpenTherm bus access

- **Slug**: concurrent-ot-bus-access
- **Fixed**: 2026-08-30
- **Assessment**: ./assessment.md
- **Status**: applied

## Summary

Added an internal FreeRTOS mutex around `ot_poll_exchange` so discovery, the poll task, and any other caller serialize on the OpenTherm master. A host concurrency test links real `ot_poll.c` and fails if `esp_ot_send_request` overlaps.

## Changes

| File | Change | Notes |
|------|--------|-------|
| `firmware/main/ot_poll.c` | modified | Create `s_bus_mtx` in init; take/give around exchange + gap |
| `firmware/main/ot_poll.h` | modified | Document bus serialization on `ot_poll_exchange` |
| `firmware/tests/host/CMakeLists.txt` | modified | Add `test_ot_poll_bus_lock` linking real `ot_poll.c` |
| `firmware/tests/host/test_ot_poll_bus_lock.c` | added | Two threads hammer exchange; assert no OT overlap |
| `firmware/tests/host/stubs/freertos/*.h` | added | Minimal FreeRTOS types for host |
| `firmware/tests/host/stubs/freertos_host_stub.c` | added | pthread-backed mutex/queue/task stubs |
| `firmware/tests/host/stubs/opentherm.h` | added | Minimal OT API for host |
| `firmware/tests/host/stubs/opentherm_host_stub.c` | added | Tracks in-flight `esp_ot_send_request` |

## Diff Highlights (optional)

```c
s_bus_mtx = xSemaphoreCreateMutex();
...
if (xSemaphoreTake(s_bus_mtx, portMAX_DELAY) != pdTRUE) {
    return OT_EXCHANGE_ERROR;
}
/* build + send + map + inter-frame gap */
xSemaphoreGive(s_bus_mtx);
```

## Tests Added or Updated

- `firmware/tests/host/test_ot_poll_bus_lock.c::test_parallel_exchange_serializes_on_bus` — two pthreads call `ot_poll_exchange`; asserts `host_ot_overlap_count() == 0` and `max_in_flight == 1`

## Local Verification

- Commands run: `firmware/tests/host/./run.sh` → 9/9 passed (including `test_ot_poll_bus_lock`)
- Manual checks: none (host concurrency coverage for the race)

## Deviations from Assessment

- Under `HOST_TEST`, `gap_ms` is a no-op so the bus-lock test stays fast; device builds still delay `APP_OT_INTER_FRAME_GAP_MS` inside the mutex.
- Did not change discovery ordering in `main.c` or suppress keepalive during discovery — mutex-only per preferred remediation.

## Follow-ups

- Run `/speckit-bug-test slug=concurrent-ot-bus-access` for formal verification recording.
- On-device: boot with credentials so poll task and `ot_catalog_discover` overlap; confirm stable exchanges / no garbled frames.
- Open question left open: whether discovery should also pause keepalive (mutex-only is sufficient for frame integrity).
