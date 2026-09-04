<!-- bmad:context -->
<!-- Verified 2026-09-04 against ecbda73. Managed by bmad-project-context; edits inside this block are replaced on refresh. Keep anything you want preserved outside the markers. -->

## ot-esp32-c6

OpenTherm Wi‑Fi MQTT gateway for WeAct ESP32-C6 Mini (ESP-IDF ≥5.4 → Home Assistant MQTT Discovery). Firmware lives in `firmware/`; agent knowledge is the OKF bundle under `knowledge/` compiled from `wiki/raw/`. Cursor skills under `.cursor/skills/` (OKF, git-workflow, domain-modeling). Behavioral edit rules: `.cursor/rules/karpathy-guidelines.mdc`.

## Policy

- Integrate via feature branches and PRs into `main`; do not push commits straight to `main`.
- Never commit `.env` or `.certs/` (gitignored local secrets; do not `git add -f` them).
- In `firmware/`: never hand-edit `managed_components/`, `sdkconfig`, or `dependencies.lock` — change `sdkconfig.defaults` / `main/idf_component.yml` (and project-level `idf_component.yml` mirror) instead.
- Treat `wiki/raw/` as immutable source; edit compiled concepts in `knowledge/` (ingest may add new raw files only when fetching sources).

## Where things are

- Firmware entry: `firmware/main/main.c`; pins/config: `firmware/main/app_config.h`.
- Host unit tests: `firmware/tests/host/`; HIL checklists: `firmware/tests/hil/`.
- Knowledge work: follow `.cursor/skills/okf-knowledge/SKILL.md` — search with `okf`, do not load the whole `knowledge/` tree.
- Commit/PR wording: `.cursor/skills/git-workflow/SKILL.md` (Conventional Commits).

## Running and verifying

- Build/flash from `firmware/` (`idf.py set-target esp32c6`, then `build` / `flash monitor`). Default serial `/dev/ttyACM0`; from repo root use `./firmware/flash.sh` (always flash+monitor; override with `PORT=`).
- Host tests: `cd firmware/tests/host && ./run.sh` — requires `IDF_PATH` (pulls Unity from ESP-IDF).
- OKF CLI: `uv tool install -e tools/okf` (devcontainer post-create does this). After writes under `knowledge/`: `okf index knowledge/` then `okf validate knowledge/`. Python ≥3.11.

## Conventions that differ from defaults

- Firmware C follows ESP-IDF (`-std=gnu17`). Keep host tests at C17 (`CMAKE_C_STANDARD 17` in `firmware/tests/host/CMakeLists.txt`) so they match.
- App sources under `firmware/main/` are C only; do not introduce C++ there without an explicit decision.

<!-- /bmad:context -->
