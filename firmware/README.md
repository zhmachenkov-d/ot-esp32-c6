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

HIL checklists: `tests/hil/`.

## SoftAP commissioning

Unconfigured boot → join SoftAP `OTC6-XXXX` (**WPA2-PSK**). The SoftAP password (16-hex PSK) and one-time **Setup PIN** are logged on USB serial when SoftAP starts. A captive portal should open automatically after you join.

If it does not, open **http://192.168.4.1/** in the phone/laptop browser while connected to the SoftAP. Portal save requires the Setup PIN from serial (not shown in the HTML form page body for silent reuse).

Long-press GPIO9 ≥5 s clears Wi‑Fi/MQTT credentials and re-enters SoftAP mode (SoftAP PSK is retained for label/QR).

## OTA firmware updates

Dual-slot OTA (`ota_0` / `ota_1`, ~1.91 MiB each on 4 MiB flash after 64 KiB app alignment) with bootloader rollback. The device polls a GitHub Releases `manifest.json`, exposes a Home Assistant MQTT Discovery **update** entity, and downloads the cached HTTPS asset URL using the **public CA bundle** (not the MQTT broker CA in NVS).

### One-time USB partition migration

The partition table drops `factory` in favour of `otadata` + dual OTA slots. **Flash once over USB** after upgrading to an OTA-capable build so the new table is written:

```bash
cd firmware
idf.py -p /dev/ttyACM0 flash
# or: ./firmware/flash.sh
```

Subsequent updates can use HA Install (OTA). Do not skip this USB flash when moving from a factory-only image.

### GitHub Release contract

Each Release that should be offered OTA must include:

1. **`manifest.json`** — uploaded as a release asset (also reachable via `…/releases/latest/download/manifest.json`).
2. The firmware **`.bin`** named by that manifest’s `url` field (typically `otc6_gateway.bin`).

CI upload may be manual for v1. Example `manifest.json`:

```json
{
  "manifest_version": 1,
  "firmware_id": "otc6_gateway",
  "version": "0.2.0",
  "url": "https://github.com/zhmachenkov-d/ot-esp32-c6/releases/download/v0.2.0/otc6_gateway.bin",
  "sha256": "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef",
  "size": 1100000,
  "release_url": "https://github.com/zhmachenkov-d/ot-esp32-c6/releases/tag/v0.2.0",
  "title": "OpenTherm Gateway",
  "summary": "OTA support and bugfixes."
}
```

Required fields: `manifest_version` (must be `1`), `firmware_id` (`otc6_gateway`), `version` (semver `X.Y.Z`), `url` (`https://` only). Optional: `sha256` (64 lowercase hex), `size`, `release_url`, `title`, `summary`. Unknown keys are ignored.

Compile-time manifest URL: `https://github.com/zhmachenkov-d/ot-esp32-c6/releases/latest/download/manifest.json`.

### Home Assistant Install

After MQTT is up, the gateway publishes an **update** entity (`device_class: firmware`) with JSON on `otc6/<device_id>/update/state` (`installed_version`, `latest_version`, and during OTA `in_progress` / `update_percentage`, plus `release_url` when known). Install uses a fixed `payload_install` of `install` on `…/update/set` — the device owns the firmware URL from the cached manifest (Install does **not** carry a URL).

Manifest poll: every 12 h while STA+MQTT operational, plus on MQTT session ready, with a 1 h minimum gap. Polling is suppressed during SoftAP provisioning and while an OTA download is in flight.

### Rollback confirm window

After an OTA reboot, the new image stays **pending verify** until MQTT session ready (`s_mqtt_session_ready` after the link-up debounce — same path as discovery/subscribe). If that does not happen within **15 minutes**, the device restarts so the bootloader rolls back to the previous slot. SoftAP-only or a dead broker after OTA will therefore undo the update.

### App size gate

Production builds must keep the app image **strictly smaller than** OTA slot size − **256 KiB** (~1.91 MiB − 256 KiB). The firmware CMake build fails if the `.bin` exceeds that limit.
