---
type: Reference
title: OpenTherm Data IDs
description: Full catalog of OpenTherm Data IDs 0–127 (v2.2 directory plus common post-v2.2 assignments).
tags: [opentherm, data-id]
timestamp: 2026-08-30T00:00:00Z
raw:
  - wiki/raw/opentherm/2003-02-07-opentherm-protocol-v2-2-data-id-directory.md
  - wiki/raw/opentherm/2023-ihormelnyk-opentherm-message-ids.md
  - wiki/raw/opentherm/2003-02-07-opentherm-protocol-v2-2-flag-bits.md
---

Catalog of **Data-ID** values in the open area (**0..127**). IDs **128..255** are member Test & Diagnostic only (MemberID handshake). See [OpenTherm Protocol](/opentherm/opentherm-protocol.md) and [OpenTherm Data Encoding](/opentherm/opentherm-data-encoding.md).

**R/W** is from the master’s perspective (R = master may READ-DATA; W = master may WRITE-DATA). **Spec**: `v2.2` = Protocol v2.2 directory; `later` = assigned in later Technical Spec revisions as listed by Melnyk `OpenThermMessageID`.

Detail pages exist for [Data ID 0 Status](/opentherm/data-id-0-status.md), [Data ID 1 TSet](/opentherm/data-id-1-tset.md), and [Data ID 25 Tboiler](/opentherm/data-id-25-tboiler.md).

## Class 1 — Control and Status

| ID | Name | R/W | Type | Spec | Notes |
|----|------|-----|------|------|-------|
| 0 | Status | R | flag8 / flag8 | v2.2 | Mandatory status exchange |
| 1 | TSet | W | f8.8 | v2.2 | CH control setpoint (°C) |
| 5 | ASF-flags / OEM-fault-code | R | flag8 / u8 | v2.2 | Application fault flags |
| 8 | TsetCH2 | W | f8.8 | v2.2 | CH2 control setpoint (°C) |
| 115 | OEM diagnostic code | R | u16 | v2.2 | OEM service code |

## Class 2 — Configuration

| ID | Name | R/W | Type | Spec | Notes |
|----|------|-----|------|------|-------|
| 2 | M-Config / M-MemberIDcode | W | flag8 / u8 | v2.2 | Master config / MemberID |
| 3 | S-Config / S-MemberIDcode | R | flag8 / u8 | v2.2 | Slave config / MemberID |
| 124 | OpenTherm version Master | W | f8.8 | v2.2 | Protocol version in master |
| 125 | OpenTherm version Slave | R | f8.8 | v2.2 | Protocol version in slave |
| 126 | Master-version | W | u8 / u8 | v2.2 | Product type / version |
| 127 | Slave-version | R | u8 / u8 | v2.2 | Product type / version |

## Class 3 — Remote Commands

| ID | Name | R/W | Type | Spec | Notes |
|----|------|-----|------|------|-------|
| 4 | Command | W | u8 / u8 | v2.2 | HB command code (1=BLOR, 2=CHWF) |

## Class 4 — Sensor and Informational

