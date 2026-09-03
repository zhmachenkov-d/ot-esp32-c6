# HIL: V7 Fail-safe

**Quickstart**: V7 — SC-004 / FR-006 Option A  
**Status**: checklist stub (sign off in T043 / `results/`)

## Steps

1. During heat demand, stop broker or drop Wi‑Fi.
2. During **10 000 ms** entry timer: app availability stays `online`; writes may still apply.
3. After timer: fail-safe active; app presents `offline`; OT keepalive continues; last CH held; writes → `ot/<N>/rejection` `rejected_failsafe`.
4. Restore link; after **2 s** debounce, entities recover; at most one retained ID 1.

## Sign-off

- [ ] Pass
- [ ] Fail (notes: )
