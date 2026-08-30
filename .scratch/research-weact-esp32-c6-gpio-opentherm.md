# Safe GPIOs for OpenTherm TTL on WeAct ESP32-C6-A / MINI

## Findings

- WeAct publishes **two different boards**: **ESP32-C6-A** (ESP32-C6-WROOM-1, DevKitC-1 pin-compatible) and **ESP32C6-MINI** (ESP32-C6-FH4 / “ESP32C6-N4 MINI”); they are not the same product or pinout. — https://github.com/WeActStudio/WeActStudio.ESP32-C6-A ; https://github.com/WeActStudio/WeActStudio.ESP32C6-MINI
- OpenTherm master firmware needs **two GPIOs**: interrupt-capable **in** (adapter RX → MCU) and **out** (MCU → adapter TX); MCU pins cannot drive the OT bus directly. — `wiki/raw/opentherm/2024-02-08-melnyk-opentherm-library.md` ; `wiki/raw/opentherm/2022-06-xyzroe-opentherm-ttl-adapter.md`
- xyzroe OpenTherm-TTL adapter TTL header is **GND / RX / TX / VCC**; VCC may be **3.3 V or 5 V** shared with the MCU — use **3.3 V** with ESP32-C6. — `wiki/raw/opentherm/2022-06-xyzroe-opentherm-ttl-adapter.md`
- ESP32-C6 digital I/O is powered from **VDDPST** rails at **3.0–3.6 V** (typical **3.3 V**); DC VIH/VIL are referenced to that VDD. — `wiki/raw/esp32/esp32-c6-datasheet-en.md` (Tables 4-2 / 4-4)
- On ESP32-C6, peripheral signals (including GPIO interrupts) can be routed through the **GPIO matrix to any GPIO**. — `wiki/raw/esp32/2023-esp32-c6-technical-reference-manual-en.md` (IO MUX and GPIO Matrix)
- **Strapping pins** (must be at valid levels at reset; usable as GPIO afterward): **GPIO4, GPIO5** (SDIO sample/drive edge via MTMS/MTDI), **GPIO8, GPIO9** (boot mode), **GPIO15** (JTAG source select). Boot table: SPI boot needs GPIO9=1; download needs GPIO8=1, GPIO9=0; avoid GPIO8=0 with GPIO9=0. — `wiki/raw/esp32/esp32-c6-datasheet-en.md` §2.6 ; https://docs.espressif.com/projects/esp-idf/en/v5.4/esp32c6/api-reference/peripherals/gpio.html
- **GPIO12 = USB_D−**, **GPIO13 = USB_D+** (USB Serial/JTAG). By default they are USB-JTAG; using them as GPIO **disables USB-JTAG** and requires reconfiguration. — `wiki/raw/esp32/esp32-c6-datasheet-en.md` §2.3.3–2.3.4 ; https://docs.espressif.com/projects/esp-idf/en/v5.4/esp32c6/api-reference/peripherals/gpio.html
- Prefer **non-highlighted** GPIOs first (avoid flash, strapping, USB_D±, pad JTAG, UART0 when those functions matter). — `wiki/raw/esp32/esp32-c6-datasheet-en.md` §2.3.4
- **SPI flash**: GPIO24–GPIO30 are for SPI0/1 flash and are **not recommended** for other use; on **in-package flash (FH4 / MINI)** they are dedicated and **GPIO10–GPIO11 are not bonded out**. On **external-flash (WROOM / C6-A)** **GPIO14 is not led out**. — https://docs.espressif.com/projects/esp-idf/en/v5.4/esp32c6/api-reference/peripherals/gpio.html ; `wiki/raw/esp32/2023-esp32-c6-technical-reference-manual-en.md`
- **WeAct ESP32-C6-A**: on-board **WS2812 on IO8**, **user/BOOT path on IO9**, UART0 via USB-UART bridge on **U0TXD/U0RXD**, and a **separate ESP USB** port on **IO12/IO13**; headers H1/H2 break out DevKitC-1-style pins including **IO12 and IO13**. — `wiki/raw/esp32/2026-06-21-weact-esp32-c6-a-schematic.md` ; https://github.com/WeActStudio/WeActStudio.ESP32-C6-A/blob/main/Hardware/ESP32_C6_A_Sch.pdf
- Official **ESP32-C6-DevKitC-1** (C6-A is P2P with it) documents J3 pins **13 = GPIO13/USB_D+**, **14 = GPIO12/USB_D−**, plus UART TX/RX and GPIO8 RGB. — https://docs.espressif.com/projects/espressif-esp-dev-kits/en/latest/esp32c6/esp32-c6-devkitc-1/user_guide.html
- On **C6-A**, **GPIO12 (OT in) / GPIO13 (OT out)** are **electrically usable** for OpenTherm if firmware treats them as GPIO (USB-JTAG off) and the **ESP USB** Type-C is left unused; programming/logs can stay on the **USB-UART** port. This matches this repo’s documented default. — `wiki/raw/esp32/2026-06-21-weact-esp32-c6-a-schematic.md` ; `knowledge/bridge/opentherm-gpio-wiring.md` ; https://docs.espressif.com/projects/esp-idf/en/v5.4/esp32c6/api-reference/peripherals/gpio.html
- **WeAct ESP32C6-MINI**: schematic uses **ESP32-C6FH4**; **IO12/IO13** are wired to the **USB-C D−/D+** (only onboard USB); silk marks **RGB@IO8**, **BOOT** near **IO9**, and headers expose **0–9, 12–15, TX, RX, 18–23**. — https://github.com/WeActStudio/WeActStudio.ESP32C6-MINI/blob/main/Hardware/ESP32_C6_MINI_Sch.pdf ; https://github.com/WeActStudio/WeActStudio.ESP32C6-MINI/blob/main/Hardware/ESP32C6-MINI%20Board%20Shape%20%E5%A4%96%E5%BD%A2.pdf
- On **MINI**, **GPIO12/GPIO13 are a poor OpenTherm choice**: reclaiming them as GPIO removes the board’s **only** USB Serial/JTAG path (recovery then needs download boot via GPIO9 + external UART). — https://github.com/WeActStudio/WeActStudio.ESP32C6-MINI/blob/main/Hardware/ESP32_C6_MINI_Sch.pdf ; https://docs.espressif.com/projects/esp-idf/en/latest/esp32c6/api-guides/usb-serial-jtag-console.html
- **Avoid for OT on both boards**: **GPIO8** (RGB + strap), **GPIO9** (BOOT + strap), **TX/RX (U0TXD/U0RXD)** (console/download), flash pins, and (unless USB is intentionally sacrificed) **GPIO12/13**. Prefer not to hang OT on **GPIO4/5/15** during reset unless strap levels are guaranteed. — sources above
- **Safe / recommended OT pairs** (header-exposed, interrupt-capable, no USB/LED/UART0/flash conflict): e.g. **GPIO2 (in) + GPIO3 (out)**, **GPIO18 + GPIO19**, or **GPIO22 + GPIO23** (works on both C6-A and MINI). On **C6-A only**, **GPIO10/11** are also free of those conflicts; on **MINI only**, **GPIO14** is available (not on WROOM). — https://docs.espressif.com/projects/esp-idf/en/v5.4/esp32c6/api-reference/peripherals/gpio.html ; MINI/C6-A schematics/board shapes cited above
- **Bottom line**: Prefer **GPIO2/GPIO3** (or **18/19**, **22/23**) for OpenTherm TTL on either board. Use project default **GPIO12 in / GPIO13 out** only on **ESP32-C6-A** when ESP USB is unused; **do not prefer 12/13 on ESP32C6-MINI**.

