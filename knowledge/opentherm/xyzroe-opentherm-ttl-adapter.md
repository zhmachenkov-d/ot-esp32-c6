---
type: Hardware
title: xyzroe OpenTherm-TTL Adapter
description: Opto-isolated OpenTherm↔TTL module (RS485-sized); RX/TX/VCC/GND header.
resource: https://github.com/xyzroe/OpenTherm-TTL-adapter
tags: [opentherm, hardware, adapter]
timestamp: 2026-08-29T00:00:00Z
raw:
  - wiki/raw/opentherm/2022-06-xyzroe-opentherm-ttl-adapter.md
  - wiki/raw/opentherm/2022-06-xyzroe-opentherm-ttl-adapter-schematic.png
---

[xyzroe/OpenTherm-TTL-adapter](https://github.com/xyzroe/OpenTherm-TTL-adapter) is a compact **OpenTherm ↔ TTL** level adapter (PCB **0.2**, schematic **2022-06**, CC BY-NC-SA 4.0). Form factor matches common **RS485-TTL** modules. DIY Gerbers/iBOM are in-repo; also sold by the author.

MCU GPIO cannot drive OT bus voltages/currents — this board (or Melnyk’s adapter) sits between the boiler and the master firmware. See [OpenTherm Protocol](/opentherm/opentherm-protocol.md) physical layer and [OpenTherm GPIO Wiring](/bridge/opentherm-gpio-wiring.md).

## Connectors

| Side | Ref | Signals |
|------|-----|---------|
| Boiler OT bus | X1 | 2-wire, polarity-free (diode bridge) |
| MCU TTL | X2 | **GND**, **RX**, **TX**, **VCC** (3.3 V or 5 V) |

Wire to this project’s defaults (**GPIO2** in / **GPIO3** out):

| Adapter pin | MCU / library | Role |
|-------------|---------------|------|
| RX | OT **in** (interrupt) | Bus → MCU |
| TX | OT **out** | MCU → bus |
| VCC / GND | Board 3V3 / GND | Shared logic supply |

Compatible with native IDF masters such as [Sazanof ESP-IDF OpenTherm](/opentherm/sazanof-esp-idf-opentherm.md) and Arduino/ESP masters such as [Melnyk OpenTherm Library](/opentherm/melnyk-opentherm-library.md).

## Circuit (summary)

- Dual **PC817B** optocouplers — galvanic isolation TX and RX paths
- **BC858B** + **1N4148** bridge + **4V7 / 15V / 4V3** zeners — OT-side drive and protection
- Red TX / green RX activity LEDs

Schematic asset: `wiki/raw/opentherm/2022-06-xyzroe-opentherm-ttl-adapter-schematic.png`.

# Citations

[1] [wiki/raw/opentherm/2022-06-xyzroe-opentherm-ttl-adapter.md](wiki/raw/opentherm/2022-06-xyzroe-opentherm-ttl-adapter.md)
[2] [wiki/raw/opentherm/2022-06-xyzroe-opentherm-ttl-adapter-schematic.png](wiki/raw/opentherm/2022-06-xyzroe-opentherm-ttl-adapter-schematic.png)
[3] [github.com/xyzroe/OpenTherm-TTL-adapter](https://github.com/xyzroe/OpenTherm-TTL-adapter)