| ID | Name | R/W | Type | Spec | Notes |
|----|------|-----|------|------|-------|
| 16 | TrSet | W | f8.8 | v2.2 | Room setpoint (°C) |
| 17 | Rel.-mod-level | R | f8.8 | v2.2 | Relative modulation (%) |
| 18 | CH-pressure | R | f8.8 | v2.2 | CH pressure (bar) |
| 19 | DHW-flow-rate | R | f8.8 | v2.2 | DHW flow (l/min) |
| 20 | Day-Time | R/W | special / u8 | v2.2 | Day of week + time |
| 21 | Date | R/W | u8 / u8 | v2.2 | Calendar date |
| 22 | Year | R/W | u16 | v2.2 | Calendar year |
| 23 | TrSetCH2 | W | f8.8 | v2.2 | Room setpoint CH2 (°C) |
| 24 | Tr | W | f8.8 | v2.2 | Room temperature (°C) |
| 25 | Tboiler | R | f8.8 | v2.2 | Boiler flow water (°C) |
| 26 | Tdhw | R | f8.8 | v2.2 | DHW temperature (°C) |
| 27 | Toutside | R | f8.8 | v2.2 | Outside temperature (°C) |
| 28 | Tret | R | f8.8 | v2.2 | Return water (°C) |
| 29 | Tstorage | R | f8.8 | v2.2 | Solar storage (°C) |
| 30 | Tcollector | R | s16 | v2.2 | Solar collector (°C); Melnyk comment says f8.8 |
| 31 | TflowCH2 | R | f8.8 | v2.2 | Flow CH2 (°C) |
| 32 | Tdhw2 | R | f8.8 | v2.2 | DHW temperature 2 (°C) |
| 33 | Texhaust | R | s16 | v2.2 | Exhaust temperature (°C) |
| 34 | TboilerHeatExchanger | R | f8.8 | later | Heat exchanger (°C) |
| 35 | BoilerFanSpeedSetpointAndActual | R | u8 / u8 | later | Fan setpoint / actual |
| 36 | FlameCurrent | R | f8.8 | later | Flame current (μA) |
| 37 | TrCH2 | R | f8.8 | later | Room temp CH2 (°C) |
| 38 | RelativeHumidity | R | f8.8 | later | Relative humidity (%) |
| 39 | TrOverride2 | R | f8.8 | later | Remote override room setpoint 2 |
| 96 | CoolingOperationHours | R/W | u16 | later | Cooling mode hours |
| 97 | PowerCycles | R/W | u16 | later | Power cycles |
| 98 | RFsensorStatusInformation | R/W | special / special | later | RF strength / battery |
| 113 | UnsuccessfulBurnerStarts | R/W | u16 | later | Failed burner starts |
| 114 | FlameSignalTooLowNumber | R/W | u16 | later | Low flame-signal count |
| 116 | Burner starts | R/W | u16 | v2.2 | Reset by writing 0 optional |
| 117 | CH pump starts | R/W | u16 | v2.2 | |
| 118 | DHW pump/valve starts | R/W | u16 | v2.2 | |
| 119 | DHW burner starts | R/W | u16 | v2.2 | |
| 120 | Burner operation hours | R/W | u16 | v2.2 | |
| 121 | CH pump operation hours | R/W | u16 | v2.2 | |
| 122 | DHW pump/valve operation hours | R/W | u16 | v2.2 | |
| 123 | DHW burner operation hours | R/W | u16 | v2.2 | |

## Class 5 — Remote Boiler Parameters

| ID | Name | R/W | Type | Spec | Notes |
|----|------|-----|------|------|-------|
| 6 | RBP-flags | R | flag8 / flag8 | v2.2 | Transfer-enable & R/W flags |
| 48 | TdhwSet-UB / TdhwSet-LB | R | s8 / s8 | v2.2 | DHW setpoint bounds (°C) |
| 49 | MaxTSet-UB / MaxTSet-LB | R | s8 / s8 | v2.2 | Max CH setpoint bounds (°C) |
| 50 | Hcratio-UB / Hcratio-LB | R | s8 / s8 | v2.2 | OTC heat-curve ratio bounds |
| 56 | TdhwSet | R/W | f8.8 | v2.2 | DHW setpoint (°C) |
| 57 | MaxTSet | R/W | f8.8 | v2.2 | Max CH water setpoint (°C) |
| 58 | Hcratio | R/W | f8.8 | v2.2 | OTC heat-curve ratio |

## Class 6 — Transparent Slave Parameters

| ID | Name | R/W | Type | Spec | Notes |
|----|------|-----|------|------|-------|
| 10 | TSP | R | u8 / u8 | v2.2 | Number of TSPs |
| 11 | TSP-index / TSP-value | R/W | u8 / u8 | v2.2 | Indexed TSP access |

## Class 7 — Fault History

| ID | Name | R/W | Type | Spec | Notes |
|----|------|-----|------|------|-------|
| 12 | FHB-size | R | u8 / u8 | v2.2 | Fault-history buffer size |
| 13 | FHB-index / FHB-value | R | u8 / u8 | v2.2 | Indexed FHB entry |

## Class 8 — Special Applications

| ID | Name | R/W | Type | Spec | Notes |
|----|------|-----|------|------|-------|
| 7 | Cooling-control | W | f8.8 | v2.2 | Cooling control signal (%) |
| 9 | TrOverride | R | f8.8 | v2.2 | Remote override room setpoint |
| 14 | Max-rel-mod-level-setting | W | f8.8 | v2.2 | Max relative modulation (%) |
| 15 | Max-Capacity / Min-Mod-Level | R | u8 / u8 | v2.2 | Max capacity (kW) / min mod (%) |
| 100 | Remote override function | R | flag8 / - | v2.2 | Manual/program override behaviour |
| 99 | RemoteOverrideOperatingModeHeatingDHW | R/W | special / special | later | HC1/HC2 / DHW operating mode |

