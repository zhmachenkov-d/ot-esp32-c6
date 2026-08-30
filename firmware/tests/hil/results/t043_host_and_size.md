# T043 / T048 validation notes (host + size + HIL)

**Date**: 2026-08-30 (updated Phase 7)

## Host tests

All Unity host suites under `firmware/tests/host` passed (`./run.sh`):

- test_ot_codec
- test_ot_catalog
- test_mqtt_discovery
- test_status_projections
- test_provision_validate (compiles production `provision_validate.c`)
- test_setpoint_bounds
- test_mqtt_commands (includes OT write-complete + retained-write policy)
- test_failsafe

## Image size

| Metric | Value | Budget |
|--------|-------|--------|
| `otc6_gateway.bin` | 1 149 360 bytes (~1.10 MiB) | ≤ 1.5 MiB (warn if > 1.2 MiB) |
| Free heap after OT+MQTT | *logged on device* (`main`: `free heap after OT+MQTT`) | ≥ 64 KiB |

Size is under both hard and soft budgets. Free-heap measurement requires a flashed WeAct Mini with OT+MQTT up; firmware now logs the value for HIL capture.

## HIL (V1–V9) sign-off

| Scenario | File | Status |
|----------|------|--------|
| V1 Commissioning | `v1_commissioning.md` | BLOCKED — no WeAct Mini / SoftAP HIL in this environment |
| V2 Discovery | `v2_discovery.md` | BLOCKED |
| V3 Read freshness | `v3_read_freshness.md` | BLOCKED |
| V4 Write roundtrip | `v4_write_roundtrip.md` | BLOCKED |
| V5 Reject | `v5_reject.md` | BLOCKED |
| V6 Keepalive under load | `v6_keepalive_under_load.md` | BLOCKED |
| V7 Fail-safe | `v7_failsafe.md` | BLOCKED |
| V8 Boiler-link | `v8_boiler_link.md` | BLOCKED |
| V9 No Zigbee/Thread | `v9_no_zigbee_thread.md` | PARTIAL — sdkconfig / link check (SC-006) OK in host/CI; on-device confirm open |

**Sign-off**: Host + image-size evidence complete. On-device HIL V1–V8 and heap floor remain hardware-gated; checklists under `firmware/tests/hil/` are ready to execute on WeAct Mini + OT adapter.

## SC-006

`CONFIG_OPENTHREAD_ENABLED` is not set in `sdkconfig`. No Zigbee component linked in the app.

## T008 C6 framing gate

Software bring-up of `sazanof/opentherm` on GPIO2/3 is complete and builds. Physical IRQ/framing validation on WeAct Mini is deferred to HIL (if framing fails on hardware, execute T008b Melnyk-port).
