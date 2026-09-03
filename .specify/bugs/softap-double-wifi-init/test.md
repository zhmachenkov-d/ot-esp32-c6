# Bug Verification: SoftAP Wi‑Fi bring-up after STA init

- **Slug**: softap-double-wifi-init
- **Tested**: 2026-08-30
- **Assessment**: ./assessment.md
- **Fix**: ./fix.md
- **Result**: partial

## Summary

Host tests pin the SoftAP Wi‑Fi planner (skip second `esp_wifi_init` after STA; stop before AP), and firmware builds cleanly with the fix in place. On-device reproduction of SoftAP-after-STA was not run, so the fix is not end-to-end verified on hardware.

## Checks Performed

| Check | Command / Action | Result | Notes |
|-------|------------------|--------|-------|
| Reproduction (post-fix) | Assessment steps: STA init then SoftAP (missing TLS CA / in-process SoftAP) | skipped | Needs device + NVS setup; destructive. Also, current `main.c` no longer auto-opens SoftAP on missing CA (`softap-ca-missing-reprovision`), so the exact boot path from the assessment is gone; SoftAP-after-STA would need an in-process or button path. |
| New / updated tests | `firmware/tests/host/./run.sh` + `./build/test_provision_validate` | pass | Both planner tests PASS: `test_softap_wifi_plan_after_sta_skips_second_init`, `test_softap_wifi_plan_virgin_calls_init` |
| Regression suite | `firmware/tests/host/./run.sh` (full host ctest) | pass | 9/9 ctest targets; 14/14 cases in `test_provision_validate` |
| Firmware build | `idf.py build` (in `firmware/`) | pass | Linked `provision_softap.c` / `provision_validate.c`; image built |
| Code spot-check | Confirm SoftAP path skips init + no `ESP_ERROR_CHECK` | pass | `provision_softap_plan_wifi` gates init/stop; no `ESP_ERROR_CHECK` left in `provision_softap.c` |
| Lint / type-check | n/a | skipped | No project lint gate for this C firmware path |

## Output Excerpts

```
test_softap_wifi_plan_after_sta_skips_second_init:PASS
test_softap_wifi_plan_virgin_calls_init:PASS
-----------------------
14 Tests 0 Failures 0 Ignored
OK
```

```
100% tests passed, 0 tests failed out of 9
```

```
Project build complete.
```

## Residual Risks

- On-device SoftAP after an already-started STA stack was not exercised; STA event handlers reconnecting while AP is up remains a follow-up from `fix.md`.
- Assessment boot repro (missing CA → SoftAP after `wifi_sta_start`) is obsolete after `softap-ca-missing-reprovision`; regression coverage for double-init is planner/host + SoftAP code path only.
- Full `esp_wifi_*` SoftAP path is not host-linked; only the bring-up plan is unit-tested.

## Recommendation

Hold for close until SoftAP is started once on-device after STA Wi‑Fi is already up (e.g. temporary debug call or future in-process reprovision without reboot). Host planner tests and firmware build support the fix; do not reopen unless device logs show a second `esp_wifi_init` abort.