## Sources

- https://github.com/WeActStudio/WeActStudio.ESP32-C6-A — WeAct ESP32-C6-A board repo (README: DevKitC-1 P2P)
- https://github.com/WeActStudio/WeActStudio.ESP32-C6-A/blob/main/Hardware/ESP32_C6_A_Sch.pdf — C6-A schematic (USB-UART + ESP USB on IO12/13, IO8 LED, IO9 button)
- `wiki/raw/esp32/2026-06-21-weact-esp32-c6-a-schematic.md` — transcribed C6-A schematic notes in this repo
- https://github.com/WeActStudio/WeActStudio.ESP32C6-MINI — WeAct ESP32C6-MINI board repo (ESP32-C6-FH4)
- https://github.com/WeActStudio/WeActStudio.ESP32C6-MINI/blob/main/Hardware/ESP32_C6_MINI_Sch.pdf — MINI schematic (FH4, USB on IO12/13)
- https://github.com/WeActStudio/WeActStudio.ESP32C6-MINI/blob/main/Hardware/ESP32C6-MINI%20Board%20Shape%20%E5%A4%96%E5%BD%A2.pdf — MINI mechanical pin silk
- https://docs.espressif.com/projects/espressif-esp-dev-kits/en/latest/esp32c6/esp32-c6-devkitc-1/user_guide.html — ESP32-C6-DevKitC-1 pin headers (GPIO12/13 = USB_D±)
- `wiki/raw/esp32/esp32-c6-datasheet-en.md` — ESP32-C6 Series Datasheet (strapping, USB_D±, IO voltage, pin restrictions)
- `wiki/raw/esp32/2023-esp32-c6-hardware-design-guidelines-en.md` — ESP32-C6 Hardware Design Guidelines (boot straps, USB on GPIO12/13)
- `wiki/raw/esp32/2023-esp32-c6-technical-reference-manual-en.md` — ESP32-C6 TRM (GPIO matrix; FH4 pin availability)
- https://docs.espressif.com/projects/esp-idf/en/v5.4/esp32c6/api-reference/peripherals/gpio.html — ESP-IDF 5.4 GPIO summary (USB-JTAG on 12/13, strapping, flash)
- https://docs.espressif.com/projects/esp-idf/en/latest/esp32c6/api-guides/usb-serial-jtag-console.html — USB Serial/JTAG console / pin-reconfig recovery notes
- `wiki/raw/opentherm/2022-06-xyzroe-opentherm-ttl-adapter.md` — xyzroe OpenTherm-TTL adapter pinout and 3.3/5 V VCC
- `wiki/raw/opentherm/2024-02-08-melnyk-opentherm-library.md` — Melnyk library: interrupt in + out GPIO requirement
- `knowledge/bridge/opentherm-gpio-wiring.md` — this project’s default CONFIG OT GPIO12/13 on WeAct ESP32-C6-A
