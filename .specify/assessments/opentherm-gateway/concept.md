# Concept: OpenTherm ↔ HA — transport choice

- **Slug**: opentherm-gateway
- **Created**: 2026-08-29
- **Recommended option**: C — Thin Wi‑Fi MQTT OpenTherm gateway (no OpenThread)

## Options

### Option A — Do nothing / buy (Zigbee path or Wi‑Fi MQTT incumbent)
- **Sketch**: Keep using the repo’s documented Zigbee↔OpenTherm bridge (ZHA / Zigbee2MQTT), or adopt a mature Wi‑Fi/Ethernet MQTT OpenTherm gateway (e.g. OTGW-firmware, OTGateway) or HA’s official `opentherm_gw` over serial/`socket://`. The operator gets boiler sensors and commands in Home Assistant without new Thread or MQTT-on-ESP work in this repo.
- **Appetite**: small
- **Trade-offs**: Wins proven HA integration, reuses existing bridge investment (or ships someone else’s product), and avoids Border Router / NAT64 / MQTT-over-Thread unknowns. Sacrifices the stated MQTT-over-OpenThread preference and leaves Thread-only or no-Wi‑Fi-at-boiler installs without a matching path. Risk: operator’s preference stays unmet and the repo never resolves whether Zigbee is the long-term product.
- **Rabbit holes**: Feature-chasing against OTGW’s large entity catalogs if “buy” turns into “customize”; unclear whether Zigbee work continues or freezes.

### Option B — Thin MQTT-over-OpenThread slice
- **Sketch**: A purpose-built OpenTherm **master** on ESP32-C6 that joins an **existing** Thread mesh (end device / FTD—not the Border Router), speaks MQTT to a broker reachable via that mesh (and typically a home OTBR), and exposes a **small** Home Assistant surface via MQTT Discovery (status, a few sensors, basic climate/setpoint commands). OpenTherm keepalive/polling stays first-class; when Thread/MQTT/BR is down, heating follows an explicit fail-safe. The documented Zigbee bridge remains a separate, parked-or-parallel design—not dual-radio on one device.
- **Appetite**: medium (weeks as a budget; Thread + MQTT + OT on one device may stretch if BR/IPv6 path or OT library validation slips)
- **Trade-offs**: Wins alignment with stated goals (sensors + commands under MQTT-over-OpenThread, OT cadence, fail-safe, no bespoke HA Core integration). Sacrifices OTGW-class entity breadth, Zigbee continuity on the same radio, and any claim of being the household Thread infrastructure. Risks: unproven MQTT-over-Thread for this product class; external BR/NAT64 dependency; Sazanof C6 OpenTherm support unconfirmed; no demand evidence beyond the operator.
- **Rabbit holes**: Broker addressing (on-Thread IPv6 vs LAN IPv6 vs IPv4/NAT64); Thread join/commissioning UX; OpenTherm stack port/validation on C6; entity-catalog creep toward OTGW parity; accidental dual Zigbee+Thread ambition; fail-safe policy debates that stall shipping.

### Option C — Thin Wi‑Fi MQTT OpenTherm gateway (no OpenThread)
- **Sketch**: Ship a thin OpenTherm **master** on ESP32-C6 that talks to Home Assistant over **Wi‑Fi MQTT** with MQTT Discovery—small entity set (status, key sensors, basic climate/setpoint), OpenTherm keepalive/polling first-class, and an explicit fail-safe when Wi‑Fi/MQTT is down. Sized to this repo’s needs, not OTGW feature parity. OpenThread is **not** in scope (no “Thread later” bet in this concept). The documented Zigbee bridge stays parked or parallel—not dual-radio.
- **Appetite**: medium
- **Trade-offs**: Wins a buildable MQTT↔HA path without BR/NAT64/Thread unknowns; reuses familiar incumbent patterns (MQTT Discovery, emergency/fail-safe) at slim scope. Sacrifices Thread differentiation and overlaps crowded Wi‑Fi MQTT prior art (OTGW-firmware, OTGateway); does not help no-Wi‑Fi-at-boiler sites. Risks: reinventing what buy/adopt already offers; diluting or abandoning the Zigbee investment without a clear product reason.
- **Rabbit holes**: Entity-catalog creep toward OTGW parity; re-introducing OpenThread mid-delivery; dual Zigbee+Wi‑Fi ambition; fail-safe policy debates; C6 OpenTherm stack validation (Sazanof unconfirmed).

## Recommendation

**Option C — Thin Wi‑Fi MQTT OpenTherm gateway (no OpenThread)**: build the OpenTherm↔HA job the operator wants (sensors, commands, OT cadence, fail-safe, MQTT Discovery) over Wi‑Fi MQTT, and drop OpenThread from this concept entirely. That removes the least-proven, highest-infrastructure risk from research while still producing a first-party firmware path—unlike A (buy/stay Zigbee only)—without committing to Thread mesh/BR/NAT64 as in B.

Accept that this overlaps incumbent Wi‑Fi MQTT gateways and that differentiation must come from slim scope and fit to this repo (ESP32-C6, existing OT knowledge), not from Thread.

## Out of Scope (for the recommended option)

- OpenThread / MQTT-over-Thread / “Thread later” as part of this delivery
- Acting as Thread Border Router or any OTBR/commissioning work
- Concurrent Zigbee + Wi‑Fi (or Zigbee + Thread) on the same radio / firmware image
- Feature parity with OTGW-firmware / OTGateway entity catalogs and full Data ID coverage
- Replacing or rewriting the documented Zigbee bridge in the same delivery (park or parallel by policy)
- Custom Home Assistant Core integration (MQTT Discovery / standard MQTT entities only)
- Certifying every boiler SKU in the first release
- General Home Assistant MQTT adoption for non-OpenTherm devices

## Assumptions to Validate

- Wi‑Fi is available at the boiler / install site (no-Wi‑Fi-at-boiler is not a hard constraint)
- OpenThread is explicitly **out**—not a deferred promise under this recommendation
- Building (vs buying OTGW/OTGateway) is justified—e.g. learning, board fit, or control of a slim stack—not reinventing for its own sake
- Target hardware remains WeAct ESP32-C6-A / ESP32-C6 as OpenTherm master + Wi‑Fi MQTT client
- Initial sensor/command set stays small (bridge fast-tier / climate essentials—not OTGW-scale)
- MQTT Discovery is acceptable for HA
- Fail-safe heating policy when Wi‑Fi/MQTT is down can be chosen and documented before ship
- Relationship to the Zigbee bridge is “park or parallel,” not dual-radio replacement
- OpenTherm on ESP-IDF for C6 is achievable within a medium appetite (Sazanof validation or another stack)
- Sensor freshness and command round-trip numeric targets can be set at specify time without expanding scope
