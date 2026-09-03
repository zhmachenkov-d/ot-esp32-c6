---
type: Data ID
title: Data ID 0 Status
description: Mandatory status exchange; master CH/DHW flags and slave fault/activity bits.
tags: [opentherm, status]
timestamp: 2026-07-02T00:00:00Z
raw:
  - wiki/raw/opentherm/2003-02-07-opentherm-protocol-v2-2.md
  - wiki/raw/opentherm/2003-02-07-opentherm-protocol-v2-2-flag-bits.md
---

ID **0** is the mandatory status exchange. Initiate with `READ-DATA(id=0, master_status, 0x0000)` and expect `READ-ACK` with slave status. Do not use WRITE for ID 0.

## Master status (HB)

| Bit | Meaning | 0 | 1 |
|-----|---------|---|---|
| 0 | CH enable | disabled | enabled |
| 1 | DHW enable | disabled | enabled |
| 2 | Cooling enable | disabled | enabled |
| 3 | OTC active | not active | active |
| 4 | CH2 enable | disabled | enabled |
| 5–7 | reserved | | |

## Slave status (LB)

| Bit | Meaning | 0 | 1 |
|-----|---------|---|---|
| 0 | Fault | no fault | fault |
| 1 | CH active | not active | active |
| 2 | DHW active | not active | active |
| 3 | Flame on | off | on |
| 4 | Cooling active | not active | active |
| 5 | CH2 active | not active | active |
| 6 | Diagnostic event | none | event |
| 7 | reserved | | |

Status bits use **flag8** encoding — see [OpenTherm Data Encoding](/opentherm/opentherm-data-encoding.md). Other flag Data IDs (config, ASF, RBP, override): [OpenTherm Data IDs — Flag data](/opentherm/opentherm-data-ids.md#flag-data-flag8-bit-maps).

# Citations

[1] [wiki/raw/opentherm/2003-02-07-opentherm-protocol-v2-2.md](wiki/raw/opentherm/2003-02-07-opentherm-protocol-v2-2.md)
[2] [wiki/raw/opentherm/2003-02-07-opentherm-protocol-v2-2-flag-bits.md](wiki/raw/opentherm/2003-02-07-opentherm-protocol-v2-2-flag-bits.md)
