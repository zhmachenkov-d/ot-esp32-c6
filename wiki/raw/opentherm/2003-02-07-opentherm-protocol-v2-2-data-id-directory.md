# OpenTherm Protocol v2.2 — Data-ID Directory

Source: OpenTherm Protocol Specification v2.2 (2003-02-07), Application Layer summary directory
(commonly mirrored as `Opentherm Protocol v2-2.pdf`). Extracted for knowledge ingest.

## Ranges

- ID 0..127: OpenTherm pre-defined (open area)
- ID 128..255: Test & Diagnostic (members only; gated by MemberID handshake)

## Directory (ID 0..127, as listed)

Columns: ID | Master→Slave (R=read request / W=write) | Slave→Master | Name | HB/LB type | Description

| ID | R | W | Name | Type | Description |
|----|---|---|------|------|-------------|
| 0 | R | | Status | flag8 / flag8 | Master and Slave Status flags |
| 1 | | W | TSet | f8.8 | Control setpoint ie CH water temperature setpoint (°C) |
| 2 | | W | M-Config / M-MemberIDcode | flag8 / u8 | Master Configuration Flags / Master MemberID Code |
| 3 | R | | S-Config / S-MemberIDcode | flag8 / u8 | Slave Configuration Flags / Slave MemberID Code |
| 4 | | W | Command | u8 / u8 | Remote Command |
| 5 | R | | ASF-flags / OEM-fault-code | flag8 / u8 | Application-specific fault flags and OEM fault code |
| 6 | R | | RBP-flags | flag8 / flag8 | Remote boiler parameter transfer-enable & read/write flags |
| 7 | | W | Cooling-control | f8.8 | Cooling control signal (%) |
| 8 | | W | TsetCH2 | f8.8 | Control setpoint for 2nd CH circuit (°C) |
| 9 | R | | TrOverride | f8.8 | Remote override room setpoint |
| 10 | R | | TSP | u8 / u8 | Number of Transparent-Slave-Parameters supported by slave |
| 11 | R | W | TSP-index / TSP-value | u8 / u8 | Index number / Value of referred-to transparent slave parameter |
| 12 | R | | FHB-size | u8 / u8 | Size of Fault-History-Buffer supported by slave |
| 13 | R | | FHB-index / FHB-value | u8 / u8 | Index number / Value of referred-to fault-history buffer entry |
| 14 | | W | Max-rel-mod-level-setting | f8.8 | Maximum relative modulation level setting (%) |
| 15 | R | | Max-Capacity / Min-Mod-Level | u8 / u8 | Maximum boiler capacity (kW) / Minimum boiler modulation level (%) |
| 16 | | W | TrSet | f8.8 | Room Setpoint (°C) |
| 17 | R | | Rel.-mod-level | f8.8 | Relative Modulation Level (%) |
| 18 | R | | CH-pressure | f8.8 | Water pressure in CH circuit (bar) |
| 19 | R | | DHW-flow-rate | f8.8 | Water flow rate in DHW circuit (litres/minute) |
| 20 | R | W | Day-Time | special / u8 | Day of Week and Time of Day |
| 21 | R | W | Date | u8 / u8 | Calendar date |
| 22 | R | W | Year | u16 | Calendar year |
| 23 | | W | TrSetCH2 | f8.8 | Room Setpoint for 2nd CH circuit (°C) |
| 24 | | W | Tr | f8.8 | Room temperature (°C) |
| 25 | R | | Tboiler | f8.8 | Boiler flow water temperature (°C) |
| 26 | R | | Tdhw | f8.8 | DHW temperature (°C) |
| 27 | R | | Toutside | f8.8 | Outside temperature (°C) |
| 28 | R | | Tret | f8.8 | Return water temperature (°C) |
| 29 | R | | Tstorage | f8.8 | Solar storage temperature (°C) |
| 30 | R | | Tcollector | s16 | Solar collector temperature (°C) |
| 31 | R | | TflowCH2 | f8.8 | Flow water temperature CH2 circuit (°C) |
| 32 | R | | Tdhw2 | f8.8 | Domestic hot water temperature 2 (°C) |
| 33 | R | | Texhaust | s16 | Boiler exhaust temperature (°C) |
| 48 | R | | TdhwSet-UB / TdhwSet-LB | s8 / s8 | DHW setpoint upper & lower bounds for adjustment (°C) |
| 49 | R | | MaxTSet-UB / MaxTSet-LB | s8 / s8 | Max CH water setpoint upper & lower bounds for adjustment (°C) |
| 50 | R | | Hcratio-UB / Hcratio-LB | s8 / s8 | OTC heat curve ratio upper & lower bounds for adjustment |
| 56 | R | W | TdhwSet | f8.8 | DHW setpoint (°C) (Remote parameter 1) |
| 57 | R | W | MaxTSet | f8.8 | Max CH water setpoint (°C) (Remote parameters 2) |
| 58 | R | W | Hcratio | f8.8 | OTC heat curve ratio (Remote parameter 3) |
| 100 | R | | Remote override function | flag8 / - | Function of manual and program changes in master and remote room setpoint |
| 115 | R | | OEM diagnostic code | u16 | OEM-specific diagnostic/service code |
| 116 | R | W | Burner starts | u16 | Number of starts burner |
| 117 | R | W | CH pump starts | u16 | Number of starts CH pump |
| 118 | R | W | DHW pump/valve starts | u16 | Number of starts DHW pump/valve |
| 119 | R | W | DHW burner starts | u16 | Number of starts burner during DHW mode |
| 120 | R | W | Burner operation hours | u16 | Number of hours that burner is in operation (i.e. flame on) |
| 121 | R | W | CH pump operation hours | u16 | Number of hours that CH pump has been running |
| 122 | R | W | DHW pump/valve operation hours | u16 | Number of hours that DHW pump has been running or DHW valve has been opened |
| 123 | R | W | DHW burner operation hours | u16 | Number of hours that burner is in operation during DHW mode |
| 124 | | W | OpenTherm version Master | f8.8 | The implemented version of the OpenTherm Protocol Specification in the master |
| 125 | R | | OpenTherm version Slave | f8.8 | The implemented version of the OpenTherm Protocol Specification in the slave |
| 126 | | W | Master-version | u8 / u8 | Master product version number and type |
| 127 | R | | Slave-version | u8 / u8 | Slave product version number and type |

## Class map (v2.2)

| Class | Topic | Typical IDs |
|-------|-------|-------------|
| 1 | Control and Status | 0, 1, 5, 8, 115 |
| 2 | Configuration | 2, 3, 124–127 |
| 3 | Remote Commands | 4 |
| 4 | Sensor and Informational | 16–33, 116–123 |
| 5 | Pre-Defined Remote Boiler Parameters | 6, 48–58 |
| 6 | Transparent Slave Parameters | 10, 11 |
| 7 | Fault History | 12, 13 |
| 8 | Special Applications | 7, 9, 14, 15, 100 |

Unlisted IDs in 0..127 are unused / reserved in v2.2 (later Technical Spec revisions may assign them).

## Source URL

https://ihormelnyk.com/Content/Pages/opentherm_library/Opentherm%20Protocol%20v2-2.pdf
