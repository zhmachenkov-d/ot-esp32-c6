# Bug Assessment: Cleartext credentials on open SoftAP portal

- **Slug**: softap-cleartext-creds
- **Created**: 2026-08-30
- **Source**: pasted text (Security Review, branch changes)
- **Verdict**: valid
- **Severity**: high

## Report (verbatim or summarized)

Security review Finding 1: SoftAP portal serves HTTP on port 80 over `WIFI_AUTH_OPEN`. An attacker within RF range who associates to `OTC6-XXXX` can passively capture the operator’s home Wi‑Fi password, MQTT username/password, and MQTT CA PEM from the `POST /save` body. Cited `firmware/main/provision_softap.c` (`save_post`, `start_httpd`) and `WIFI_AUTH_OPEN` in `provision_softap_start`.

## Symptom

During commissioning, secrets are submitted in cleartext on an open multi-client SoftAP. Expected (from a security standpoint): credentials are not passively sniffable by concurrent SoftAP clients—e.g. SoftAP WPA2-PSK and/or HTTPS on the portal.

## Reproduction

1. Device enters SoftAP provisioning (`OTC6-XXXX`, open auth, portal on port 80).
2. Attacker associates to the SoftAP (up to `max_connection = 4`).
3. Operator submits the captive portal form with Wi‑Fi/MQTT secrets.
4. Attacker sniffs HTTP `POST /save` and recovers credentials / CA PEM.

## Suspected Code Paths

- `firmware/main/provision_softap.c:113` — `save_post` reads `wifi_pass`, `mqtt_pass`, `mqtt_ca` from form body and persists via NVS
- `firmware/main/provision_softap.c:210-221` — `httpd` on default HTTP port 80; registers `POST /save`
- `firmware/main/provision_softap.c:261-262` — `max_connection = 4`, `authmode = WIFI_AUTH_OPEN`

## Root Cause Hypothesis

v1 SoftAP is intentionally open (`specs/001-wifi-mqtt-opentherm/contracts/softap-provisioning.md`: “Open … physical presence”). Combined with cleartext HTTP and multiple concurrent associations, “physical presence on the SSID” does not exclude a passive sniffer during the commissioning window. Confidence: **high**.

## Proposed Remediation

**Preferred**: WPA2-PSK SoftAP with a per-device random PSK (label / serial / QR), so only the operator who knows the PSK can join and see the portal traffic.

**Alternatives**:
- Serve the portal over HTTPS with a device-generated certificate (harder UX on captive portals).
- Document-only mitigation (provision in isolation) — reduces risk but does not fix the protocol exposure.

**Files likely to change**:
- `firmware/main/provision_softap.c`
- `specs/001-wifi-mqtt-opentherm/contracts/softap-provisioning.md` (security row)
- Portal UX / docs for how the operator obtains the SoftAP PSK

**Tests to add or update**:
- SoftAP config assertion: authmode is not open when PSK path is enabled (host or integration mock of wifi config).

## Risks & Considerations

- Contract currently **accepts open SoftAP for v1**; remediation is a deliberate product/security upgrade, not a silent bugfix.
- Captive-portal HTTPS and WPA2 both have phone UX friction; pick one primary control.
- Related: `softap-unauth-save` (anyone on SoftAP can POST `/save`).

## Open Questions

- [NEEDS CLARIFICATION: Is WPA2 SoftAP in scope for a v1.x fix, or deferred as accepted commissioning risk?]
