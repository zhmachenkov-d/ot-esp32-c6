---
type: Reference
title: OpenTherm Data Encoding
description: f8.8 temperature encoding and flag8 status bit packing.
tags: [opentherm, encoding]
timestamp: 2026-08-30T00:00:00Z
raw:
  - wiki/raw/opentherm/2003-02-07-opentherm-protocol-v2-2.md
  - wiki/raw/opentherm/2003-02-07-opentherm-protocol-v2-2-flag-bits.md
---

## f8.8 temperatures

Temperatures and setpoints use **f8.8** fixed point: 16-bit signed, 8 fractional bits (divide by 256 for °C).

Example: 21.5°C → `0x1580` (5504 / 256 = 21.5).

Used by [Data ID 1 TSet](/opentherm/data-id-1-tset.md) and [Data ID 25 Tboiler](/opentherm/data-id-25-tboiler.md). Full ID list: [OpenTherm Data IDs](/opentherm/opentherm-data-ids.md).

## Other numeric types

| Type | Meaning |
|------|---------|
| u8 | unsigned 8-bit (0..255) |
| s8 | signed 8-bit (−128..127) |
| u16 | unsigned 16-bit |
| s16 | signed 16-bit |
| flag8 | eight single-bit flags in one byte |

**HB** / **LB** = high / low byte of the 16-bit DATA-VALUE. Dummy / unavailable bytes use `0x00`.

## flag8 fields

A **flag8** packs eight independent bits. Sense is usually `[clear/0, set/1]`. Reserved bits: transmitter writes **0**; receiver ignores (may be assigned in later specs).

Common flag Data IDs and full bit maps (Status, Master/Slave config, ASF, RBP, remote override): see [OpenTherm Data IDs — Flag data](/opentherm/opentherm-data-ids.md#flag-data-flag8-bit-maps) and [Data ID 0 Status](/opentherm/data-id-0-status.md).

# Citations

[1] [wiki/raw/opentherm/2003-02-07-opentherm-protocol-v2-2.md](wiki/raw/opentherm/2003-02-07-opentherm-protocol-v2-2.md)
[2] [wiki/raw/opentherm/2003-02-07-opentherm-protocol-v2-2-flag-bits.md](wiki/raw/opentherm/2003-02-07-opentherm-protocol-v2-2-flag-bits.md)
[3] [OpenTherm Protocol v2.2 PDF](https://ihormelnyk.com/Content/Pages/opentherm_library/Opentherm%20Protocol%20v2-2.pdf)
