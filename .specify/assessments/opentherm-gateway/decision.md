# Decision: Go — thin Wi‑Fi MQTT OpenTherm gateway

- **Slug**: opentherm-gateway
- **Decided**: 2026-08-29
- **Verdict**: go
- **Artifacts reviewed**: intake.md | research.md | problem.md | concept.md
- **Supersedes**: prior kill on this slug (same date); operator ruled out Zigbee and confirmed Option C (Wi‑Fi MQTT, no Thread)

## Scorecard

| Criterion | Rating | Justification |
|-----------|--------|---------------|
| Problem validity | adequate | Operator needs OpenTherm sensors/commands in HA without Zigbee; Wi‑Fi MQTT closes that job under an explicit no-Zigbee / no-Thread constraint. |
| Evidence strength | adequate | Research medium-confidence and well-cited on OT cadence, MQTT Discovery, and Wi‑Fi MQTT prior art; supports a slim first-party build once Zigbee is off the table. |
| Value vs. inaction | adequate | With Zigbee unavailable, inaction is buy/adopt only; operator chose to own a slim ESP32-C6 MQTT stack rather than adopt OTGW/OTGateway. |
| Feasibility / appetite | adequate | Option C is a credible medium-appetite slice; Thread/BR risk removed; C6 OT stack validation remains but is in-appetite. |
| Strategic fit | adequate | Operator product direction is Wi‑Fi MQTT (not Zigbee/Thread); constitution still Zigbee-framed — amend during specify, do not dual-radio. |
| Risk posture | adequate | Major Thread risks dropped; remaining risks (entity creep, fail-safe, C6 OT lib, slim vs OTGW) are known and suitable for specify/clarify. |

## Verdict & Rationale

**Go** on **Option C — Thin Wi‑Fi MQTT OpenTherm gateway**. Operator constraints: **no Zigbee, no OpenThread**. That removes the earlier kill’s “prefer the Zigbee bridge” alternative and keeps the shaped, buildable MQTT↔HA path without BR/NAT64/Thread unknowns. Differentiation is slim scope and fit to this repo (ESP32-C6 + existing OT knowledge), not transport novelty. Overlap with Wi‑Fi MQTT incumbents is accepted.

## If go — Handoff to `/speckit-specify`

- **Problem**: Close OpenTherm boiler monitor/control in Home Assistant over Wi‑Fi MQTT on ESP32-C6, without Zigbee or Thread.
- **Chosen approach**: Option C — thin OpenTherm **master** + Wi‑Fi MQTT client + Home Assistant MQTT Discovery; small entity set; OT keepalive/polling first-class; explicit fail-safe when Wi‑Fi/MQTT is down.
- **In scope / out of scope**:
  - **In**: WeAct ESP32-C6-A (or confirmed ESP32-C6) as OT master; Wi‑Fi MQTT to broker; MQTT Discovery climate/sensors/status; small Data ID / command set; fail-safe on link loss; OT poll cadence ≥ ~1 s floor.
  - **Out**: OpenThread / MQTT-over-Thread / “Thread later”; Zigbee (any role, including dual-radio); acting as OTBR; OTGW-scale entity/Data ID parity; custom HA Core integration; certifying every boiler SKU.
- **Success metrics**: Sensor freshness and command round-trip within bounds set at specify; OT keepalive/poll floor held under HA traffic; documented fail-safe on Wi‑Fi/MQTT outage; operator can see/control in-scope entities via MQTT Discovery without bespoke HA Core code.
- **Carried-forward open questions**:
  - Confirm hardware: WeAct ESP32-C6-A / ESP32-C6?
  - Initial Data IDs and HA commands (keep small—e.g. Status, TSet, Tboiler / climate essentials)?
  - Fail-safe policy when Wi‑Fi/MQTT is down (hold last setpoint vs demand off / other)?
  - OpenTherm stack on ESP-IDF: Sazanof (C6 unconfirmed) vs alternate?
  - Numeric targets for sensor freshness and command round-trip?
  - MQTT broker assumptions (LAN Mosquitto / HA add-on, credentials, TLS)?
  - Amend constitution from Zigbee-framed product language to Wi‑Fi MQTT (specify/governance follow-up)?
