---
type: Hardware
title: WeAct ESP32-C6 Mini (Zephyr)
description: Project default board — ESP32-C6FH4 Mini; Zephyr weact_esp32c6_mini; ESP-IDF target esp32c6.
resource: https://docs.zephyrproject.org/latest/boards/weact/esp32c6_mini/doc/index.html
tags: [esp32, board, weact, zephyr]
timestamp: 2026-08-30T00:00:00Z
raw:
  - wiki/raw/zephyr/2026-08-30-weact-esp32c6-mini.md
---

Zephyr board target **`weact_esp32c6_mini`** for the [WeAct ESP32-C6 Mini](https://github.com/WeActStudio/WeActStudio.ESP32C6-Mini): compact board with **ESP32-C6FH4** chip-down (not a WROOM module), **4 MB** SPI flash, USB Type-C, boot/reset, RGB LED. Integrates Wi-Fi 6, Bluetooth LE, Zigbee, and Thread.

**Project default hardware** for this repo’s ESP-IDF firmware (`idf.py set-target esp32c6`). Distinct from [WeAct ESP32-C6-A](/esp32/weact-esp32-c6-a.md) (WROOM module, DevKitC-1 pinout). SoC details: [ESP32-C6 Datasheet](/esp32/esp32-c6-datasheet.md), [Technical Reference Manual](/esp32/esp32-c6-technical-reference-manual.md). OpenTherm adapter defaults: [OpenTherm GPIO Wiring](/bridge/opentherm-gpio-wiring.md) (GPIO2 in / GPIO3 out — not USB GPIO12/13).

Vendor status in Zephyr docs: **not actively maintained**. Architecture: RISC-V / `esp32c6`.

## On-board features (ESP-IDF relevance)

| Function | GPIO / signal | Notes |
|----------|---------------|-------|
| WS2812 RGB LED | IO8 | Avoid for OT |
| BOOT / user button | IO9 | SoftAP re-provision long-press |
| Native USB | GPIO12/13 | Only USB Serial/JTAG path — keep free for flash/console |
| UART0 | TX / RX | Header silk |

## Targets

| Target | RAM | Flash |
|--------|-----|-------|
| `weact_esp32c6_mini/esp32c6/hpcore` | 512 KiB | 4 MiB |
| `weact_esp32c6_mini/esp32c6/lpcore` | 15 KiB | 4 MiB |

On-board Zephyr features include `gpio-keys` and WS2812 via `worldsemi,ws2812-i2s`. HP core exposes the usual ESP32-C6 peripherals (Wi-Fi, BT HCI, 802.15.4, UART/USB-serial, TWAI, etc.).

## LP Core

Enable with `CONFIG_ESP32_ULP_COPROC_ENABLED=y` (Sysbuild-integrated). LP assists HP when HP is active, or handles wake events when HP is powered down.

## Prerequisites

```bash
west blobs fetch hal_espressif   # after west update
```

## Programming

| Runner | flash | debug |
|--------|-------|-------|
| esp32 | default | — |
| openocd | yes | default |

**Simple boot** (default): single image, no 2nd-stage bootloader; no secure boot / OTA.

**MCUboot**: set `CONFIG_BOOTLOADER_MCUBOOT=y`; flash bootloader at least once.

```bash
west build -b weact_esp32c6_mini --sysbuild samples/hello_world
west build -b weact_esp32c6_mini samples/hello_world
west flash
west flash --reset-type watchdog-reset   # if stuck in download mode after USB Serial/JTAG flash
west espressif monitor
```

Faster flash options: `--esp-skip-flashed`, `--esp-diff`, `--esp-no-progress`. Flash/PSRAM size variants via `-S espressif-flash-*` / `-S espressif-psram-*` snippets.

## Debugging

Custom Espressif OpenOCD required. Thread awareness needs OpenOCD ESP32 ≥ v0.12.0-esp32-20251215 and `CONFIG_DEBUG_THREAD_INFO=y`.

# Citations

[1] [wiki/raw/zephyr/2026-08-30-weact-esp32c6-mini.md](wiki/raw/zephyr/2026-08-30-weact-esp32c6-mini.md)
[2] [ESP32-C6-Mini — Zephyr Project Documentation](https://docs.zephyrproject.org/latest/boards/weact/esp32c6_mini/doc/index.html)
