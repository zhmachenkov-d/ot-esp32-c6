# Bug Assessment: Unauthenticated SoftAP POST /save takeover

- **Slug**: softap-unauth-save
- **Created**: 2026-08-30
- **Source**: pasted text (Security Review, branch changes)
- **Verdict**: valid
- **Severity**: medium

## Report (verbatim or summarized)

Security review Finding 2: Any client on the SoftAP can `POST /save` with no session token, PIN, or proof-of-ownership and persist arbitrary Wi‑Fi/MQTT/CA settings, redirecting the gateway to an attacker-controlled broker or network. Cited URI registration in `start_httpd` (`GET /`, `POST /save` only).

## Symptom

Joining the open SoftAP is enough to reconfigure the device. Expected (security): `/save` requires a one-time token, PIN, or physical confirmation so a concurrent SoftAP client cannot silently hijack provisioning.

## Reproduction

1. Device broadcasts open SoftAP `OTC6-XXXX` (suffix predictable from MAC / device_id last 4 hex digits).
2. Attacker associates and `POST`s `/save` with attacker `mqtt_host`, credentials, optional CA.
3. Device saves to NVS and reboots into attacker-chosen configuration.

## Suspected Code Paths

- `firmware/main/provision_softap.c:113` — `save_post` has no auth check before `nvs_store_save`
- `firmware/main/provision_softap.c:217-219` — only `GET /` and `POST /save` registered
- `firmware/main/provision_softap.c:262` — open SoftAP enables unauthenticated association

## Root Cause Hypothesis

Portal treats SoftAP association as sufficient authorization. Contract explicitly says v1 does not require authenticated SoftAP management beyond physical presence; open SoftAP + multi-client still makes that weak. Confidence: **high**.

## Proposed Remediation

**Preferred**: Require a one-time provisioning token or serial-displayed PIN on `/save` (and optionally button-confirm before accepting the POST).

**Alternatives**:
- WPA2 SoftAP alone (`softap-cleartext-creds` fix) raises the bar to join but does not authenticate the POST among associated clients.
- Rate-limit + require GPIO9 long-press to unlock `/save` for a short window.

**Files likely to change**:
- `firmware/main/provision_softap.c` (handler + form)
- Portal HTML / docs
- Possibly `failsafe` / button path if unlock-gated

**Tests to add or update**:
- Host or unit test: `save_post` rejects missing/invalid token; accepts valid token once.

## Risks & Considerations

- Overlaps with `softap-cleartext-creds`; WPA2 SoftAP reduces who can reach `/save` but does not fully replace POST auth.
- Contract currently accepts presence-only control; changing this is a product decision.
- PIN on device display/serial must be available offline for first boot.

## Open Questions

- [NEEDS CLARIFICATION: Prefer SoftAP PSK, portal PIN, button-unlock, or a combination for the next hardening pass?]
