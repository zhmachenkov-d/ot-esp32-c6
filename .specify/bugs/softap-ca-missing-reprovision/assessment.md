# Bug Assessment: Missing MQTT CA reopens SoftAP

- **Slug**: softap-ca-missing-reprovision
- **Created**: 2026-08-30
- **Source**: pasted text (Security Review, branch changes)
- **Verdict**: valid
- **Severity**: medium

## Report (verbatim or summarized)

Security review Finding 3: A device that already has Wi‑Fi credentials but lacks a stored CA while `mqtt_tls` is enabled re-enters open SoftAP + unauthenticated HTTP provisioning. That extends the SoftAP attack surface beyond first-boot commissioning and conflicts with the contract that SoftAP on an operational device is long-press only (sustained link failure alone must not open SoftAP). Cited `firmware/main/main.c` (TLS CA missing → `provision_softap_start`).

## Symptom

Misconfigured or corrupt MQTT CA with TLS enabled causes automatic SoftAP on boot even though Wi‑Fi/MQTT settings otherwise exist. Expected: stay on STA (or fail MQTT safely) and require GPIO9 long-press for reprovision; do not open SoftAP solely because CA is missing.

## Reproduction

1. Store Wi‑Fi + MQTT config with `mqtt_tls` enabled but no (or empty/corrupt) CA PEM in NVS.
2. Boot: STA may start (`wifi_sta_start`), then CA load fails.
3. Firmware logs “MQTT TLS enabled but CA PEM missing — re-provision” and calls `provision_softap_start`.
4. Nearby attacker can exploit SoftAP cleartext / unauthenticated `/save` during that window.

## Suspected Code Paths

- `firmware/main/main.c:210-217` — `mqtt_tls` + failed/empty `nvs_store_mqtt_ca_load` → SoftAP + return
- `firmware/main/provision_softap.c` — opens open SoftAP + HTTP portal
- `specs/001-wifi-mqtt-opentherm/contracts/softap-provisioning.md` — Operational: long-press only; “Sustained Wi‑Fi/MQTT failure alone MUST NOT open SoftAP in v1”

## Root Cause Hypothesis

Missing CA is treated as a full reprovision trigger rather than an MQTT/TLS configuration error with a constrained recovery path. That reuses the first-boot SoftAP surface on an otherwise-configured device. Confidence: **high**. Related crash path on the same trigger: `softap-double-wifi-init`.

## Proposed Remediation

**Preferred**: Do not auto-open SoftAP on missing CA. Keep STA up, log/signal clearly (LED/serial), leave MQTT disconnected until CA is fixed via **button-forced** reprovision only.

**Alternatives**:
- Require GPIO9 long-press confirmation before SoftAP on the CA-missing path.
- Soft-fail TLS (refuse MQTT connect) without SoftAP; allow operator to fix CA via a future authenticated path.

**Files likely to change**:
- `firmware/main/main.c` (CA-missing branch)
- Possibly LED/status signaling helpers
- Contract clarification if “config integrity failure” is ever allowed to open SoftAP

**Tests to add or update**:
- Boot-path / host state-machine test: credentials present + `mqtt_tls` + missing CA → SoftAP **not** started; button path still can start SoftAP.

## Risks & Considerations

- Operators who enable TLS without saving CA currently get SoftAP as recovery UX; removing it requires a clear alternate recovery (button + docs).
- Closely related to `softap-double-wifi-init` (same call site after STA init); fix ordering carefully.
- SoftAP surface while open still inherits `softap-cleartext-creds` / `softap-unauth-save`.

## Open Questions

- [NEEDS CLARIFICATION: Should any config-integrity failure ever auto-open SoftAP in v1, or is long-press the sole SoftAP entry once Wi‑Fi credentials exist?]
