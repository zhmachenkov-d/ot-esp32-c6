# T043 validation notes (host + size)

**Date**: 2026-08-30

## Host tests

All Unity host suites under `firmware/tests/host` passed (`./run.sh`):

- test_ot_codec
- test_ot_catalog
- test_mqtt_discovery
- test_status_projections
- test_provision_validate
- test_setpoint_bounds
- test_mqtt_commands
- test_failsafe

## Image size

| Metric | Value | Budget |
|--------|-------|--------|
| `otc6_gateway.bin` | 1 061 360 bytes (~1.01 MiB) | ≤ 1.5 MiB (warn if > 1.2 MiB) |
| Free heap after OT+MQTT | *not measured (no device in CI)* | ≥ 64 KiB |

Note: size is under hard budget; slightly over the soft warn threshold of 1.2 MiB — PR should call out.

## HIL (V1–V9)

On-device HIL checklists remain open under `firmware/tests/hil/` — require WeAct Mini + OT adapter / boiler or simulator.

## SC-006

`CONFIG_OPENTHREAD_ENABLED` is not set in `sdkconfig`. No Zigbee component linked in the app.

## T008 C6 framing gate

Software bring-up of `sazanof/opentherm` on GPIO2/3 is complete and builds. Physical IRQ/framing validation on WeAct Mini is deferred to HIL (if framing fails on hardware, execute T008b Melnyk-port).
