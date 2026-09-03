# OpenTherm Protocol v2.2 — flag8 bit maps

Source: OpenTherm Protocol Specification v2.2 (2003-02-07), Application Layer.
Extracted for knowledge ingest. Bit sense is `[clear/0, set/1]` unless noted.

## Type and packing rules

- **flag8**: byte composed of 8 single-bit flags.
- **HB** / **LB**: high-byte / low-byte of the 16-bit DATA-VALUE field.
- Reserved / unused bits or bytes: transmitter sets to **0**; receiver **ignores** them (may be defined later).

## ID 0 — Status (flag8 / flag8)

### HB: Master status

| Bit | Name | 0 | 1 |
|-----|------|---|---|
| 0 | CH enable | CH disabled | CH enabled |
| 1 | DHW enable | DHW disabled | DHW enabled |
| 2 | Cooling enable | Cooling disabled | Cooling enabled |
| 3 | OTC active | OTC not active | OTC active |
| 4 | CH2 enable | CH2 disabled | CH2 enabled |
| 5–7 | reserved | | |

### LB: Slave status

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

Note: CH enable has priority over Control Setpoint (ID 1). Master may clear CH enable even if setpoint is non-zero.

## ID 2 — Master configuration (flag8 / u8)

### HB: Master configuration flags

| Bit | Name |
|-----|------|
| 0–7 | reserved (v2.2) |

### LB

Master MemberID code (`u8`, 0..255). Zero = customer non-specific device.

## ID 3 — Slave configuration (flag8 / u8)

### HB: Slave configuration flags

| Bit | Name | 0 | 1 |
|-----|------|---|---|
| 0 | DHW present | DHW not present | DHW present |
| 1 | Control type | modulating | on/off |
| 2 | Cooling config | cooling not supported | cooling supported |
| 3 | DHW config | instantaneous or not-specified | storage tank |
| 4 | Master low-off & pump control function | allowed | not allowed |
| 5 | CH2 present | CH2 not present | CH2 present |
| 6–7 | reserved | | |

### LB

Slave MemberID code (`u8`, 0..255).

## ID 5 — Application-specific fault flags (flag8 / u8)

### HB: ASF flags

| Bit | Name | 0 | 1 |
|-----|------|---|---|
| 0 | Service request | service not required | service required |
| 1 | Lockout-reset | remote reset disabled | remote reset enabled |
| 2 | Low water press | no water-pressure fault | water-pressure fault |
| 3 | Gas/flame fault | no gas/flame fault | gas/flame fault |
| 4 | Air press fault | no air-pressure fault | air-pressure fault |
| 5 | Water over-temp | no over-temperature fault | over-temperature fault |
| 6–7 | reserved | | |

### LB

OEM fault code (`u8`, 0..255) — OEM-specific; master may display only.

## ID 6 — Remote boiler parameter flags (flag8 / flag8)

Bit *n* corresponds to remote-boiler-parameter *n+1* (bit 0 → parameter 1 / DHW setpoint; bit 1 → parameter 2 / max CH setpoint).

### HB: Transfer-enable flags

| Bit | Parameter | 0 | 1 |
|-----|-----------|---|---|
| 0 | DHW setpoint | transfer disabled | transfer enabled |
| 1 | max CH setpoint | transfer disabled | transfer enabled |
| 2–7 | reserved | | |

### LB: Read/write flags

| Bit | Parameter | 0 | 1 |
|-----|-----------|---|---|
| 0 | DHW setpoint | read-only | read/write |
| 1 | max CH setpoint | read-only | read/write |
| 2–7 | reserved | | |

UNKNOWN-DATAID on read of ID 6 ≡ no remote-parameter support (all transfer-enable clear).

## ID 100 — Remote override function (flag8 / -)

### LB: Remote override function

| Bit | Name | 0 | 1 |
|-----|------|---|---|
| 0 | Manual change priority | disable overruling remote setpoint by manual change | enable overruling by manual change |
| 1 | Program change priority | disable overruling remote setpoint by program change | enable overruling by program change |
| 2–7 | reserved | | |

### HB

Reserved (`u8` 0). Used with ID 9 (`TrOverride`): non-zero = valid remote room setpoint; zero = no override.

## Source URL

https://ihormelnyk.com/Content/Pages/opentherm_library/Opentherm%20Protocol%20v2-2.pdf
