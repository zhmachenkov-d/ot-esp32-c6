---
type: Playbook
title: Catalog Commands
description: RescanCatalog and ClearCatalog manufacturer commands on the spillover cluster.
tags: [bridge, catalog, commands]
timestamp: 2026-07-02T00:00:00Z
---

Manufacturer-specific ZCL commands on the spillover cluster for catalog management.

## Command table

| Command | ID | Action |
|---------|-----|--------|
| `RescanCatalog` | `0x00` | Full re-probe of IDs 0–127; update NVS and endpoints |
| `ClearCatalog` | `0x01` | Erase NVS cache; device reboots for full discovery |

Both commands are sent to endpoint `CONFIG_OT_SPILLOVER_ENDPOINT` (default 20), cluster `0xFC01`.

## RescanCatalog

```
Coordinator → RescanCatalog (0x00)
  → ot_catalog_force_rescan()
  → zb_ot_bridge_apply_catalog_update()
  → ot_poll_rebuild_list()
```

Runs in a background FreeRTOS task. Expect ~13 s of OpenTherm traffic during the scan. Endpoint topology may change if boiler capabilities changed.

## ClearCatalog

```
Coordinator → ClearCatalog (0x01)
  → ot_catalog_clear()  (NVS erase)
  → esp_restart()
```

On next boot, no NVS cache exists so `ot_discover_all()` runs a full scan before Zigbee join completes endpoint registration.

## Home Assistant usage

Install the ZHA quirk from `docs/ha-zha-quirk-ot-zb-bridge-v2.py`, or issue commands manually:

```python
# RescanCatalog
await zha.issue_zigbee_cluster_command(
    "issue_command",
    ieee=device.ieee,
    endpoint_id=20,
    cluster_id=0xFC01,
    command=0x00,
    command_type=0x01,  # server
)

# ClearCatalog (triggers reboot)
await zha.issue_zigbee_cluster_command(
    "issue_command",
    ieee=device.ieee,
    endpoint_id=20,
    cluster_id=0xFC01,
    command=0x01,
    command_type=0x01,
)
```

Adjust `endpoint_id` if `CONFIG_OT_SPILLOVER_ENDPOINT` was changed in menuconfig.

# Citations

[1] `main/zb_ot_bridge.c` — `zb_spillover_cmd_handler`, `catalog_rescan_task`
[2] `main/zb_ot_bridge.h` — `OT_SPILLOVER_CMD_RESCAN`, `OT_SPILLOVER_CMD_CLEAR`
[3] `docs/ha-zha-quirk-ot-zb-bridge-v2.py`
