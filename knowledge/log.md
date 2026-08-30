# Directory Update Log

## 2026-08-30
* **Update**: Project default board → [WeAct ESP32-C6 Mini](/zephyr/weact-esp32c6-mini.md); [OpenTherm GPIO Wiring](/bridge/opentherm-gpio-wiring.md) defaults GPIO2 in / GPIO3 out; [WeAct ESP32-C6-A](/esp32/weact-esp32-c6-a.md) no longer default.
* **Creation**: Added [WeAct ESP32-C6 Mini (Zephyr)](/zephyr/weact-esp32c6-mini.md) from `wiki/raw/zephyr/2026-08-30-weact-esp32c6-mini.md` (Zephyr board docs).
* **Update**: Linked [WeAct ESP32-C6-A](/esp32/weact-esp32-c6-a.md) and [ESP32](/esp32/) index to the Mini board; added [Zephyr](/zephyr/) directory.

## 2026-08-29
* **Creation**: Added [xyzroe OpenTherm-TTL Adapter](/opentherm/xyzroe-opentherm-ttl-adapter.md) from `wiki/raw/opentherm/2022-06-xyzroe-opentherm-ttl-adapter.md` (+ schematic PNG).
* **Update**: Linked [OpenTherm GPIO Wiring](/bridge/opentherm-gpio-wiring.md) and [OpenTherm Protocol](/opentherm/opentherm-protocol.md) to the xyzroe adapter.

## 2026-08-29
* **Creation**: Added [Sazanof ESP-IDF OpenTherm](/opentherm/sazanof-esp-idf-opentherm.md) from `wiki/raw/opentherm/2026-04-14-sazanof-esp-idf-opentherm.md`.
* **Update**: Linked [Melnyk OpenTherm Library](/opentherm/melnyk-opentherm-library.md) and [OpenTherm GPIO Wiring](/bridge/opentherm-gpio-wiring.md) to the Sazanof component.

## 2026-08-29
* **Lint**: Regenerated stale `index.md` / `bridge/index.md`; replaced broken links to removed `docs/thermostats_data_ids_mapping.md` with [Water Setpoint Mapping](/bridge/water-setpoint-mapping.md) and [Local Temperature Mapping](/bridge/local-temperature-mapping.md).

## 2026-07-02
* **Update**: Phase 6–7 — v2 poll engine (`ot_poll`), Kconfig tunables, and bridge playbooks for discovery, endpoints, spillover, catalog commands, and poll tiers.
* **Creation**: Added [Discovery Catalog](/bridge/discovery-catalog.md), [Endpoint Layout](/bridge/endpoint-layout.md), [Spillover Encoding](/bridge/spillover-encoding.md), [Catalog Commands](/bridge/catalog-commands.md), and [Poll Tiers](/bridge/poll-tiers.md).
* **Update**: Revised [End-to-End Control Flow](/bridge/end-to-end-control-flow.md) and [Local Temperature Mapping](/bridge/local-temperature-mapping.md) for v2 layered routing.

## 2026-07-02
* **Update**: V1c complete — compiled `esp32/`, `esp-idf/`, `zigbee/`, and `bridge/` domains.
* **Creation**: Added [Water Setpoint Mapping](/bridge/water-setpoint-mapping.md), [Local Temperature Mapping](/bridge/local-temperature-mapping.md), [End-to-End Control Flow](/bridge/end-to-end-control-flow.md), and [OpenTherm GPIO Wiring](/bridge/opentherm-gpio-wiring.md).
* **Deprecation**: Retired `wiki/articles/` in favor of this bundle.

## 2026-07-02
* **Initialization**: Created `knowledge/` bundle and compiled `opentherm/` concepts from `wiki/raw/`.
* **Creation**: Added [OpenTherm Protocol](/opentherm/opentherm-protocol.md), [Frame Format](/opentherm/opentherm-frame-format.md), [Data Encoding](/opentherm/opentherm-data-encoding.md), [Data ID 0 Status](/opentherm/data-id-0-status.md), [Data ID 1 TSet](/opentherm/data-id-1-tset.md), [Data ID 25 Tboiler](/opentherm/data-id-25-tboiler.md), and [Melnyk OpenTherm Library](/opentherm/melnyk-opentherm-library.md).
