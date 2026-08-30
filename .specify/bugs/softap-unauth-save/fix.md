# Bug Fix: SoftAP portal Setup PIN on POST /save

- **Slug**: softap-unauth-save
- **Fixed**: 2026-08-30
- **Assessment**: ./assessment.md
- **Status**: applied

## Summary

`POST /save` now requires a one-time Setup PIN generated at SoftAP start and logged on USB serial (not embedded in portal HTML). Missing or wrong PIN returns 403; a successful validated save consumes the PIN so another SoftAP client cannot reuse it.

## Changes

| File | Change | Notes |
|------|--------|-------|
| `firmware/main/provision_softap.h` | modified | `provision_save_auth_t` + set/matches/consume API |
| `firmware/main/provision_validate.c` | modified | Host-testable one-time token check |
| `firmware/main/provision_softap.c` | modified | Generate/log PIN; form field; gate `save_post` |
| `firmware/tests/host/test_provision_validate.c` | modified | Reject invalid; accept once |
| `specs/001-wifi-mqtt-opentherm/contracts/softap-provisioning.md` | modified | Setup PIN field + security notes |
| `specs/001-wifi-mqtt-opentherm/quickstart.md` | modified | Commissioning uses serial Setup PIN |

## Diff Highlights (optional)

```c
/* SoftAP start: random 8-hex PIN, serial only */
ESP_LOGI(TAG, "Setup PIN (required on Save, one-time)=%s", save_token);

/* save_post: match before validate; consume only after form OK */
if (!provision_save_auth_matches(&s_save_auth, setup_pin)) { /* 403 */ }
/* ... provision_validate ... */
if (!provision_save_auth_consume(&s_save_auth, setup_pin)) { /* 403 */ }
```

## Tests Added or Updated

- `test_provision_validate.c::test_save_auth_rejects_missing_or_invalid_token` — NULL/empty/wrong PIN rejected; not consumed
- `test_provision_validate.c::test_save_auth_accepts_valid_token_once` — valid PIN accepted once; second use fails

## Local Verification

- Commands run: `firmware/tests/host/./run.sh` + `ctest` → 9/9 passed
- Commands run: `firmware/ idf.py build` → success
- Manual checks: none (on-device SoftAP save with PIN not exercised here)

## Deviations from Assessment

- Chose portal Setup PIN (preferred remediation) in combination with already-applied WPA2 SoftAP (`softap-cleartext-creds`); did not implement GPIO9 unlock-window alternative.
- Extracted host-testable `provision_save_auth_*` rather than linking full `save_post` / `esp_http_server` on host.
- PIN is consumed only after form validation succeeds so a bad submit does not burn the one-time token.

## Follow-ups

- Run `/speckit-bug-test slug=softap-unauth-save`.
- On-device: SoftAP start → note Setup PIN from serial → save without PIN fails → save with PIN succeeds → second save with same PIN fails (or device already rebooting).
- Optional later: rate-limit `/save` or button-unlock window for extra hardening among SoftAP clients who also have serial access.
