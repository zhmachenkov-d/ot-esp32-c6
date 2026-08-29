# Problem Definition: OpenTherm ↔ Home Assistant without a Thread-native path

- **Slug**: opentherm-gateway
- **Created**: 2026-08-29
- **Inputs used**: intake.md | research.md

## Problem Statement

Operators who want to monitor and control OpenTherm boilers from Home Assistant lack a reliable way to do so when the preferred (or only) home-automation backhaul is MQTT over an OpenThread mesh. Mature OpenTherm↔HA options today assume Wi‑Fi/Ethernet MQTT, a serial/socket gateway, or Zigbee—so Thread-oriented installs, or sites without suitable Wi‑Fi at the boiler, still cannot close the loop between boiler sensors/commands and HA under those constraints.

## Affected Users & Stakeholders

- **Users**: Project operator (this repo) — wants OpenTherm boiler readings in HA and the ability to issue boiler control commands from HA, with the HA link realized as MQTT over OpenThread. — [source: intake.md, research.md]
- **Users**: [NEEDS CLARIFICATION: are there other intended end users beyond the operator—e.g. Thread-only HA households, or is this a single-operator product?] — no external demand evidence found [source: research.md]
- **Stakeholders**: Project operator — decides direction, funds effort, and owns trade-offs vs the existing Zigbee bridge investment. — [source: intake.md, research.md]
- **Stakeholders**: Home occupants / heating safety — impacted by loss of HA connectivity or unsafe fail-open/fail-closed heating behavior when the mesh or broker is down. — [source: research.md; OTGateway emergency-mode prior art]

## Goals

- Make boiler status and key sensor values continuously visible in Home Assistant under the operator’s chosen HA link constraints (MQTT carried over OpenThread).
- Allow Home Assistant to request boiler control actions that are executed on the OpenTherm bus with acceptable lag for heating use.
- Preserve OpenTherm master keepalive/polling cadence so the boiler link does not starve while the HA link is active or degraded.
- Define and meet an explicit fail-safe heating outcome when Thread, MQTT, or the border path is unavailable.
- Clarify relationship to the repo’s existing Zigbee↔OpenTherm bridge investment so effort is not duplicated or accidentally abandoned. — [NEEDS CLARIFICATION on replacement vs parallel vs evolution]

## Non-Goals

- Competing feature-for-feature with incumbent Wi‑Fi MQTT OpenTherm gateways (e.g. OTGW-firmware’s large entity catalogs) unless later scoped as required.
- Acting as the household Thread Border Router or replacing existing OTBR / commissioning infrastructure.
- Solving general Home Assistant MQTT or OpenThread adoption for non-OpenTherm devices.
- Guaranteeing concurrent Zigbee + Thread on the same radio without a later deliberate platform decision.
- Certifying or productizing for every boiler SKU / every OpenTherm Data ID in the first release.

## Success Metrics

- **Sensor freshness in HA**: time from OpenTherm master read of in-scope Data IDs to visible HA entity update stays within an agreed bound under normal Thread/MQTT conditions (baseline: unknown; target: [NEEDS CLARIFICATION: e.g. ≤ N seconds for fast-tier IDs]).
- **Command round-trip**: time from HA command publish to observed effect on the OpenTherm bus (and reflected status) stays within an agreed bound (baseline: unknown; target: [NEEDS CLARIFICATION]).
- **OpenTherm link health**: master keepalive / status polling continues at ≥ ~1 s cadence without starvation while HA traffic is present (baseline: project poll-tier design ~1 s fast / ~60 s slow; target: hold that floor). — [source: research.md]
- **Outage behavior**: when Thread/MQTT/BR path is down, heating follows the documented fail-safe policy with no undefined boiler demand (qualitative + timed check; baseline: undefined today; target: [NEEDS CLARIFICATION: hold last setpoint / open-therm demand off / other]).
- **Operator usability (qualitative)**: operator can commission onto an existing Thread network and see/control the in-scope HA entities without bespoke HA Core integration code (baseline: Zigbee bridge path documented; MQTT/Thread path absent in OKF).

## Cost of Inaction

If nothing is built for this idea, OpenTherm↔HA remains available only via the documented Zigbee bridge path (ZHA / Zigbee2MQTT) or by adopting an existing Wi‑Fi/Ethernet MQTT or serial/`socket://` gateway. Thread-oriented or no-Wi‑Fi-at-boiler installs keep lacking a matching off-the-shelf OpenTherm path; the operator’s stated MQTT-over-OpenThread preference goes unmet, and the repo continues investing in Zigbee-framed constitution/playbooks without resolving whether that investment should be replaced, forked, or left alone.

## Open Questions

- [NEEDS CLARIFICATION: intended hardware target—confirm WeAct ESP32-C6-A / ESP32-C6, or another board?]
- [NEEDS CLARIFICATION: is this a replacement for, parallel to, or evolution of the documented Zigbee bridge design?]
- [NEEDS CLARIFICATION: which boiler sensor Data IDs and HA commands are in scope for an initial release?]
- [NEEDS CLARIFICATION: MQTT topic model / Home Assistant discovery expectations (MQTT Discovery vs manual YAML)?]
- [NEEDS CLARIFICATION: OpenThread role and provisioning (FTD vs MTD/FED; join via external commissioner / BR; who provides the OTBR in the home?)]
- [NEEDS CLARIFICATION: is OpenThread a hard requirement (no Wi‑Fi at install site / Thread-only mesh policy), or a preference vs Wi‑Fi MQTT?]
- [NEEDS CLARIFICATION: where does the MQTT broker live (on-Thread IPv6, LAN IPv6 via BR, or IPv4 via NAT64 / HA Mosquitto add-on)?]
- [NEEDS CLARIFICATION: fail-safe heating behavior when Thread/MQTT/BR is down?]
- [NEEDS CLARIFICATION: preferred OpenTherm stack on ESP-IDF—Melnyk (Arduino-oriented, documented) vs Sazanof (native IDF, C6 unconfirmed)?]
- [NEEDS CLARIFICATION: are there intended users beyond the project operator?]
- [NEEDS CLARIFICATION: numeric targets for sensor freshness and command round-trip?]
