# V10 OTA — HIL results

Date: 2026-09-04 (cases 1–2); 2026-09-05 (cases 3–6)  
Baseline on device: **0.2.0** (USB flash; includes GitHub redirect fix)  
Release under test: [v0.2.2](https://github.com/zhmachenkov-d/ot-esp32-c6/releases/tag/v0.2.2) (cases 1–2); [v0.2.3](https://github.com/zhmachenkov-d/ot-esp32-c6/releases/tag/v0.2.3) (cases 3–6)  
Manifest: `https://github.com/zhmachenkov-d/ot-esp32-c6/releases/latest/download/manifest.json`

| # | Case | Result | Notes |
|---|------|--------|-------|
| 1 | Happy path (HA Install → reboot → installed 0.2.2) | ✓ | Catalog wait ~3.5 min; GitHub 302 fix required; Install → reboot → 0.2.2. |
| 2 | Bad asset / TLS (Install fails, slot stays A) | ✓ | Temporary v0.2.3 manifest → missing `.bin`; serial `File not found(404)` / `ota begin failed: ESP_FAIL`; no reboot (stayed 0.2.2). |
| 3 | Power loss mid-write → last valid boots | ✓ | HA Install 0.2.3; serial `Starting OTA` / `Writing to <ota_0>`; USB hard-reset ~2 s later; rebooted **0.2.2** from `ota_1`. |
| 4 | Confirm timeout (~15 min) → rollback to A | ✗ / retry | OTA to 0.2.3 OK (`ota_0`). NVS erase via esptool reset while pending-verify → bootloader rolled back to **0.2.2** immediately (no `confirm timeout` log). Need retest: keep 0.2.3 running and block MQTT ~15 min (no chip reset). NVS restored. |
| 5 | SoftAP: Install does not run | ✓ | Erased NVS (HIL stand-in for long-press clear); `wifi=0 mqtt=0`; SoftAP `OTC6-dd40` + portal; no MQTT / no manifest poll. NVS restored afterward. |
| 6 | Manifest poll shows newer `latest_version` | ✓ | After v0.2.3 Release published; reboot → session ready → `manifest ok version=0.2.3 newer=1` (~3.5 min catalog wait). |

## Preconditions checklist

- [x] USB flash wrote `otadata` + `ota_0`/`ota_1` + 0.2.0 (`idf.py -p /dev/ttyACM0 flash`)
- [x] GitHub Release v0.2.1 with `manifest.json` + `otc6_gateway.bin`
- [x] Device on Wi‑Fi + MQTT (serial: STA `10.0.10.18`, `mqtt_ha: connected`)
- [x] HA shows **Firmware** update entity; case 1 completed at installed **0.2.2**
- [x] GitHub Release v0.2.3 with `manifest.json` + `otc6_gateway.bin` (HIL cases 3–6)
