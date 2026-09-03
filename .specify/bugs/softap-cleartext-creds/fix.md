# Bug Fix: SoftAP WPA2-PSK instead of open cleartext portal

- **Slug**: softap-cleartext-creds
- **Fixed**: 2026-08-30
- **Assessment**: ./assessment.md
- **Status**: applied

## Summary

SoftAP provisioning now uses WPA2-PSK with a per-device random 16-hex PSK persisted in NVS (`softap_psk`), so RF neighbors without the PSK cannot join and sniff HTTP `POST /save` credentials. The PSK is logged at SoftAP start for serial commissioning and survives credential clear.

## Changes

| File | Change | Notes |
|------|--------|-------|
| `firmware/main/nvs_store.h` | modified | `NVS_SOFTAP_PSK_MAX`, `nvs_store_ensure_softap_psk` |
| `firmware/main/nvs_store.c` | modified | Generate/persist SoftAP PSK; keep across clear_credentials |
| `firmware/main/provision_softap.h` | modified | `provision_softap_build_ap_params` + auth enums |
| `firmware/main/provision_validate.c` | modified | Host-testable AP SSID/WPA2 param builder |
| `firmware/main/provision_softap.c` | modified | SoftAP starts as `WIFI_AUTH_WPA2_PSK`; refuse open |
| `firmware/tests/host/test_provision_validate.c` | modified | Assert WPA2 when PSK present; reject short PSK |
| `specs/001-wifi-mqtt-opentherm/contracts/softap-provisioning.md` | modified | Security row + notes → WPA2-PSK |
| `specs/001-wifi-mqtt-opentherm/quickstart.md` | modified | Commissioning uses serial SoftAP password |

## Diff Highlights (optional)

```c
ap.ap.authmode = WIFI_AUTH_WPA2_PSK;
strncpy((char *)ap.ap.password, ap_params.password, ...);
/* was: WIFI_AUTH_OPEN */
```

## Tests Added or Updated

- `test_provision_validate.c::test_softap_ap_params_wpa2_when_psk_present` — authmode is WPA2-PSK, not open
- `test_provision_validate.c::test_softap_ap_params_reject_short_or_missing_psk` — PSK path cannot fall back to open

## Local Verification

- Commands run: `firmware/tests/host/./run.sh` → 9/9 passed
- Commands run: `firmware/ idf.py build` → success
- Manual checks: none (on-device SoftAP join not exercised here)

## Deviations from Assessment

- Extracted `provision_softap_build_ap_params` for host-testable authmode assertion (assessment asked for SoftAP config assertion; full `esp_wifi` path is not host-linked).
- SoftAP PSK is obtained from USB serial at SoftAP start (and NVS for stability); factory label/QR printing is not implemented in firmware — documented as the operator path in quickstart/contract.
- Open question “WPA2 in v1.x?” treated as yes by running this fix command with the preferred remediation.

## Follow-ups

- Run `/speckit-bug-test slug=softap-cleartext-creds`.
- On-device: join `OTC6-XXXX` with serial PSK; confirm open association fails; confirm portal save still works.
- Optional: factory label/QR tooling that mirrors NVS `softap_psk` without requiring serial.
- Related residual: `softap-unauth-save` (anyone who knows the PSK can still POST `/save`).
- `research.md` still describes historical “open SoftAP” decision; contract/quickstart are the live commissioning source of truth.