## Ventilation / heat-recovery (later)

| ID | Name | R/W | Type | Spec |
|----|------|-----|------|------|
| 70 | StatusVentilationHeatRecovery | R | flag8 / flag8 | later |
| 71 | Vset | W | - / u8 | later |
| 72 | ASFflagsOEMfaultCodeVentilationHeatRecovery | R | flag8 / u8 | later |
| 73 | OEMDiagnosticCodeVentilationHeatRecovery | R | u16 | later |
| 74 | SConfigSMemberIDCodeVentilationHeatRecovery | R | flag8 / u8 | later |
| 75 | OpenThermVersionVentilationHeatRecovery | R | f8.8 | later |
| 76 | VentilationHeatRecoveryVersion | R | u8 / u8 | later |
| 77 | RelVentLevel | R | - / u8 | later |
| 78 | RHexhaust | R | - / u8 | later |
| 79 | CO2exhaust | R | u16 | later |
| 80 | Tsi | R | f8.8 | later |
| 81 | Tso | R | f8.8 | later |
| 82 | Tei | R | f8.8 | later |
| 83 | Teo | R | f8.8 | later |
| 84 | RPMexhaust | R | u16 | later |
| 85 | RPMsupply | R | u16 | later |
| 86 | RBPflagsVentilationHeatRecovery | R | flag8 / flag8 | later |
| 87 | NominalVentilationValue | W | u8 / - | later |
| 88 | TSPventilationHeatRecovery | R | u8 / u8 | later |
| 89 | TSPindexTSPvalueVentilationHeatRecovery | R/W | u8 / u8 | later |
| 90 | FHBsizeVentilationHeatRecovery | R | u8 / u8 | later |
| 91 | FHBindexFHBvalueVentilationHeatRecovery | R | u8 / u8 | later |

## Solar storage / electricity / brand (later)

| ID | Name | R/W | Type | Spec |
|----|------|-----|------|------|
| 93 | Brand | R/W | u8 / u8 | later |
| 94 | BrandVersion | R/W | u8 / u8 | later |
| 95 | BrandSerialNumber | R/W | u8 / u8 | later |
| 101 | StatusSolarStorage | R | flag8 / flag8 | later |
| 102 | ASFflagsOEMfaultCodeSolarStorage | R | flag8 / u8 | later |
| 103 | SConfigSMemberIDcodeSolarStorage | R | flag8 / u8 | later |
| 104 | SolarStorageVersion | R | u8 / u8 | later |
| 105 | TSPSolarStorage | R | u8 / u8 | later |
| 106 | TSPindexTSPvalueSolarStorage | R/W | u8 / u8 | later |
| 107 | FHBsizeSolarStorage | R | u8 / u8 | later |
| 108 | FHBindexFHBvalueSolarStorage | R | u8 / u8 | later |
| 109 | ElectricityProducerStarts | R/W | u16 | later |
| 110 | ElectricityProducerHours | R/W | u16 | later |
| 111 | ElectricityProduction | R | u16 | later |
| 112 | CumulativElectricityProduction | R | u16 | later |

## Unassigned in this catalog

Within 0..127, IDs not listed above (for example **40–47**, **51–55**, **59–69**, **92**) have no entry in the v2.2 directory or the Melnyk enum used here. Treat as unknown until a boiler/MemberID document defines them.

## Flag data (flag8 bit maps)

**flag8** is one byte of eight single-bit flags. In a 16-bit DATA-VALUE, **HB** is the high byte and **LB** the low byte. Reserved bits: transmitter sets **0**; receiver ignores. Encoding overview: [OpenTherm Data Encoding](/opentherm/opentherm-data-encoding.md). Detail page for ID 0: [Data ID 0 Status](/opentherm/data-id-0-status.md).

### ID 0 — Status (flag8 / flag8)

Master status (**HB**):

