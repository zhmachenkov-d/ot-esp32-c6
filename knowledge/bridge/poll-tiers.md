---
type: Playbook
title: Poll Tiers
description: Fast, slow, and promoted poll scheduling with time-budgeted multi-read per tick.
tags: [bridge, polling]
timestamp: 2026-07-02T00:00:00Z
---

How the **Bridge** schedules OpenTherm READ traffic for available Data IDs.

## Tier classes

| Tier | Catalog default | Poll cadence |
|------|-----------------|--------------|
| Fast | `OT_POLL_FAST` | Every fast tick (~1 s) |
| Slow | `OT_POLL_SLOW` | At most once per slow interval (default 60 s) |
| Promoted | (temporary) | Fast cadence until decay |

Tier defaults are defined per ID in `components/opentherm/ot_data_catalog.c`.

## Adaptive promotion

`ot_poll_promote(id)` moves an ID to the fast tier for `CONFIG_OT_POLL_PROMOTE_MS` (default 5 min) after:

- A Zigbee write to that ID (Thermostat, discovery endpoint, or spillover)
- A detected value or validity change on read (`raw` or `status` delta)

See repo `CONTEXT.md` — Adaptive promotion.

## Fast tick loop (`ot_poll_task`)

Each `CONFIG_OT_POLL_FAST_INTERVAL_MS` period (default 1000 ms):

```
opentherm_process()
  → status keepalive (master ID 0 flags)
  → 120 ms gap
  → dedicated slave READ of ID 0
  → time-budgeted reads (promoted → fast RR → slow due)
  → sleep remainder to hit fast interval
```

## Time-budgeted multi-read

Within each tick, after ID 0, the poll engine performs multiple `opentherm_read_id()` calls until the tick budget is exhausted (~50 ms margin before the fast interval ends). Priority when picking the next ID:

1. Promoted IDs
2. Fast-tier IDs (round-robin)
3. Slow-tier IDs that are due

Each inter-read gap is **120 ms** per OpenTherm frame timing.

With a 1 s tick and 120 ms gaps, typical throughput is **5–7 reads per tick**, keeping fast-tier sensors (e.g. ID 25 Tboiler) near 1 Hz.

## Change detection and routing

On change (`raw` or `status` differs from cache):

1. `ot_poll_promote(id)`
2. `ot_zcl_route_apply_read(id, raw, status)` if Zigbee joined

No report is sent when the value is unchanged.

## Configuration

| Kconfig | Default | Purpose |
|---------|---------|---------|
| `OT_POLL_FAST_INTERVAL_MS` | 1000 | Fast tick period |
| `OT_POLL_SLOW_INTERVAL_MS` | 60000 | Slow tier minimum gap |
| `OT_POLL_PROMOTE_MS` | 300000 | Promotion decay duration |

## Module layout

Poll scheduling lives in `main/ot_poll.c`. Thermostat write paths remain in `main/ot_bridge.c`.

# Citations

[1] `main/ot_poll.c` — `ot_poll_task`, `poll_budget_reads`, `ot_poll_promote`
[2] `main/Kconfig.projbuild` — poll interval options
[3] `components/opentherm/ot_data_catalog.c` — per-ID tier defaults
