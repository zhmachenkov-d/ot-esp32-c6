# V10 — OTA (dual-slot HTTPS + HA update)

Preconditions: partition table with `otadata`/`ota_0`/`ota_1` flashed once over USB; device on Wi‑Fi with MQTT; GitHub Release with `manifest.json` + `.bin` per `firmware/README.md`.

1. **Happy path:** Manifest newer than `APP_FW_VERSION`; HA Update entity shows `latest_version`; Install → `in_progress`/percentage → reboot → MQTT up → `installed_version` matches Release.
2. **Bad asset / TLS:** Point manifest (or block host) so Install fails → no slot flip; state returns `in_progress: false`.
3. **Power loss mid-write:** Reset during download → last valid app boots.
4. **Confirm timeout / rollback:** Flash a broken B that never reaches MQTT session ready → after ~15 min device restarts and returns to A.
5. **SoftAP:** Boot without credentials (or long-press clear) → Install does not run; pending-verify still times out for rollback if applicable.
6. **Manifest poll:** Publish newer Release; within poll policy (MQTT-up or ≤12h) HA shows new `latest_version` without USB flash.

Record pass/fail in `firmware/tests/hil/results/` when executed.
