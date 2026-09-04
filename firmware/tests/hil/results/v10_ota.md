# V10 OTA — HIL results

Date: 2026-09-04  
Baseline on device: **0.2.0** (USB flash; includes GitHub redirect fix)  
Release under test: [v0.2.2](https://github.com/zhmachenkov-d/ot-esp32-c6/releases/tag/v0.2.2)  
Manifest: `https://github.com/zhmachenkov-d/ot-esp32-c6/releases/latest/download/manifest.json`

| # | Case | Result | Notes |
|---|------|--------|-------|
| 1 | Happy path (HA Install → reboot → installed 0.2.2) | ✓ | Catalog wait ~3.5 min; GitHub 302 fix required; Install → reboot → 0.2.2. |
| 2 | Bad asset / TLS (Install fails, slot stays A) | ☐ | |
| 3 | Power loss mid-write → last valid boots | ☐ | |
| 4 | Confirm timeout (~15 min) → rollback to A | ☐ | |
| 5 | SoftAP: Install does not run | ☐ | |
| 6 | Manifest poll shows newer `latest_version` | ☐ | |

## Preconditions checklist

- [x] USB flash wrote `otadata` + `ota_0`/`ota_1` + 0.2.0 (`idf.py -p /dev/ttyACM0 flash`)
- [x] GitHub Release v0.2.1 with `manifest.json` + `otc6_gateway.bin`
- [x] Device on Wi‑Fi + MQTT (serial: STA `10.0.10.18`, `mqtt_ha: connected`)
- [x] HA shows **Firmware** update entity; case 1 completed at installed **0.2.2**
