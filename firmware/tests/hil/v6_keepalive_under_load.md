# HIL: V6 Keepalive under load

**Quickstart**: V6 — SC-003 / FR-011  
**Status**: checklist stub (sign off in T043 / `results/`)

## Steps

1. Generate bursty HA writes across many writables.
2. Monitor OT keepalive/status cadence (analyzer, metric, or simulator timestamps).

## Expect

- ≥1 keepalive/status cycle per second sustained.

## Sign-off

- [ ] Pass
- [ ] Fail (notes: )
