<!-- bmad:context -->
<!-- Verified 2026-09-09 against 7a664dcab5f3. Managed by bmad-project-context; edits inside this block are replaced on refresh. Keep anything you want preserved outside the markers. -->

## ot-esp32-c6

OpenTherm Wi‑Fi MQTT gateway (OTC6) for the WeAct ESP32-C6 Mini. ESP-IDF ≥5.4 firmware lives in `firmware/`; compiled domain knowledge in `knowledge/`; ADRs in `docs/adr/` (create the tree on first ADR). BMAD planning output goes under `_bmad-output/`.

## Policy

- Only long-lived branch is `main` (always releasable). Short-lived `feature/*`, `fix/*`, `chore/*`, `hotfix/*` from `main`; update them with `git rebase`, never `git merge` from `main`.
- Merge to `main` only via PR: squash-and-merge; the squash commit must be Conventional Commits (`type(scope): description`). Prefer 1 approval (2 if the PR touches bootloader/NVS/OTA paths). Tech Lead merges.
- Release intent on `main`: SemVer from Conventional Commits, tag `vX.Y.Z`, GitHub Release with OTA `manifest.json` + firmware `.bin` (see `firmware/README.md`). Do not invent release branches.
- Never hardcode secrets (tokens, passwords, keys) in source; use GitHub Secrets, local `.env` (untracked), or device NVS filled by SoftAP commissioning.
- Architectural changes: add an ADR under `docs/adr/` before implementing.

## Where things are

- Firmware app and config: `firmware/` (`main/`, `partitions.csv`, `sdkconfig.defaults`); flash helper `firmware/flash.sh`
- Host unit tests: `firmware/tests/host/` (`./run.sh`); HIL checklists: `firmware/tests/hil/`
- Domain knowledge (OKF): `knowledge/`; immutable raw sources: `wiki/raw/` — maintain with `okf` (`tools/okf/`)
- ADRs: `docs/adr/`

## Running and verifying

- Tooling: ESP-IDF ≥5.4 (lockfile uses 5.4.4), target `esp32c6`; Python ≥3.11 for `okf`.
- Build/flash from `firmware/`: `idf.py set-target esp32c6 && idf.py build`, then `idf.py -p /dev/ttyACM0 flash monitor` or `./firmware/flash.sh` (`PORT=…` if needed). First dual-OTA image after a partition-table change must be USB-flashed once.
- Host tests: `cd firmware/tests/host && ./run.sh` (needs `IDF_PATH`).
- Knowledge: `okf index|validate|lint|search knowledge/` — not bare `pytest` for that tree.
- App SemVer is `APP_FW_VERSION` in `firmware/main/app_config.h`, not a root `VERSION` file.

## Conventions that differ from defaults

- Commits and squash subjects: Conventional Commits; scope when a subsystem is clear (`wifi`, `mqtt`, `nvs`, `ota`, …).
- MQTT credentials and Wi‑Fi come from SoftAP → NVS (optional broker CA PEM). Do not assume client-certificate MQTT, encrypted NVS, Secure Boot, or cryptographic OTA image signing — those are not enabled; OTA integrity today is HTTPS (public CA bundle) plus optional manifest SHA-256 and dual-slot rollback.

## Known pitfalls

- Do not invent Zigbee features, stacks, or product paths — this project is OpenTherm + Wi‑Fi + MQTT. Treat `knowledge/zigbee/` as unused reference material unless the user explicitly asks for it.

<!-- /bmad:context -->
