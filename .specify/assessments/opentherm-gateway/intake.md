# Idea Intake: OpenTherm Gateway Firmware (Home Assistant via MQTT / OpenThread)

- **Slug**: opentherm-gateway
- **Created**: 2026-08-29
- **Source**: pasted text; repo OKF knowledge under `knowledge/` (esp. `knowledge/opentherm/`, `knowledge/bridge/`, `knowledge/esp32/`); online docs referenced by requester but no URL supplied
- **Type**: new-capability

## Idea (as captured)

> I want create a firmware for OpenTherm gateway. This device should connect to OpenTherm boilers and translate data from sensors to Home Assistant. Also this device should receive and execute commands from Home Assistant. Connection to Home Assistant should be realized by MQTT over OpenThread. You can find documents about it in OKF knowledge base in this project and online.

Project knowledge already documents related building blocks (not taken as the chosen design for this idea): OpenTherm protocol / Data IDs / Melnyk library (`knowledge/opentherm/`); WeAct ESP32-C6-A and ESP-IDF get-started (`knowledge/esp32/`, `knowledge/esp-idf/`); and an existing Zigbee-oriented bridge playbook stack (`knowledge/bridge/`, `knowledge/zigbee/`) covering GPIO wiring, poll tiers, discovery catalog, and end-to-end control flow. No MQTT or OpenThread concepts were found in the OKF bundle at intake time.

## Restated

Build firmware for a device that acts as an OpenTherm master toward boilers, exposes boiler/sensor readings to Home Assistant, and accepts HA commands to control the boiler—using MQTT carried over an OpenThread network as the HA link.

## Origin & Context

- **Raised by**: project operator (interactive Spec Kit intake in this repo)
- **Trigger**: desire for OpenTherm ↔ Home Assistant integration with MQTT over OpenThread; points at existing OKF knowledge and unspecified online documentation
- **Related repo context**: `knowledge/` is compiled for “esp32-c6-opentherm”; bridge concepts today describe Zigbee↔OpenTherm paths—relationship of this MQTT/OpenThread idea to that prior direction is unsettled

## First-Glance Unknowns

- [NEEDS CLARIFICATION: intended hardware target—confirm WeAct ESP32-C6-A / ESP32-C6, or another board?]
- [NEEDS CLARIFICATION: is this a replacement for, parallel to, or evolution of the documented Zigbee bridge design?]
- [NEEDS CLARIFICATION: which boiler sensor Data IDs and HA commands are in scope for an initial release?]
- [NEEDS CLARIFICATION: MQTT topic model / Home Assistant discovery expectations (e.g. MQTT Discovery vs manual entities)?]
- [NEEDS CLARIFICATION: OpenThread role and provisioning (FTD/MTD, how the device joins the Thread network / border router)?]
- [NEEDS CLARIFICATION: which online documents should be treated as authoritative sources for research?]
