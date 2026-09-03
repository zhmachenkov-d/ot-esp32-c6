# Bug Fix: Missing MQTT CA no longer opens SoftAP

- **Slug**: softap-ca-missing-reprovision
- **Fixed**: 2026-08-30
- **Assessment**: ./assessment.md
- **Status**: applied

## Summary

When Wi‑Fi/MQTT credentials exist but TLS CA is missing, boot now keeps STA up, skips MQTT start, and logs that recovery is via GPIO9 long-press — SoftAP is no longer auto-opened on that path.

## Changes

| File | Change | Notes |
|------|--------|-------|
| `firmware/main/main.c` | modified | CA-missing branch no longer calls SoftAP; MQTT init skipped |
| `firmware/main/provision_softap.h` | modified | Added `provision_boot_action` / `PROVISION_BOOT_*` |
| `firmware/main/provision_validate.c` | modified | Host-testable boot SoftAP/MQTT policy |
| `firmware/tests/host/test_provision_validate.c` | modified | Pins missing-CA ≠ SoftAP; button-clear → SoftAP |
| `specs/001-wifi-mqtt-opentherm/contracts/softap-provisioning.md` | modified | Clarifies missing CA must not open SoftAP |

## Diff Highlights (optional)

```c
/* Before: SoftAP + return */
/* After: */
if (boot == PROVISION_BOOT_RUN_NO_MQTT) {
    ESP_LOGE(TAG, "... MQTT disabled; long-press GPIO9 to re-provision");
} else {
    mqtt_ha_init(...); mqtt_ha_start();
}
```

## Tests Added or Updated

- `test_provision_validate.c::test_boot_tls_missing_ca_does_not_open_softap` — credentials + TLS + no CA → `PROVISION_BOOT_RUN_NO_MQTT`
- `test_provision_validate.c::test_boot_after_button_clears_credentials_opens_softap` — cleared credentials → SoftAP
- `test_provision_validate.c::test_boot_no_credentials_opens_softap` / `test_boot_tls_with_ca_runs_mqtt` — first-boot and healthy TLS paths

## Local Verification

- Commands run: `firmware/tests/host/./run.sh` → 9/9 passed
- Manual checks: none (on-device SoftAP window not exercised here)

## Deviations from Assessment

- Extracted `provision_boot_action` so the SoftAP policy is host-testable (assessment asked for a boot-path/state-machine test; `main.c` alone is not host-linked).
- No LED helper changes — serial `ESP_LOGE` is the signal for v1.
- Contract line added for the open question: once credentials exist, long-press is the sole SoftAP entry for config-integrity (missing CA) failures.

## Follow-ups

- Run `/speckit-bug-test slug=softap-ca-missing-reprovision`.
- On-device: TLS without CA should stay on STA, log the CA-missing message, and only open SoftAP after GPIO9 ≥5 s.
- `softap-double-wifi-init` is less likely on this path now; button reprovision still uses clear-credentials + restart (separate SoftAP entry).
