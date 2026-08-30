# Bug Fix: SoftAP Wi‑Fi bring-up after STA init

- **Slug**: softap-double-wifi-init
- **Fixed**: 2026-08-30
- **Assessment**: ./assessment.md
- **Status**: applied

## Summary

`provision_softap_start` is now idempotent with respect to Wi‑Fi: if the stack is already initialized (e.g. after STA), it skips a second `esp_wifi_init`, stops the driver, then switches to AP mode with explicit error returns instead of `ESP_ERROR_CHECK` aborts.

## Changes

| File | Change | Notes |
|------|--------|-------|
| `firmware/main/provision_softap.h` | modified | Added `provision_softap_plan_wifi` / `provision_softap_wifi_plan_t` |
| `firmware/main/provision_validate.c` | modified | Host-testable SoftAP Wi‑Fi bring-up planner |
| `firmware/main/provision_softap.c` | modified | Skip init when already up; stop before AP; no abort on Wi‑Fi errors |
| `firmware/tests/host/test_provision_validate.c` | modified | Pins post-STA plan: no second init + stop before AP |

## Diff Highlights (optional)

```c
provision_softap_plan_wifi(wifi_already_init, wifi_started, &plan);
if (plan.call_wifi_init) {
    err = esp_wifi_init(&cfg);
    if (err != ESP_OK && err != ESP_ERR_WIFI_INIT_STATE) return err;
}
if (plan.call_wifi_stop) {
    esp_wifi_stop(); /* NOT_STARTED is fine */
}
esp_wifi_set_mode(WIFI_MODE_AP);
```

## Tests Added or Updated

- `test_provision_validate.c::test_softap_wifi_plan_after_sta_skips_second_init` — after STA: `call_wifi_init=false`, `call_wifi_stop=true`
- `test_provision_validate.c::test_softap_wifi_plan_virgin_calls_init` — first SoftAP: init yes, stop no

## Local Verification

- Commands run: `firmware/tests/host/./run.sh` → 9/9 passed
- Commands run: `firmware/ idf.py build` → success
- Manual checks: none (on-device SoftAP-after-STA not exercised here)

## Deviations from Assessment

- Extracted `provision_softap_plan_wifi` for host-testable state-machine coverage (full `esp_wifi` SoftAP path is not host-linked).
- Current `main.c` no longer auto-opens SoftAP on missing TLS CA (`softap-ca-missing-reprovision`); button path still clears + restarts. SoftAP itself is still hardened for any future/in-process call after STA.
- Open question (MQTT/STA graceful teardown before SoftAP): Wi‑Fi-only handling for v1, as preferred remediation.

## Follow-ups

- Run `/speckit-bug-test slug=softap-double-wifi-init`.
- If SoftAP is ever started in-process while STA handlers remain registered, unregister or gate `esp_wifi_connect` on `WIFI_EVENT_STA_DISCONNECTED` to avoid reconnect fighting AP mode.
- Optional: stop MQTT before SoftAP when button-triggered reprovision is changed to avoid reboot.
