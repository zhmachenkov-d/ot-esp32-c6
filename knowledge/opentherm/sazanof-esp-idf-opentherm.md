---
type: Reference
title: Sazanof ESP-IDF OpenTherm
description: Native ESP-IDF OpenTherm master component (sazanof/opentherm); GPIO interrupt + esp_timer framing.
resource: https://github.com/sazanof/esp-idf-opentherm
tags: [opentherm, library, esp-idf, esp32]
timestamp: 2026-08-29T00:00:00Z
raw:
  - wiki/raw/opentherm/2026-04-14-sazanof-esp-idf-opentherm.md
---

[sazanof/esp-idf-opentherm](https://github.com/sazanof/esp-idf-opentherm) is a **native ESP-IDF** OpenTherm master component (`sazanof/opentherm` **1.0.7**, MIT, IDF **≥ 5.2**). Unlike the Arduino-oriented [Melnyk OpenTherm Library](/opentherm/melnyk-opentherm-library.md), it depends only on `driver` and `esp_timer` — no Arduino core.

Install: `idf.py add-dependency "sazanof/opentherm^1.0.3"`. README marks ESP32 as tested; ESP32-C6 and other targets are listed as unconfirmed.

## Requirements

| Item | Detail |
|------|--------|
| Protocol | OpenTherm master framing per [OpenTherm Protocol](/opentherm/opentherm-protocol.md) / [Frame Format](/opentherm/opentherm-frame-format.md) |
| Hardware | OpenTherm adapter — MCU GPIO cannot drive the bus directly |
| Pins | 2 GPIOs: interrupt-capable **in** and **out** (example uses **22 / 23**; this project defaults to **12 / 13** — see [OpenTherm GPIO Wiring](/bridge/opentherm-gpio-wiring.md)) |
| Timing | Example polls on a ~1 s FreeRTOS task |

## API mapping

| Function | Maps to |
|----------|---------|
| `esp_ot_set_boiler_status(ch, dhw, cooling, otc, ch2)` | [Data ID 0 Status](/opentherm/data-id-0-status.md) |
| `esp_ot_set_boiler_temperature(°C)` | [Data ID 1 TSet](/opentherm/data-id-1-tset.md) |
| `esp_ot_get_boiler_temperature()` | [Data ID 25 Tboiler](/opentherm/data-id-25-tboiler.md) |

Also exposes DHW setpoint/temperature, return/outside/modulation/pressure helpers, fault/ASF flags, and `OpenThermMessageID` (`MSG_ID_*`) covering spec IDs through ventilation/solar ranges. Init: `esp_ot_init(pin_in, pin_out, is_slave, response_callback)`.

# Citations

[1] [wiki/raw/opentherm/2026-04-14-sazanof-esp-idf-opentherm.md](wiki/raw/opentherm/2026-04-14-sazanof-esp-idf-opentherm.md)
[2] [github.com/sazanof/esp-idf-opentherm](https://github.com/sazanof/esp-idf-opentherm)
