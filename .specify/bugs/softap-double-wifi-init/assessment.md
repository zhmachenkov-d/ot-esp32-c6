# Bug Assessment: Double esp_wifi_init on SoftAP reprovision

- **Slug**: softap-double-wifi-init
- **Created**: 2026-08-30
- **Source**: pasted text (Bugbot review, branch changes)
- **Verdict**: valid
- **Severity**: high

## Report (verbatim or summarized)

Bugbot: `provision_softap_start` always calls `esp_wifi_init`, but `main` may already have initialized Wi‑Fi in STA mode (e.g. after `wifi_sta_start` when TLS CA is missing), causing `ESP_ERROR_CHECK` to abort instead of opening the captive portal. Cited `firmware/main/provision_softap.c:253-255`.

## Symptom

When credentials exist but MQTT TLS CA is missing (or another path calls SoftAP after STA init), the device aborts on `esp_wifi_init` instead of presenting the provisioning portal. Expected: SoftAP portal starts cleanly even if STA Wi‑Fi was already initialized.

## Reproduction

1. Store Wi‑Fi + MQTT config with `mqtt_tls` enabled but no CA PEM in NVS.
2. Boot: `wifi_sta_start()` runs (`main.c:206` → `esp_wifi_init` at `main.c:96`).
3. CA load fails → `provision_softap_start` (`main.c:216`) → second `esp_wifi_init` (`provision_softap.c:255`) → `ESP_ERROR_CHECK` abort.

## Suspected Code Paths

- `firmware/main/main.c:91-107` — `wifi_sta_start` always `esp_wifi_init`
- `firmware/main/main.c:210-217` — TLS CA missing → SoftAP after STA init
- `firmware/main/provision_softap.c:247-264` — unconditional `esp_wifi_init` + AP mode

## Root Cause Hypothesis

SoftAP start assumes a virgin Wi‑Fi stack. Reprovision/fallback paths after STA bring-up violate that. Confidence: **high**.

## Proposed Remediation

**Preferred**: Make `provision_softap_start` idempotent w.r.t. Wi‑Fi init: if already initialized, skip `esp_wifi_init`, stop STA as needed, switch to AP (or APSTA), then configure SoftAP. Treat `ESP_ERR_WIFI_INIT_STATE` / already-init as non-fatal.

**Alternatives**:
- Tear down STA (`esp_wifi_stop` / `esp_wifi_deinit`) before SoftAP — heavier but clear.
- Check CA before `wifi_sta_start` so SoftAP is the first Wi‑Fi user on that path — fixes one call site only; button-triggered reprovision may still hit the race.

**Files likely to change**:
- `firmware/main/provision_softap.c`
- Possibly `firmware/main/main.c` (order of CA check vs STA)
- Any provision-button path that starts SoftAP while STA is up

**Tests to add or update**:
- Integration or host-level state-machine test: SoftAP start after mocked STA init must not call init twice / must return `ESP_OK`.

## Risks & Considerations

- Switching mode while connected drops STA; ensure callers expect that on reprovision.
- `ESP_ERROR_CHECK` elsewhere may still abort on related Wi‑Fi errors — prefer explicit handling.

## Open Questions

- [NEEDS CLARIFICATION: Should button-hold reprovision also stop MQTT/STA gracefully before SoftAP, or is Wi‑Fi-only handling enough for v1?]
