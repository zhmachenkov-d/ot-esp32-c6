# OTC6 OpenTherm Wi‑Fi MQTT Gateway (firmware)

ESP-IDF 5.4 application for the **WeAct ESP32-C6 Mini**: OpenTherm master → Home Assistant via MQTT Discovery.

## Build / flash

```bash
cd firmware
idf.py set-target esp32c6
idf.py build
idf.py -p /dev/ttyACM0 flash monitor
# or from anywhere (restores cwd on exit):
./firmware/flash.sh
# PORT=/dev/ttyUSB0 ./firmware/flash.sh
```

OpenTherm stack: **sazanof/opentherm** (GPIO in=2 / out=3). C6 framing/IRQ must be confirmed on hardware (tasks T008); if it fails, Melnyk-port fallback is T008b.

## Host tests

```bash
cd firmware/tests/host
./run.sh
```

## Validation

End-to-end scenarios: [`specs/001-wifi-mqtt-opentherm/quickstart.md`](../specs/001-wifi-mqtt-opentherm/quickstart.md).

HIL checklists: `tests/hil/`.

## SoftAP commissioning

Unconfigured boot → join SoftAP `OTC6-XXXX` (**WPA2-PSK**). The SoftAP password (16-hex PSK) and one-time **Setup PIN** are logged on USB serial when SoftAP starts — see [`contracts/softap-provisioning.md`](../specs/001-wifi-mqtt-opentherm/contracts/softap-provisioning.md). A captive portal should open automatically after you join.

If it does not, open **http://192.168.4.1/** in the phone/laptop browser while connected to the SoftAP. Portal save requires the Setup PIN from serial (not shown in the HTML form page body for silent reuse).

Long-press GPIO9 ≥5 s clears Wi‑Fi/MQTT credentials and re-enters SoftAP mode (SoftAP PSK is retained for label/QR).
