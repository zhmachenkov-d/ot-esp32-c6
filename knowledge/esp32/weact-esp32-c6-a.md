---
type: Hardware
title: WeAct ESP32-C6-A
description: DevKitC-1 pin-compatible ESP32-C6 WROOM board (former project default; superseded by Mini).
resource: https://github.com/WeActStudio/WeActStudio.ESP32-C6-A
tags: [esp32, board, weact]
timestamp: 2026-07-02T00:00:00Z
raw:
  - wiki/raw/esp32/weactstudio-esp32-c6-a.md
  - wiki/raw/esp32/2026-06-21-weact-esp32c6-a-board-shape.md
  - wiki/raw/esp32/2026-06-21-weact-esp32-c6-a-schematic.md
---

The [WeActStudio ESP32-C6-A](https://github.com/WeActStudio/WeActStudio.ESP32-C6-A) is a compact board around **ESP32-C6-WROOM-1-N4/N8**, designed as a pin-to-pin replacement for [ESP32-C6-DevKitC-1](https://docs.espressif.com/projects/espressif-esp-dev-kits/en/latest/esp32c6/esp32-c6-devkitc-1/index.html).

## On-board features

| Function | GPIO / signal | Notes |
|----------|---------------|-------|
| WS2812 RGB LED | IO8 | Level-shifted |
| User button | IO9 | SW2 |
| UART console | U0TXD, U0RXD | USB bridge |
| Native USB | USB_DP, USB_DN | |
| BOOT / RESET | SW1, CHIP_PU | Auto-program via DTR/RTS |

Headers **H1** and **H2** expose DevKitC-1-style GPIO breakout including IO12 and IO13. Current project OpenTherm defaults are on Mini — see [OpenTherm GPIO Wiring](/bridge/opentherm-gpio-wiring.md).

## Design assets

- Schematic: `Hardware/ESP32_C6_A_Sch.pdf`
- Board outline: `Hardware/ESP32C6-A Board Shape 外形.pdf`

Pair schematic with board outline for enclosure design and peripheral wiring.

## Related boards

Project default hardware is [WeAct ESP32-C6 Mini](/zephyr/weact-esp32c6-mini.md) (ESP32-C6FH4 chip-down, Zephyr target `weact_esp32c6_mini`), not DevKitC-1 pin-compatible.

## Software target

Use ESP-IDF with DevKitC-1 board profile or equivalent `sdkconfig` — see [ESP-IDF Get Started](/esp-idf/esp32-c6-get-started.md).

# Citations

[1] [wiki/raw/esp32/weactstudio-esp32-c6-a.md](wiki/raw/esp32/weactstudio-esp32-c6-a.md)
[2] [wiki/raw/esp32/2026-06-21-weact-esp32-c6-a-schematic.md](wiki/raw/esp32/2026-06-21-weact-esp32-c6-a-schematic.md)
[3] [wiki/raw/esp32/2026-06-21-weact-esp32c6-a-board-shape.md](wiki/raw/esp32/2026-06-21-weact-esp32c6-a-board-shape.md)
