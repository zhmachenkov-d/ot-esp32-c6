# ot-esp32-c6

**OTC6** — OpenTherm Wi‑Fi MQTT gateway for the [WeAct ESP32-C6 Mini](https://github.com/WeActStudio). Bridges an OpenTherm boiler to Home Assistant via MQTT Discovery.

## What it does

- Talks OpenTherm as master (GPIO in=2 / out=3, sazanof/opentherm stack)
- SoftAP captive portal (`OTC6-XXXX`) commissions Wi‑Fi and MQTT into NVS
- Publishes boiler state and HA Discovery entities over MQTT
- Dual-slot OTA from GitHub Releases (`manifest.json` + `otc6_gateway.bin`), with bootloader rollback

## Layout

| Path | Role |
|------|------|
| `firmware/` | ESP-IDF ≥5.4 app (`main/`, partitions, flash helper) |
| `firmware/tests/host/` | Host unit tests (`./run.sh`) |
| `firmware/tests/hil/` | Hardware-in-the-loop checklists |
| `knowledge/` | Compiled domain knowledge (OKF) |
| `wiki/raw/` | Immutable source docs for OKF |
| `docs/adr/` | Architecture decision records |
| `_bmad-output/` | Planning / specs output |

## Quick start

Needs ESP-IDF ≥5.4 and target `esp32c6`.

```bash
cd firmware
idf.py set-target esp32c6
idf.py build
idf.py -p /dev/ttyACM0 flash monitor
# or: ./firmware/flash.sh   # PORT=/dev/ttyUSB0 if needed
```

Host tests:

```bash
cd firmware/tests/host && ./run.sh   # needs IDF_PATH
```

After first join to SoftAP, open the captive portal (or http://192.168.4.1/). Setup PIN and SoftAP PSK are printed on USB serial. Long-press GPIO9 ≥5 s clears Wi‑Fi/MQTT credentials.

## Releases / OTA

Tag a SemVer `vX.Y.Z` on `main` that is **strictly newer** than `APP_FW_VERSION` in `firmware/main/app_config.h`. GitHub Actions builds and publishes the OTA assets. First dual-OTA image after a partition-table change must be USB-flashed once.

Full build, commissioning, OTA contract, and LED notes: **[firmware/README.md](firmware/README.md)**.

## Contributing

- Long-lived branch: `main` only. Short-lived `feature/*`, `fix/*`, `chore/*`, `hotfix/*`; rebase onto `main`, do not merge from it.
- PRs: squash-and-merge with Conventional Commits subject.
- No secrets in source — use SoftAP → NVS, GitHub Secrets, or local `.env`.
- Architectural changes: ADR under `docs/adr/` first.
