# Contract: SoftAP Captive Portal Provisioning

**Feature**: `001-wifi-mqtt-opentherm`  
**Date**: 2026-08-30  
**Consumers**: Human operator (phone/laptop browser)

## Modes

| Mode | Entry | Exit |
|------|-------|------|
| Provisioning | No saved credentials, or GPIO9 long-press ≥5 s (clears credentials) | Valid form submit → NVS save → STA join attempt |
| Operational | STA associated + MQTT configured | Long-press only for re-provision (v1) |

Sustained Wi‑Fi/MQTT failure alone MUST NOT open SoftAP in v1.
Missing or corrupt MQTT CA while TLS is enabled MUST NOT open SoftAP; recover via long-press only.

## SoftAP

| Parameter | Value |
|-----------|-------|
| SSID | `OTC6-XXXX` (XXXX from device id suffix) |
| Security | **WPA2-PSK** — per-device random 16-hex PSK in NVS (`softap_psk`); logged on SoftAP start (serial); survives credential clear for label/QR |
| Captive | DNS catch-all → HTTP UI on gateway AP IP |

## HTTP UI fields

| Field | Required | Persistence |
|-------|----------|-------------|
| Wi‑Fi SSID | yes | NVS |
| Wi‑Fi password | yes (if network secured) | NVS |
| MQTT host | yes | NVS |
| MQTT port | yes (default 1883) | NVS |
| MQTT username | no | NVS |
| MQTT password | no | NVS |
| MQTT TLS | no (default off) | NVS |
| MQTT CA PEM | **yes if TLS** | NVS blob `mqtt_ca` (max 4096 bytes, NUL-terminated PEM) |
| CH setpoint min °C | yes (seed 10.0) | NVS |
| CH setpoint max °C | yes (seed 90.0) | NVS |
| Setup PIN | yes (from USB serial at SoftAP start; one-time) | not persisted |

**Validation**: min < max; host non-empty; when MQTT TLS is checked, CA PEM must be non-empty; **Setup PIN must match the current SoftAP session token** (logged on serial; not served in HTML); reject submit with inline error otherwise. Do not proceed to STA with invalid form. A valid PIN is consumed on successful save so a concurrent SoftAP client cannot reuse it.

When TLS is off, any previously stored CA PEM is cleared on save.

## Button

| Signal | Board | Action |
|--------|-------|--------|
| Long-press ≥5 s | WeAct SW2 **GPIO9** | Clear Wi‑Fi + MQTT credentials **and** MQTT CA PEM; enter SoftAP provisioning |

## Security notes

- Credentials and CA PEM stored in NVS only; never commit
- SoftAP uses WPA2-PSK so concurrent RF neighbors without the device PSK cannot join or sniff cleartext HTTP `POST /save`
- SoftAP PSK is device-local (NVS); obtain it from serial log at SoftAP start, or a factory label/QR that mirrors the same value
- `POST /save` requires a **one-time Setup PIN** logged on USB serial at SoftAP start (distinct from the SoftAP PSK). The PIN is not embedded in the portal HTML; missing/invalid PIN → 403; successful save consumes the PIN for that SoftAP session
- With a provisioned CA PEM, the client verifies the broker against that trust anchor and skips certificate CN checks so LAN IPs work with self-signed Mosquitto certs
- Portal remains HTTP on the SoftAP network; association uses SoftAP PSK; save authorization uses the serial Setup PIN
