---
type: Playbook
title: OpenTherm GPIO Wiring
description: Default GPIO2/3 pin assignment for OpenTherm adapter on WeAct ESP32-C6 Mini.
tags: [bridge, hardware, gpio]
timestamp: 2026-08-30T00:00:00Z
---

Default OpenTherm adapter wiring for this firmware on [WeAct ESP32-C6 Mini](/zephyr/weact-esp32c6-mini.md).

## Pin assignment

| Signal | Default GPIO | Kconfig |
|--------|--------------|---------|
| Adapter input (interrupt) | **GPIO2** | `CONFIG_OT_IN_GPIO` |
| Adapter output | **GPIO3** | `CONFIG_OT_OUT_GPIO` |

Override in `idf.py menuconfig` → **OpenTherm Bridge**.

## Requirements

- GPIO **input must be interrupt-capable** — Manchester decoding uses `handleInterrupt()`
- MCU GPIO cannot drive the OT bus directly — use an OpenTherm adapter (7–18 V / mA signalling per [OpenTherm Protocol](/opentherm/opentherm-protocol.md)), e.g. [xyzroe OpenTherm-TTL Adapter](/opentherm/xyzroe-opentherm-ttl-adapter.md) or [Melnyk’s adapter](https://ihormelnyk.com/opentherm_adapter)
- On xyzroe modules: header **RX → OT in**, **TX → OT out**, plus VCC/GND
- Do **not** use GPIO12/13 for OT on Mini — they are the board’s only USB Serial/JTAG path

## Library defaults vs this project

Melnyk library examples use GPIO 4/5; [Sazanof ESP-IDF OpenTherm](/opentherm/sazanof-esp-idf-opentherm.md) examples use 22/23. This project uses **2/3** so USB console/flash stay available on Mini.

# Citations

[1] `main/Kconfig.projbuild` — `OT_IN_GPIO`, `OT_OUT_GPIO`
[2] [Melnyk OpenTherm Library](/opentherm/melnyk-opentherm-library.md)
[3] [xyzroe OpenTherm-TTL Adapter](/opentherm/xyzroe-opentherm-ttl-adapter.md)
