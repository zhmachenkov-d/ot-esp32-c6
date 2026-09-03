# Bug Verification: SoftAP WPA2-PSK (no open cleartext portal)

- **Slug**: softap-cleartext-creds
- **Tested**: 2026-08-30
- **Assessment**: ./assessment.md
- **Fix**: ./fix.md
- **Result**: partial

## Summary

Host tests and static review confirm SoftAP AP params require WPA2-PSK (no open-auth fallback) and firmware builds with `WIFI_AUTH_WPA2_PSK`. The assessment’s on-device RF sniff / open-association reproduction was not run in this workspace, so end-to-end commissioning UX remains unconfirmed.

## Checks Performed

| Check | Command / Action | Result | Notes |
|-------|------------------|--------|-------|
| Reproduction (post-fix) | SoftAP open join + sniff `POST /save` (assessment steps 1–4) | skipped | Needs device + RF client; not available here |
| Static SoftAP auth audit | Grep `WIFI_AUTH_*` / softap start path in `provision_softap.c` | pass | `authmode = WIFI_AUTH_WPA2_PSK`; refuses non-WPA2 `ap_params`; no `WIFI_AUTH_OPEN` in SoftAP start |
| Contract / quickstart | Read security row + commissioning step | pass | Contract Security = WPA2-PSK; quickstart requires serial PSK |
| New / updated tests | `./build/test_provision_validate` | pass | Both SoftAP AP-param tests PASS |
| Regression suite | `firmware/tests/host/./run.sh` | pass | 9/9 tests passed |
| Firmware build | `idf.py build` (in `firmware/`) | pass | `otc6_gateway.bin` built successfully |
| Lint / type-check | — | skipped | No separate host lint step for this path |

## Output Excerpts

```
test_softap_ap_params_wpa2_when_psk_present:PASS
test_softap_ap_params_reject_short_or_missing_psk:PASS
12 Tests 0 Failures 0 Ignored
OK

100% tests passed, 0 tests failed out of 9
```

SoftAP start (post-fix): `ap.ap.authmode = WIFI_AUTH_WPA2_PSK` after `nvs_store_ensure_softap_psk` + `provision_softap_build_ap_params`; open authmode returns `ESP_ERR_INVALID_STATE`.

## Residual Risks

- On-device still needed: join `OTC6-XXXX` with serial PSK works; association without PSK fails; portal `POST /save` still commissions.
- Portal remains HTTP (cleartext) for clients who know the PSK — intentional per preferred remediation; related residual `softap-unauth-save`.
- Factory label/QR for SoftAP PSK not implemented; serial log is the commissioning path today.

## Recommendation

Hold for a short on-device SoftAP join check (PSK required, open associate fails), then close. Host evidence strongly supports the fix; do not reopen unless the device still advertises an open SoftAP or accepts association without the NVS PSK.
