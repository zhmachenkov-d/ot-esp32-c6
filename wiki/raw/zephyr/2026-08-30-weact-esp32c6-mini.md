# ESP32-C6-Mini — Zephyr Project Documentation

> Source: https://docs.zephyrproject.org/latest/boards/weact/esp32c6_mini/doc/index.html
> Collected: 2026-08-30
> Published: Unknown (Zephyr docs latest)

Board name: `weact_esp32c6_mini`  
Vendor: WeAct Studio  
Status: Not actively maintained  
Architecture: riscv  
SoC: esp32c6

## Overview

WeAct ESP32-C6 Mini is a compact development board based on ESP32-C6FH4 chip with integrated 4 MB flash. This board integrates complete Wi-Fi, Bluetooth LE, Zigbee, and Thread functions.

For more information, check WeAct ESP32-C6 Mini (https://github.com/WeActStudio/WeActStudio.ESP32C6-Mini).

## Hardware

The WeAct ESP32-C6 Mini is a compact board with the ESP32-C6FH4 chip directly mounted, featuring a 4 MB SPI flash. The board includes a USB Type-C connector, boot and reset buttons, and an RGB LED.

### ESP32-C6 Features

ESP32-C6 is Espressif’s first Wi-Fi 6 SoC integrating 2.4 GHz Wi-Fi 6, Bluetooth 5.3 (LE) and the 802.15.4 protocol. It consists of a high-performance (HP) 32-bit RISC-V processor (up to 160 MHz), and a low-power (LP) 32-bit RISC-V processor (up to 20 MHz). It has a 320KB ROM, a 512KB SRAM, and works with external flash.

ESP32-C6 includes the following features:

- 32-bit core RISC-V microcontroller with a clock speed of up to 160 MHz
- 400 KB of internal RAM
- WiFi 802.11 ax 2.4GHz; fully compatible with IEEE 802.11b/g/n
- Bluetooth LE: Bluetooth 5.3 certified
- Internal co-existence mechanism between Wi-Fi and Bluetooth to share the same antenna
- IEEE 802.15.4 (Zigbee and Thread)

Digital interfaces:

- 30x GPIOs (QFN40), or 22x GPIOs (QFN32)
- 2x UART
- 1x Low-power (LP) UART
- 1x General purpose SPI
- 1x I2C
- 1x Low-power (LP) I2C
- 1x I2S
- 1x Pulse counter
- 1x USB Serial/JTAG controller
- 1x TWAI® controller, compatible with ISO 11898-1 (CAN Specification 2.0)
- 1x SDIO 2.0 slave controller
- LED PWM controller, up to 6 channels
- 1x Motor control PWM (MCPWM)
- 1x Remote control peripheral
- 1x Parallel IO interface (PARLIO)
- General DMA controller (GDMA), with 3 transmit channels and 3 receive channels
- Event task matrix (ETM)

Analog interfaces:

- 1x 12-bit SAR ADCs, up to 7 channels
- 1x temperature sensor

Timers:

- 1x 52-bit system timer
- 1x 54-bit general-purpose timers
- 3x Watchdog timers
- 1x Analog watchdog timer

Low Power:

- Four power modes: Active, Modem-sleep, Light-sleep, Deep-sleep

Security:

- Secure boot
- Flash encryption
- 4-Kbit OTP, up to 1792 bits for users
- Cryptographic hardware acceleration: AES-128/256, ECC, HMAC, RSA, SHA, Digital signature, Hash
- Random number generator (RNG)

### Low-Power CPU (LP CORE)

The ESP32-C6 SoC has two RISC-V cores: the High-Performance Core (HP CORE) and the Low-Power Core (LP CORE). The LP Core features ultra low power consumption, an interrupt controller, a debug module and a system bus interface for memory and peripheral access.

The LP Core is in sleep mode by default. It has two application scenarios:

- Power insensitive scenario: When the HP Core is active, the LP Core can assist with speed- and efficiency-insensitive controls and computations.
- Power sensitive scenario: When the HP CPU is powered down, the LP Core can be woken up to handle external wake-up events.

Enable the LP Core with:

```
CONFIG_ESP32_ULP_COPROC_ENABLED=y
```

LP Core support is integrated with Sysbuild (System build).

For more information, see the ESP32-C6 Datasheet and ESP32-C6 Technical Reference Manual.

### Supported Features

#### weact_esp32c6_mini/esp32c6/hpcore target

On-target memory: 512 KiB of RAM, 4 MiB of Flash.

| Type | Location | Description | Compatible |
|------|----------|-------------|------------|
| CPU | on-chip | Espressif RISC-V CPU 1 | espressif,riscv |
| ADC | on-chip | ESP32 ADC 1 | espressif,esp32-adc |
| Bluetooth | on-chip | Bluetooth HCI for Espressif ESP32 1 | espressif,esp32-bt-hci |
| CAN | on-chip | ESP32 TWAI 2 | espressif,esp32-twai |
| Clock control | on-chip | ESP32 Clock Module 1 | espressif,esp32-clock |
| Counter | on-chip | ESP32 general-purpose timers 2 | espressif,esp32-timer |
| Counter | on-chip | ESP32 counters 2 | espressif,esp32-counter |
| Counter | on-chip | ESP32 RTC Main Timer 1 | espressif,esp32-rtc-timer |
| Cryptographic accelerator | on-chip | ESP32 SHA 1 | espressif,esp32-sha |
| Cryptographic accelerator | on-chip | ESP32 AES 1 | espressif,esp32-aes |
| DMA | on-chip | ESP32 GDMA 1 | espressif,esp32-gdma |
| Flash controller | on-chip | ESP32 flash controller 1 | espressif,esp32-flash-controller |
| GPIO & Headers | on-chip | ESP32 GPIO controller 1 | espressif,esp32-gpio |
| I2C | on-chip | ESP32 I2C 1 | espressif,esp32-i2c |
| I2S | on-chip | ESP32 I2S 1 | espressif,esp32-i2s |
| IEEE 802.15.4 | on-chip | Espressif IEEE 802.15.4 node 1 | espressif,esp32-ieee802154 |
| Input | on-board | Group of GPIO-bound input keys 1 | gpio-keys |
| Interrupt controller | on-chip | ESP32 Interrupt controller 1 | espressif,esp32-intc |
| LED strip | on-board | Worldsemi WS2812 LED strip, I2S binding 1 | worldsemi,ws2812-i2s |
| Mailbox | on-chip | ESP32 soft mailbox 1 | espressif,mbox-esp32 |
| MTD | on-chip | Flash node 1 | soc-nv-flash |
| Pin control | on-chip | ESP32 pin controller 1 | espressif,esp32-pinctrl |
| Pulse IO | on-chip | ESP32 RMT 1 | espressif,esp32-rmt |
| PWM | on-chip | ESP32 LEDC 1 | espressif,esp32-ledc |
| PWM | on-chip | ESP32 MCPWM 1 | espressif,esp32-mcpwm |
| RNG | on-chip | ESP32 TRNG 1 | espressif,esp32-trng |
| Sensors | on-chip | ESP32 internal temperature sensor 1 | espressif,esp32-temp |
| Sensors | on-chip | ESP32 Pulse Counter (PCNT) 1 | espressif,esp32-pcnt |
| Serial controller | on-chip | ESP32 UART 1 1 | espressif,esp32-uart |
| Serial controller | on-chip | ESP32 Low Power UART 1 | espressif,esp32-lpuart |
| Serial controller | on-chip | ESP32 USB Serial 1 | espressif,esp32-usb-serial |
| SPI | on-chip | ESP32 SPI controller 1 | espressif,esp32-spi |
| Timer | on-chip | ESP32 System Timer 1 | espressif,esp32-systimer |
| Watchdog | on-chip | ESP32 watchdog 1 1 | espressif,esp32-watchdog |
| Wi-Fi | on-chip | ESP32 SoC Wi-Fi 1 | espressif,esp32-wifi |

#### weact_esp32c6_mini/esp32c6/lpcore target

On-target memory: 15 KiB of RAM, 4 MiB of Flash.

| Type | Location | Description | Compatible |
|------|----------|-------------|------------|
| CPU | on-chip | Espressif RISC-V CPU 1 | espressif,riscv |
| Flash controller | on-chip | ESP32 flash controller 1 | espressif,esp32-flash-controller |
| GPIO & Headers | on-chip | ESP32 Low Power GPIO for LP Core 1 | espressif,esp32-lpgpio |
| Interrupt controller | on-chip | ESP32 Interrupt controller 1 | espressif,esp32-intc |
| Mailbox | on-chip | ESP32 soft mailbox 1 | espressif,mbox-esp32 |
| MTD | on-chip | Flash node 1 | soc-nv-flash |
| Serial controller | on-chip | ESP32 Low Power UART 1 | espressif,esp32-lpuart |
| SRAM | on-chip | Generic on-chip SRAM 1 | mmio-sram |

## System Requirements

### Binary Blobs

Espressif HAL requires RF binary blobs. Retrieve them with:

```
west blobs fetch hal_espressif
```

Recommended after `west update`.

## Programming and Debugging

Runners for `weact_esp32c6_mini`:

| | flash | debug | debugserver | rtt | attach |
|---|-------|-------|-------------|-----|--------|
| esp32 | yes (default) | | | | |
| openocd | yes | yes (default) | yes | yes | yes |

### Simple Boot

Default: single binary image, no 2nd stage bootloader. Does not provide security features or OTA updates.

### MCUboot Bootloader

Optional. Bootloader must be built and flashed at least once. Enable with:

```
CONFIG_BOOTLOADER_MCUBOOT=y
```

Options: Sysbuild or Manual build.

### Sysbuild

```
west build -b <board> --sysbuild samples/hello_world
```

Creates MCUboot and application images. Build directory structure:

```
build/
├── hello_world
│   └── zephyr
│       ├── zephyr.elf
│       └── zephyr.bin
├── mcuboot
│    └── zephyr
│       ├── zephyr.elf
│       └── zephyr.bin
└── domains.yaml
```

With `--sysbuild`, the bootloader is rebuilt and re-flashed on pristine builds.

### Manual Build

```
# From the root of the zephyr repository
west build -b <board> samples/hello_world
west flash
```

On USB Serial/JTAG targets, the chip may stay in download mode after `west flash`. Use:

```
west flash --reset-type watchdog-reset
```

### Faster Flashing

```
west flash --esp-skip-flashed
west flash --esp-diff
west flash --esp-no-progress
```

Monitor:

```
west espressif monitor
```

Expected output:

```
***** Booting Zephyr OS vx.x.x-xxx-gxxxxxxxxxxxx *****
Hello World! <board>
```

### Board variants using Snippets

Snippets under `snippets/espressif`:

| Snippet name | Description |
|--------------|-------------|
| espressif-flash-4M | 4MB flash |
| espressif-flash-8M | 8MB flash |
| espressif-flash-16M | 16MB flash |
| espressif-flash-32M | 32MB flash |
| espressif-flash-64M | 64MB flash |
| espressif-flash-128M | 128MB flash |
| espressif-psram-2M | 2MB PSRAM |
| espressif-psram-4M | 4MB PSRAM |
| espressif-psram-8M | 8MB PSRAM |
| espressif-psram-reloc | Relocate flash to PSRAM |
| espressif-psram-wifi | Wi-Fi buffers in PSRAM |

```
west build -b <board> -S espressif-flash-32M -S espressif-psram-4M samples/hello_world
```

Only applicable with compatible hardware. Board defaults apply when no snippet is used.

### OpenOCD Debugging

Espressif chips need a custom OpenOCD with ESP32 patches. See OpenOCD for ESP32 and JTAG debugging for ESP32.

Zephyr thread awareness (OpenOCD ESP32 v0.12.0-esp32-20251215 or later; `CONFIG_DEBUG_THREAD_INFO=y`):

- `info threads`
- thread names, priorities, states
- switch contexts; backtraces

```
west build -b <board> samples/hello_world -- -DCONFIG_DEBUG_THREAD_INFO=y -DOPENOCD=<path/to/bin/openocd> -DOPENOCD_DEFAULT_PATH=<path/to/openocd/share/openocd/scripts>
west debug
```

## References (from Zephyr page)

1. ESP32-C6 Datasheet — https://www.espressif.com/sites/default/files/documentation/esp32-c6_datasheet_en.pdf
2. ESP32-C6 Technical Reference Manual — https://www.espressif.com/sites/default/files/documentation/esp32-c6_technical_reference_manual_en.pdf
3. OpenOCD for ESP32 — https://github.com/espressif/openocd-esp32/releases
4. OpenOCD ESP32 v0.12.0-esp32-20251215 — https://github.com/espressif/openocd-esp32/releases
5. JTAG debugging for ESP32 — https://docs.espressif.com/projects/esp-idf/en/latest/esp32c6/api-guides/jtag-debugging/index.html
6. WeAct ESP32-C6 Mini — https://github.com/WeActStudio/WeActStudio.ESP32C6-Mini
