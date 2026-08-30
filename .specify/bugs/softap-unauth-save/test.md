# Bug Verification: SoftAP POST /save Setup PIN

- **Slug**: softap-unauth-save
- **Tested**: 2026-08-30
- **Assessment**: ./assessment.md
- **Fix**: ./fix.md
- **Result**: partial

## Summary

Host unit tests and static review confirm `POST /save` is gated by a one-time Setup PIN (match before validate, consume after success; missing/wrong → 403). The assessment’s on-device SoftAP association + unauthenticated `/save` reproduction was not run in this workspace, so end-to-end portal behavior remains unconfirmed.

## Checks Performed

| Check | Command / Action | Result | Notes |
|-------|------------------|--------|-------|
| Reproduction (post-fix) | SoftAP join → `POST /save` without PIN → with PIN → reuse PIN (assessment steps 1–3) | skipped | Needs device in SoftAP + RF/HTTP client; workspace has serial flash monitor only; device currently on STA |
| Static `save_post` audit | Read `provision_softap.c` form + handler + SoftAP start | pass | `setup_pin` required in HTML; `matches` → 403 before validate; `consume` after validate; PIN logged serial-only at start |
| Contract / quickstart | Read Setup PIN rows | pass | Contract requires session PIN + 403; quickstart commissions with serial PIN |
| New / updated tests | `./build/test_provision_validate` | pass | Both save-auth tests PASS |
| Regression suite | `firmware/tests/host/./run.sh` + `ctest` | pass | 9/9 host suites passed |
| Firmware build | `idf.py build` (in `firmware/`) | pass | `otc6_gateway.bin` built successfully |
| Lint / type-check | — | skipped | No separate host lint step for this path |

## Output Excerpts

```
test_save_auth_rejects_missing_or_invalid_token:PASS
test_save_auth_accepts_valid_token_once:PASS
16 Tests 0 Failures 0 Ignored
OK

100% tests passed, 0 tests failed out of 9
```

Static gate (post-fix): `provision_save_auth_matches` before form validate; `provision_save_auth_consume` only after `provision_validate` succeeds; SoftAP start logs `Setup PIN (required on Save, one-time)=…` and does not embed the token in `HTML_FORM`.

## Residual Risks

- On-device still needed: SoftAP start logs PIN; save without PIN → 403 and no NVS write; save with PIN succeeds once; second save with same PIN fails (or device already rebooting after first success).
- Clients who can read USB serial still learn the PIN — intentional for physical-presence commissioning; SoftAP PSK (`softap-cleartext-creds`) limits who can reach the portal.
- Fix is uncommitted in the working tree; flash currently monitored on `/dev/ttyACM0` may not yet include this binary until re-flashed after this build.

## Recommendation

Hold for a short on-device SoftAP save check (403 without PIN; one successful save with serial PIN), then close. Host evidence strongly supports the fix; do not reopen unless the device still accepts `/save` without a matching Setup PIN.
