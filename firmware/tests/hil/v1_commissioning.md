# HIL: V1 Commissioning

**Quickstart**: V1 — SoftAP commission + GPIO9 re-provision  
**Status**: checklist stub (sign off in T043 / `results/`)

## Steps

1. Flash unconfigured board (`idf.py flash`).
2. Join open SoftAP `OTC6-XXXX` (XXXX = last 4 of device id).
3. Open captive portal; submit Wi‑Fi + MQTT + CH min/max (defaults 10 / 90).
4. Confirm STA + MQTT `online`; HA sees device after discovery.
5. Hold GPIO9 ≥5 s → credentials cleared → SoftAP returns.

## Expect

- No serial credential entry required (SC-005 / FR-005).
- SoftAP is **open** (no WPA password).

## Sign-off

- [ ] Pass
- [ ] Fail (notes: )
