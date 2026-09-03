# Bug Verification: Missing MQTT CA no longer opens SoftAP

- **Slug**: softap-ca-missing-reprovision
- **Tested**: 2026-08-30
- **Assessment**: ./assessment.md
- **Fix**: ./fix.md
- **Result**: partial

## Summary

Host policy tests and a static call-site audit confirm credentials + TLS + missing CA maps to `PROVISION_BOOT_RUN_NO_MQTT` and `main.c` no longer calls SoftAP on that path. The assessment’s on-device SoftAP-window reproduction was not run, so end-to-end STA-only boot remains unconfirmed in this environment.

## Checks Performed

| Check | Command / Action | Result | Notes |
|-------|------------------|--------|-------|
| Reproduction (post-fix) | Boot with Wi‑Fi/MQTT + `mqtt_tls` and empty/missing CA; observe SoftAP SSID / `/save` | skipped | Requires device + NVS prep; not available in this host workspace |
| Static call-site audit | Grep `provision_softap_start` / CA-missing branch in `main.c` | pass | SoftAP only when credentials missing; CA-missing → log + skip MQTT |
| New / updated tests | `./build/test_provision_validate` | pass | All four boot-policy cases PASS |
| Regression suite | `firmware/tests/host/./run.sh` | pass | 9/9 tests passed |
| Contract line | Read `softap-provisioning.md` | pass | Explicit “MUST NOT open SoftAP; recover via long-press only” |
| Lint / type-check | — | skipped | No separate host lint step for this path |

## Output Excerpts

```
test_boot_no_credentials_opens_softap:PASS
test_boot_tls_missing_ca_does_not_open_softap:PASS
test_boot_tls_with_ca_runs_mqtt:PASS
test_boot_after_button_clears_credentials_opens_softap:PASS
10 Tests 0 Failures 0 Ignored
OK

100% tests passed, 0 tests failed out of 9
```

Call sites (post-fix):

- `main.c` credentials missing → `provision_softap_start`
- `main.c` TLS + `!ca_pem_ok` → `PROVISION_BOOT_RUN_NO_MQTT` (MQTT skipped; SoftAP not started)
- SoftAP after credentials exist → GPIO9 long-press path only (clear credentials + restart)

## Residual Risks

- On-device confirmation still needed: TLS without CA should stay on STA, log the CA-missing message, and only open SoftAP after GPIO9 ≥5 s.
- Button reprovision SoftAP entry is unchanged; related `softap-double-wifi-init` is a separate bug if that path still double-inits Wi‑Fi.

## Recommendation

Hold for a short device check (TLS enabled, CA wiped/corrupt), then close. Host evidence strongly supports the fix; do not reopen unless SoftAP still appears on that boot without a long-press.