| Bit | Name | 0 | 1 |
|-----|------|---|---|
| 0 | CH enable | CH disabled | CH enabled |
| 1 | DHW enable | DHW disabled | DHW enabled |
| 2 | Cooling enable | Cooling disabled | Cooling enabled |
| 3 | OTC active | OTC not active | OTC active |
| 4 | CH2 enable | CH2 disabled | CH2 enabled |
| 5–7 | reserved | | |

Slave status (**LB**):

| Bit | Name | 0 | 1 |
|-----|------|---|---|
| 0 | Fault indication | no fault | fault |
| 1 | CH mode | CH not active | CH active |
| 2 | DHW mode | DHW not active | DHW active |
| 3 | Flame status | flame off | flame on |
| 4 | Cooling status | cooling not active | cooling active |
| 5 | CH2 mode | CH2 not active | CH2 active |
| 6 | Diagnostic indication | no diagnostics | diagnostic event |
| 7 | reserved | | |

CH enable overrides [Data ID 1 TSet](/opentherm/data-id-1-tset.md): master may clear CH enable even if setpoint is non-zero.

### ID 2 — Master configuration (flag8 / u8)

| Field | Content |
|-------|---------|
| HB bits 0–7 | reserved (v2.2) |
| LB | Master MemberID (`u8`, 0..255); 0 = customer non-specific |

### ID 3 — Slave configuration (flag8 / u8)

| Bit (HB) | Name | 0 | 1 |
|----------|------|---|---|
| 0 | DHW present | not present | present |
| 1 | Control type | modulating | on/off |
| 2 | Cooling config | not supported | supported |
| 3 | DHW config | instantaneous / not-specified | storage tank |
| 4 | Master low-off & pump control | allowed | not allowed |
| 5 | CH2 present | not present | present |
| 6–7 | reserved | | |

**LB**: Slave MemberID (`u8`, 0..255).

### ID 5 — ASF flags / OEM fault (flag8 / u8)

| Bit (HB) | Name | 0 | 1 |
|----------|------|---|---|
| 0 | Service request | not required | required |
| 1 | Lockout-reset | remote reset disabled | remote reset enabled |
| 2 | Low water press | no fault | water-pressure fault |
| 3 | Gas/flame fault | no fault | gas/flame fault |
| 4 | Air press fault | no fault | air-pressure fault |
| 5 | Water over-temp | no fault | over-temperature fault |
| 6–7 | reserved | | |

**LB**: OEM fault code (`u8`) — display only; meaning is OEM-specific.

### ID 6 — Remote boiler parameter flags (flag8 / flag8)

Bit *n* maps to remote-boiler-parameter *n+1* (bit 0 = DHW setpoint; bit 1 = max CH setpoint).

**HB** transfer-enable: 0 = disabled, 1 = enabled. **LB** read/write: 0 = read-only, 1 = read/write. Bits 2–7 reserved. UNKNOWN-DATAID on ID 6 ≡ no remote-parameter support.

### ID 100 — Remote override function (flag8 / -)

| Bit (LB) | Name | 0 | 1 |
|----------|------|---|---|
| 0 | Manual change priority | disable overruling remote setpoint by manual change | enable overruling by manual change |
| 1 | Program change priority | disable overruling remote setpoint by program change | enable overruling by program change |
| 2–7 | reserved | | |

**HB** reserved. Used with ID 9 (`TrOverride`): non-zero = valid remote room setpoint; zero = no override.

# Citations

[1] [wiki/raw/opentherm/2003-02-07-opentherm-protocol-v2-2-data-id-directory.md](wiki/raw/opentherm/2003-02-07-opentherm-protocol-v2-2-data-id-directory.md)
[2] [wiki/raw/opentherm/2023-ihormelnyk-opentherm-message-ids.md](wiki/raw/opentherm/2023-ihormelnyk-opentherm-message-ids.md)
[3] [wiki/raw/opentherm/2003-02-07-opentherm-protocol-v2-2-flag-bits.md](wiki/raw/opentherm/2003-02-07-opentherm-protocol-v2-2-flag-bits.md)
[4] [OpenTherm Protocol v2.2 PDF](https://ihormelnyk.com/Content/Pages/opentherm_library/Opentherm%20Protocol%20v2-2.pdf)
[5] [github.com/ihormelnyk/opentherm_library OpenTherm.h](https://github.com/ihormelnyk/opentherm_library/blob/master/src/OpenTherm.h)
