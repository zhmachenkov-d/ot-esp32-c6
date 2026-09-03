# Bug Verification: Concurrent OpenTherm bus access

- **Slug**: concurrent-ot-bus-access
- **Tested**: 2026-08-30
- **Assessment**: ./assessment.md
- **Fix**: ./fix.md
- **Result**: partial

## Summary

Host concurrency coverage shows the race is fixed: two threads calling `ot_poll_exchange` never overlap in `esp_ot_send_request`, and the bus mutex is present in `ot_poll.c`. On-device cold-start (poll task + `ot_catalog_discover` on `app_main`) was not run, so full end-to-end verification is incomplete.

## Checks Performed

| Check | Command / Action | Result | Notes |
|-------|------------------|--------|-------|
| Reproduction (post-fix) | Assessment boot path: credentials → `ot_poll_start` then `ot_catalog_discover` | skipped | Needs ESP32-C6 + Wi‑Fi/MQTT + boiler timing |
| Automated race equivalent | `./build/test_ot_poll_bus_lock` | pass | Two pthreads; `overlap_count == 0`, `max_in_flight == 1` |
| Mutex present | Inspect `ot_poll.c` (`s_bus_mtx` create/take/give) | pass | Lock wraps exchange + inter-frame gap |
| New / updated tests | `./build/test_ot_poll_bus_lock` | pass | `test_parallel_exchange_serializes_on_bus` |
| Regression suite | `firmware/tests/host/./run.sh` | pass | 9/9 host tests |
| Lint / type-check | — | skipped | No project lint gate for these C sources |
| On-device overlap smoke | Flash + boot with credentials during discovery | skipped | Hardware / consent not available |

## Output Excerpts

```
test_parallel_exchange_serializes_on_bus:PASS
1 Tests 0 Failures 0 Ignored
OK
```

```
100% tests passed, 0 tests failed out of 9
```

## Residual Risks

- Device FreeRTOS scheduling + real OpenTherm RTT were not exercised; host uses pthread mutex + stubbed `esp_ot_send_request`.
- Keepalive still runs during discovery (mutex-only); frames serialize but discovery latency may grow under contention.
- MQTT write path delay while the lock is held for one exchange was not measured on target.

## Recommendation

Hold for on-device smoke — host race coverage and code inspection look good, but the assessment’s cold-start reproduction was not exercised. After a boot with credentials where poll and discovery overlap without garbled frames, re-run this command (or treat as close).
