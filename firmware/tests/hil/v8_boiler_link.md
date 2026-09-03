# HIL: V8 Boiler-link vs MQTT availability

**Quickstart**: V8 — FR-012  
**Status**: checklist stub (sign off in T043 / `results/`)

## Steps

1. Keep Wi‑Fi/MQTT up; break OT adapter path only.
2. Wait until ≥3 consecutive **keepalive/status** failures.
3. Observe `otc6/<id>/boiler_link` → `unhealthy`; MQTT `status` stays `online`.
4. Restore OT; confirm `healthy` after one successful keepalive/status.

## Expect

- Boiler-link is distinct from MQTT availability.
- Tiered catalog reads must not drive the unhealthy counter.

## Sign-off

- [ ] Pass
- [ ] Fail (notes: )
