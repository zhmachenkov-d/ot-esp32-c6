---
type: Playbook
title: Discovery Catalog
description: OpenTherm Data ID discovery, NVS cache, boot validation, and available vs unknown classification.
tags: [bridge, discovery, catalog]
timestamp: 2026-07-02T00:00:00Z
---

How the **Bridge** discovers which OpenTherm Data IDs the boiler supports and persists that set for fast subsequent boots.

## Classification

| Discovery result | Meaning |
|------------------|---------|
| `READ-ACK` | Available — ID is supported and returned valid data |
| `DATA_INVALID` | Available — ID is supported but value is not valid right now |
| `UNKNOWN-DATAID` | Unsupported — ID is not implemented on this boiler |

Only **available** IDs appear in the runtime catalog and get Zigbee endpoints. See repo `CONTEXT.md` glossary — Available Data ID.

## Discovery flow

```
ot_catalog_init()
  → load NVS cache if present
ot_catalog_start()  (background task)
  → cache hit: ot_catalog_validate() re-probes cached IDs once
  → cache miss: ot_discover_all() probes IDs 0–127 (~13 s)
  → save to NVS, mark validated
```

`ot_discover_all()` uses `opentherm_read_id()` with **120 ms** inter-frame gaps.

## NVS persistence

| Field | Content |
|-------|---------|
| Namespace | `ot_catalog` |
| Payload | `version`, sorted `id_list[]`, `timestamp` |

`ot_catalog_nvs_load()` / `ot_catalog_nvs_save()` / `ot_catalog_nvs_clear()` in `components/opentherm/ot_catalog_nvs.c`.

## Boot validation

On cache hit, `ot_catalog_validate()` re-probes each cached ID. If the set changes, NVS is updated and `zb_ot_bridge_apply_catalog_update()` registers or removes discovery-driven endpoints.

No periodic background scan — validation runs at boot and on manual rescan only.

## Runtime access

`ot_catalog_get()` returns the validated `ot_runtime_catalog_t`. The poll engine and ZCL routing use this to know which IDs to read and expose.

## Manual operations

See [Catalog Commands](/bridge/catalog-commands.md) for `RescanCatalog` and `ClearCatalog`.

# Citations

[1] `components/opentherm/ot_discover.c`
[2] `components/opentherm/ot_catalog_nvs.c`
[3] `CONTEXT.md` — Available Data ID, Cached discovery catalog
