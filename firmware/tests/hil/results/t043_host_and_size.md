# T043 / T048 / T057 validation notes (host + size + HIL)

**Date**: 2026-08-31 (Phase 9 / T057)

## Host tests

All Unity host suites under `firmware/tests/host` passed (`./run.sh`), including:

- test_ot_codec
- test_ot_catalog (directory write-class + probe path)
- test_mqtt_discovery
- test_status_projections
- test_provision_validate (compiles production `provision_validate.c`)
- test_setpoint_bounds
- test_mqtt_commands (includes OT write-complete + retained-write policy)
- test_failsafe
- test_boiler_link (consecutive-fail FSM for keepalive/status)
- test_ot_poll_bus_lock

## Image size

| Metric | Value | Budget |
|--------|-------|--------|
| `otc6_gateway.bin` | 1 149 360 bytes (~1.10 MiB) | ≤ 1.5 MiB (warn if > 1.2 MiB) |
| Free heap after OT+MQTT | *not measured in this environment* | ≥ 64 KiB |

Size is under both hard and soft budgets. Firmware logs `free heap after OT+MQTT: …` on device once STA+MQTT are up (`main.c`) for HIL capture.

## HIL (V1–V8) sign-off — hardware deferral

| Scenario | File | Status |
|----------|------|--------|
| V1 Commissioning | `v1_commissioning.md` | DEFERRED — no WeAct Mini / SoftAP HIL in this environment |
| V2 Discovery | `v2_discovery.md` | DEFERRED |
| V3 Read freshness | `v3_read_freshness.md` | DEFERRED |
| V4 Write roundtrip | `v4_write_roundtrip.md` | DEFERRED |
| V5 Reject | `v5_reject.md` | DEFERRED |
| V6 Keepalive under load | `v6_keepalive_under_load.md` | DEFERRED |
| V7 Fail-safe | `v7_failsafe.md` | DEFERRED |
| V8 Boiler-link | `v8_boiler_link.md` | DEFERRED |
| V9 No Zigbee/Thread | `v9_no_zigbee_thread.md` | PARTIAL — sdkconfig / link check (SC-006) OK in host/CI; on-device confirm open |

**Owner of hardware deferral**: feature branch maintainer (`features/001-wifi-mqtt-opentherm`) — execute `firmware/tests/hil/v1`–`v8` on WeAct ESP32-C6 Mini + OT adapter + LAN MQTT/HA; record free-heap line from serial and flip statuses above to PASS/FAIL with dates.

**Sign-off**: Host + image-size evidence complete. On-device HIL V1–V8 and ≥64 KiB heap floor remain **explicitly deferred** pending physical hardware (T057).

## SC-006

`CONFIG_OPENTHREAD_ENABLED` is not set in `sdkconfig`. No Zigbee component linked in the app.

## T008 C6 framing gate

Software bring-up of `sazanof/opentherm` on GPIO2/3 is complete and builds. Physical IRQ/framing validation on WeAct Mini is deferred to HIL (if framing fails on hardware, execute T008b Melnyk-port).
