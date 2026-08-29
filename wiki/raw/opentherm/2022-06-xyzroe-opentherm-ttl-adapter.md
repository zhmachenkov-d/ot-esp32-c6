# OpenTherm-TTL Adapter (xyzroe)

> Source: https://github.com/xyzroe/OpenTherm-TTL-adapter
> Project site: https://xyzroe.cc/OpenTherm-TTL-adapter/
> Collected: 2026-08-29
> Published: 2022-06 (schematic); PCB revision 0.2; Gerber_v0.2.zip

## About

This adapter connects an OpenTherm-compatible boiler to any TTL port (TTL-USB, ESP8266, ESP32, Arduino, etc.).

Same size as popular RS485-TTL modules for mechanical compatibility.

License: Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0).

## Suggested software (from README)

- OpenTherm gateway for HomeAssistant — https://github.com/Laxilef/OTGateway
- Tasmota OpenTherm — https://tasmota.github.io/docs/OpenTherm/
- jpraus/arduino-opentherm — https://github.com/jpraus/arduino-opentherm
- Melnyk OpenTherm library (README notes slave mode only for that pairing) — https://github.com/ihormelnyk/opentherm_library/

## DIY assets

- iBOM: `files/iBOM.html`
- Gerbers: `files/Gerber_v0.2.zip`
- Schematic image: `images/Schematic.png` (local copy: `wiki/raw/opentherm/2022-06-xyzroe-opentherm-ttl-adapter-schematic.png`)

## Schematic summary (rev 1.0 / PCB 0.2, EasyEDA, xyzroe.cc, Jun 2022)

### Connectors

| Ref | Side | Pins |
|-----|------|------|
| X1 | OpenTherm bus | 2-pin screw terminal (polarity-free via bridge) |
| X2 | TTL | 1 GND, 2 RX, 3 TX, 4 VCC |

TTL naming relative to the MCU:

- **RX** — adapter → MCU (connect to library **in** / interrupt GPIO)
- **TX** — MCU → adapter (connect to library **out** GPIO)
- **VCC** — 3.3 V or 5 V logic supply shared with MCU

### Isolation and signalling

- Two **PC817B** optocouplers (U1 TX path, U2 RX path) galvanically isolate OT bus from TTL
- **BC858B** PNP (Q1) on OT-side TX drive
- Bridge of four **1N4148** (D1–D4) for polarity-independent OT connection
- Zeners: **4V7** (D5), **15V** (D6), **4V3** (D7) — OT voltage/current shaping
- Status LEDs: red TX, green RX (with 1 kΩ series)

### Key passives (from iBOM)

| Value | Refs |
|-------|------|
| 100 Ω | R2 |
| 220 Ω | R1 |
| 330 Ω | R3, R5 |
| 1 kΩ | R6, R7 |
| 10 kΩ | R4 (RX pull-down) |

## Inspired by (README)

- Laxilef OTGateway
- Schelte Bron Opentherm Gateway (otgw.tclcode.com)
- jpraus/arduino-opentherm
- Ihor Melnyk OpenTherm Adapter (ihormelnyk.com/opentherm_adapter)
- Tasmota OpenTherm
