# ESP-IDF OpenTherm (sazanof/opentherm)

> Source: https://github.com/sazanof/esp-idf-opentherm
> Collected: 2026-08-29
> Published: 2024 (copyright); component version 1.0.7 as of 2026-04-14

## README

# Installation

`idf.py add-dependency "sazanof/opentherm^1.0.3"`

| Supported Targets | ESP32 | ESP32-C2 | ESP32-C3 | ESP32-C6 | ESP32-H2 | ESP32-S2 | ESP32-S3 |
| ----------------- | ----- | -------- | -------- | -------- | -------- | -------- | -------- |
|                   |   ✓   |    ?     |    ?     |     ?    |     ?    |     ?    |     ?    |

# ESP-IDF Opentherm

This component provides an implementation of Opentherm protocol with ESP-IDF. Tested on ESP-IDF > 5 version.

## How to Use Example

Before project configuration and build, be sure to set the correct chip target using `idf.py set-target <chip_name>`.

### Hardware Required

* A development board (e.g., ESP32-S3-DevKitC, ESP32-C6-DevKitC etc.)
* A USB cable for Power supply and programming
* An opentherm adapter

### Configure the Project

Open the project configuration menu (`idf.py menuconfig`).

In the `OpenTherm Configuration` menu:

* Select GPIO in pin
* Select GPIO out pin

### Build and Flash

Run `idf.py -p PORT flash monitor` to build, flash and monitor the project.

## Example Output

```text
I (9369) OT: ====== OPENTHERM =====
I (9369) OT: Free heap size before: 293684
I (9369) OT: Central Heating: OFF
I (9369) OT: Hot Water: OFF
I (9369) OT: Flame: OFF
I (9379) OT: Fault: NO
I (9659) OT: Set CH Temp to: 60
I (9929) OT: Set DHW Temp to: 59
I (10199) OT: DHW Temp: 0.0
I (10469) OT: CH Temp: 44.3
I (10739) OT: Slave OT Version: 0.0
I (11009) OT: Slave Version: C07FA308
I (11279) OT: Slave OT Version: 3.0
I (11279) OT: Free heap size after: 293684
I (11279) OT: ====== OPENTHERM =====
```

## idf_component.yml

```
version: "1.0.7"
description: "Opentherm library for ESP-IDF framework"
url: "https://github.com/sazanof/esp-idf-opentherm"
maintainer: "Mikhail Sazanof <m@sazanof.ru>"
license: "MIT"
dependencies:
  idf:
    version: ">=5.2"
```

## Repository layout

- `opentherm.c` — core Manchester framing / request helpers
- `include/opentherm.h` — enums, frame builders, high-level getters/setters
- `include/opentherm_struct.h` — fault flags, slave config, min/max helpers
- `examples/main/main.c` — FreeRTOS control-task demo
- `CMakeLists.txt` — `REQUIRES driver esp_timer`
- MIT license

Component registry name: `sazanof/opentherm` (add via `idf.py add-dependency`).

## Example usage (excerpt from examples/main/main.c)

```c
#define GPIO_OT_IN 22
#define GPIO_OT_OUT 23

esp_ot_init(GPIO_OT_IN, GPIO_OT_OUT, false, esp_ot_process_response_callback);

// In a FreeRTOS task (~1 s loop):
unsigned long status = esp_ot_set_boiler_status(false, true, false, false, false);
esp_ot_set_boiler_temperature(60);
esp_ot_set_dhw_setpoint(59);
float chTemp = esp_ot_get_boiler_temperature();
float dhwTemp = esp_ot_get_dhw_temperature();
```

## opentherm.h — key API (excerpt)

**open_therm_response_status_t:** OT_STATUS_NONE, OT_STATUS_SUCCESS, OT_STATUS_INVALID, OT_STATUS_TIMEOUT

**open_therm_message_type_t:** OT_READ_DATA, OT_WRITE_DATA, OT_INVALID_DATA, OT_RESERVED, OT_READ_ACK, OT_WRITE_ACK, OT_DATA_INVALID, OT_UNKNOWN_DATA_ID

**OpenThermMessageID (selected):** MSG_ID_STATUS=0, MSG_ID_T_SET=1, MSG_ID_S_CONFIG_S_MEMEBER_ID_CODE=3, MSG_ID_ASF_FLAGS=5, MSG_ID_REL_MOD_LEVEL=17, MSG_ID_CH_PRESSURE=18, MSG_ID_TR=24, MSG_ID_TBOILER=25, MSG_ID_TDHW=26, MSG_ID_TOUTSIDE=27, MSG_ID_TRET=28, MSG_ID_TDHW_SET=56, MSG_ID_MAX_TSET=57, MSG_ID_OPENTHERM_VERSION_SLAVE=125

**Init / framing:** `esp_ot_init(pin_in, pin_out, is_slave, response_callback)`; `esp_ot_send_request` / `esp_ot_send_request_async`; `esp_ot_build_request`; `esp_ot_handle_interrupt`; `esp_ot_get_last_response_status`

**High-level:** `esp_ot_set_boiler_status(ch, dhw, cooling, otc, ch2)`; `esp_ot_set_boiler_temperature`; `esp_ot_get_boiler_temperature`; `esp_ot_set_dhw_setpoint`; `esp_ot_get_dhw_temperature`; `esp_ot_get_return_temperature`; `esp_ot_get_modulation`; `esp_ot_get_pressure`; `esp_ot_get_slave_ot_version`; status bit helpers `esp_ot_is_flame_on`, `esp_ot_is_fault`, etc.

Native ESP-IDF (no Arduino core): uses `driver/gpio`, `esp_timer`, FreeRTOS critical sections / IRAM interrupt path.
