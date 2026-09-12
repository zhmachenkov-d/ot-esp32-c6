# Status LED bring-up ladder (ot-esp32-c6)

## Done
- LED answers **how far bring-up got** via a **live** ladder (recompute every tick; no high-water).
- Hardware: WeAct Mini **WS2812 on IO8**; dim ~10–20%; slow blink **1 Hz**, fast blink **4 Hz**.
- **SoftAP** beats **critical**; critical beats all other rungs.
- Critical when any: fail-safe active; `PROVISION_BOOT_RUN_NO_MQTT`; OTA live `s_failed`.
- Ladder (else, highest wins): SoftAP → no Wi‑Fi/IP → Wi‑Fi no MQTT → MQTT up, session not ready, OT down → session not ready + OT healthy → session ready + OT unhealthy → both (top).
- Bind to existing flags: SoftAP active, STA+IP, `mqtt_ha_connected()`, `s_mqtt_session_ready`, `ot_poll_boiler_link_healthy()`.
- Colors: SoftAP magenta slow blink; critical red fast blink; no Wi‑Fi orange slow blink; Wi‑Fi no MQTT yellow slow blink; MQTT incomplete cyan slow blink; OT-only blue solid; MQTT-ready OT-down purple solid; both green solid.

## Rejected
- Separate “live health dashboard” job for the LED.
- Sticky post-rollback / pending-verify-as-critical (v1).
- High-water mark after first green.

## Surviving cracks
- Colorblind users may need pattern more than hue (blink vs solid helps; magenta/purple/blue still close).
- `s_failed` clears on next successful path only if firmware already does — confirm on implement.
