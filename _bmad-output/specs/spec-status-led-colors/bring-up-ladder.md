# Bring-up ladder (status LED)

Live evaluation every tick; **no high-water**. SoftAP and critical are evaluated first (see SPEC Constraints); remaining rungs use else/highest-wins order below.

## Priority overrides

| Priority | Condition | Color | Pattern |
| --- | --- | --- | --- |
| Highest | SoftAP active | Magenta | Slow blink (1 Hz) |
| Next | Critical (fail-safe active **or** `PROVISION_BOOT_RUN_NO_MQTT` **or** OTA live `s_failed`) | Red | Fast blink (4 Hz) |

SoftAP beats critical.

## Ladder rungs (else, highest wins)

Bind inputs: SoftAP active, STA+IP, `mqtt_ha_connected()`, `s_mqtt_session_ready`, `ot_poll_boiler_link_healthy()`.

| Order | Condition (informal) | Color | Pattern |
| --- | --- | --- | --- |
| 1 (lowest) | No Wi‑Fi / no IP | Orange | Slow blink |
| 2 | Wi‑Fi up, MQTT not connected | Yellow | Slow blink |
| 3 | MQTT up, session not ready, OT down | Cyan | Slow blink |
| 4 | Session not ready, OT healthy | Blue | Solid |
| 5 | Session ready, OT unhealthy | Purple | Solid |
| 6 (top) | Session ready and OT healthy | Green | Solid |

Forge shorthand for the same mapping: SoftAP magenta slow; critical red fast; no Wi‑Fi orange slow; Wi‑Fi no MQTT yellow slow; MQTT incomplete cyan slow; OT-only blue solid; MQTT-ready OT-down purple solid; both green solid.
