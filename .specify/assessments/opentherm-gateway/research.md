# Idea Research: OpenTherm Gateway Firmware (HA via MQTT / OpenThread)

- **Slug**: opentherm-gateway
- **Created**: 2026-08-29
- **Evidence confidence (overall)**: medium

## Users & Demand

- The idea is raised by the project operator for this repo; no support tickets, interviews, usage telemetry, or external request volume were available in-repo at research time. — [source: `.specify/assessments/opentherm-gateway/intake.md`] (confidence: high, cited)
- Stated want: OpenTherm master firmware that exposes boiler sensors to Home Assistant and accepts HA commands, with the HA link realized as MQTT over OpenThread. — [source: intake.md] (confidence: high, cited)
- Observed external behavior (not this user): mature open-source OpenTherm↔HA products already use **MQTT over Wi‑Fi** (not Thread) as the primary HA path, and treat MQTT auto-discovery as the recommended integration. — [source: https://github.com/rvdbreemen/OTGW-firmware ; https://github.com/Laxilef/OTGateway] (confidence: high, cited)
- No quantitative demand signal for “MQTT over OpenThread specifically” (vs Wi‑Fi MQTT or Zigbee) was found. — [ASSUMPTION] (confidence: low)

## Prior Art

### Internal (this repository)

- Project knowledge already documents a **Zigbee↔OpenTherm bridge** playbook: join/control flow, GPIO wiring on WeAct ESP32-C6-A (GPIO12/13), discovery catalog, poll tiers (~1 s fast / ~60 s slow), Thermostat channel mapping, and spillover. Roles: ESP32-C6 as OpenTherm **master** + Zigbee **end device**; HA via ZHA / Zigbee2MQTT. — [source: `knowledge/bridge/end-to-end-control-flow.md`, `knowledge/bridge/index.md`, `knowledge/bridge/opentherm-gpio-wiring.md`, `knowledge/bridge/poll-tiers.md`, `knowledge/bridge/discovery-catalog.md`] (confidence: high, cited)
- OpenTherm protocol, Data IDs (0 Status, 1 TSet, 25 Tboiler), encoding, and Melnyk master library reference are compiled under `knowledge/opentherm/`. — [source: `knowledge/opentherm/index.md`, `knowledge/opentherm/melnyk-opentherm-library.md`] (confidence: high, cited)
- **Sazanof ESP-IDF OpenTherm** (`sazanof/esp-idf-opentherm`, component `sazanof/opentherm` ^1.0.3 / 1.0.7, MIT, IDF ≥ 5.2) is now in OKF: native ESP-IDF master (GPIO interrupt + `esp_timer`), no Arduino core; maps Status/TSet/Tboiler helpers; README marks ESP32 tested and **ESP32-C6 unconfirmed**. — [source: `knowledge/opentherm/sazanof-esp-idf-opentherm.md`; https://github.com/sazanof/esp-idf-opentherm] (confidence: high, cited)
- Hardware default is WeAct ESP32-C6-A; datasheet notes IEEE 802.15.4 with **Thread 1.3 and Zigbee 3.0** (same radio family as the documented Zigbee design). — [source: `knowledge/esp32/weact-esp32-c6-a.md`, `knowledge/esp32/esp32-c6-datasheet.md`] (confidence: high, cited)
- OKF search found **no** MQTT or OpenThread concepts in the knowledge bundle (only Zigbee/bridge + OpenTherm + ESP32; Thread appears only as a datasheet radio capability). — [source: `knowledge/` grep; intake.md note] (confidence: high, cited)
- Project constitution principles and rationale still frame the product as “firmware that bridges OpenTherm and **Zigbee**.” — [source: `.specify/memory/constitution.md`] (confidence: high, cited)

### External

- **rvdbreemen/OTGW-firmware** — Networked OpenTherm Gateway firmware with MQTT + Home Assistant MQTT Auto Discovery (README claims 250+ HA entities; 80+ OT message IDs monitored); also TCP serial bridge on port **25238** for HA’s built-in OpenTherm Gateway integration. Actively maintained (~201 stars; latest GitHub release tag **v1.7.4**, 2026-08-10; `dev` README describes **v2.0.0** ESP32/OTGW32 platform with OTDirect GPIO master/slave). Network path is **Wi‑Fi/Ethernet**, not Thread. — [source: https://github.com/rvdbreemen/OTGW-firmware ; https://raw.githubusercontent.com/rvdbreemen/OTGW-firmware/dev/README.md] (confidence: high, cited)
- **Home Assistant OpenTherm Gateway (`opentherm_gw`)** — official integration controls an OpenTherm Gateway via **serial/USB or network serial socket** (`socket://[IP]:[port]`, e.g. `socket://192.168.0.250:25238`). Adds three devices per gateway; main control is a single `climate` entity; many sensors disabled by default. Docs note OT is polling-based so HA↔thermostat propagation can lag. **No MQTT** on this integration page. — [source: https://www.home-assistant.io/integrations/opentherm_gw/] (confidence: high, cited)
- **Home Assistant MQTT** — MQTT runs over TCP/IP; entities may be set up via **MQTT discovery** or manual YAML/subentries; discovery supports Climate (HVAC), Sensor, Switch, Number, etc. Discovery topic shape: `<discovery_prefix>/<component>/[<node_id>/]<object_id>/config` (default prefix `homeassistant`). Birth/LWT status default topic `homeassistant/status`. — [source: https://www.home-assistant.io/integrations/mqtt/] (confidence: high, cited)
- **Laxilef/OTGateway** — ESP32/ESP8266 OpenTherm gateway with Home Assistant via MQTT; emergency mode on loss of Wi‑Fi/MQTT/sensors; ~451 stars; latest release **1.6.0** (2025-12-20); topics include MQTT / Home Assistant. Wi‑Fi-centric. — [source: https://github.com/Laxilef/OTGateway] (confidence: high, cited)
- GitHub search `opentherm mqtt` returns a cluster of Wi‑Fi MQTT gateways (OTGateway, OTGW-firmware, SmartTherm, mqtt_thermostat, opentherm2mqtt, etc.); search `opentherm thread` returned **0** repositories. — [source: GitHub Search API] (confidence: medium, cited)
- **ihormelnyk/opentherm_library** — widely referenced OpenTherm v2.2 master library (~282 stars; already mirrored in project knowledge). — [source: `knowledge/opentherm/melnyk-opentherm-library.md`; https://github.com/ihormelnyk/opentherm_library] (confidence: high, cited)
- **ESP-IDF OpenThread (ESP32-C6)** — OpenThread is an IP stack on 802.15.4; **Standalone Node** mode (full stack + app on-chip) is available on ESP32-C6. `openthread/ot_cli` demonstrates init and **socket-based** server/client. To launch a border router on ESP, docs require connecting an **RCP to a Wi‑Fi-capable chip** (e.g. ESP32) and `esp_openthread_border_router_init()` / `openthread/ot_br` example. — [source: https://docs.espressif.com/projects/esp-idf/en/stable/esp32c6/api-guides/openthread.html] (confidence: high, cited)
- **ESP-MQTT (ESP-IDF ≥ v6)** — MQTT client moved to separate `espressif/mqtt` component; IDF examples cover MQTT/MQTT 5 over TLS to brokers. Docs describe a standard IP MQTT client (TCP/TLS/WS), not a Thread-specific transport. — [source: https://docs.espressif.com/projects/esp-idf/en/stable/esp32c6/api-reference/protocols/mqtt.html] (confidence: high, cited)
- **OpenThread Border Router / roles** — A Thread network **requires a Border Router** to connect to other networks; BR provides bidirectional IP (Thread ↔ Wi‑Fi/Ethernet), mDNS↔SRP service discovery, external commissioning. Device types: FTD (radio always on; ECD/FED) vs MTD (MED/SED). BR is a distinct role from End Device. OTBR NAT64 translates TCP/UDP/ICMP for IPv4 reachability from Thread hosts. — [source: https://openthread.io/guides/border-router ; https://openthread.io/guides/thread-primer/node-roles-and-types ; https://openthread.io/codelabs/openthread-border-router-nat64] (confidence: high, cited)
- **MQTT on Thread end devices (ESP32-C6)** — Espressif IDF issue #18355 (open, 2026-03) asks whether `esp-mqtt` can run on a Thread end device through a Thread Border Router to an external broker, and raises TCP/power/MTU/reliability concerns. — [source: https://github.com/espressif/esp-idf/issues/18355] (confidence: medium, cited)
- **ESPHome + OpenThread + MQTT** — community report of ESP32-C6 OpenThread device failing to resolve an IPv6 MQTT broker address even when Thread ping/API works (issue #13643, closed). — [source: https://github.com/esphome/esphome/issues/13643] (confidence: medium, cited)
- No mature open-source **OpenTherm gateway** was found that advertises **MQTT over OpenThread** as its HA transport (searches surfaced Wi‑Fi MQTT OT gateways and separate Thread/MQTT plumbing questions). — [ASSUMPTION based on search coverage] (confidence: medium)

## Market & Context

- Users who want OpenTherm ↔ Home Assistant today commonly adopt: (a) OTGW + MQTT auto-discovery firmware, (b) OTGW/serial/`socket://` + official HA `opentherm_gw` integration, or (c) DIY ESP32 MQTT gateways (e.g. OTGateway). — [source: https://github.com/rvdbreemen/OTGW-firmware ; https://www.home-assistant.io/integrations/opentherm_gw/ ; https://github.com/Laxilef/OTGateway] (confidence: high, cited)
- Cost of doing nothing (for this repo): keep building / using the documented Zigbee bridge path already modeled in `knowledge/bridge/` — HA still reachable via ZHA/Zigbee2MQTT without inventing MQTT or Thread stacks. — [source: `knowledge/bridge/end-to-end-control-flow.md`] (confidence: high, cited)
- Cost of doing nothing (for the operator’s stated MQTT/Thread preference): remaining on Zigbee or adopting an existing Wi‑Fi MQTT OT gateway; Thread-only homes without Wi‑Fi at the boiler location would lack a matching off-the-shelf OT gateway per research above. — [ASSUMPTION] (confidence: low)
- Differentiator of *this* idea vs market incumbents is the **transport** (MQTT/OpenThread on ESP32-C6), not the existence of OpenTherm↔HA bridging itself. — [ASSUMPTION] (confidence: medium)
- If the product speaks MQTT Discovery, HA already documents climate/sensor/number discovery patterns; reinventing HA entity models is unnecessary. — [source: https://www.home-assistant.io/integrations/mqtt/] (confidence: high, cited)

## Data & Constraints

- OpenTherm master must keep communicating on the order of **≥ ~1 s** (keepalive / status); project poll engine uses ~1 s fast ticks and 120 ms inter-frame gaps. Any HA link (MQTT or Zigbee) must not starve this cadence. HA’s own OT gateway docs also warn that OT polling delays HA↔thermostat propagation. — [source: `knowledge/opentherm/melnyk-opentherm-library.md`, `knowledge/bridge/poll-tiers.md`; https://www.home-assistant.io/integrations/opentherm_gw/] (confidence: high, cited)
- Physical OT bus needs a level/current adapter (GPIO cannot drive 7–18 V / mA directly); galvanic isolation constraints apply; shorted terminals must register heat demand within 15 s. — [source: `knowledge/opentherm/opentherm-protocol.md`] (confidence: high, cited)
- ESP32-C6 802.15.4 radio is **250 Kbps**; MQTT is TCP/IP (ESP-MQTT); Thread path to a LAN/IPv4 Mosquitto/HA broker needs a **Border Router** and often **NAT64** for IPv4 brokers. — [source: `knowledge/esp32/esp32-c6-datasheet.md`; https://docs.espressif.com/projects/esp-idf/en/stable/esp32c6/api-reference/protocols/mqtt.html ; https://openthread.io/guides/border-router ; https://openthread.io/codelabs/openthread-border-router-nat64] (confidence: high, cited)
- ESP32-C6 can run OpenThread as a **Standalone Node** (app + stack on-chip); launching BR on ESP requires Wi‑Fi-capable host + RCP. So the gateway device would normally be an **end device (or FTD) on someone else’s Thread mesh**, not the BR. — [source: https://docs.espressif.com/projects/esp-idf/en/stable/esp32c6/api-guides/openthread.html ; https://openthread.io/guides/thread-primer/node-roles-and-types] (confidence: high, cited)
- Native ESP-IDF OT library option exists (Sazanof) but **C6 support is unconfirmed** by upstream README — porting/validation risk for this board. — [source: `knowledge/opentherm/sazanof-esp-idf-opentherm.md`; https://github.com/sazanof/esp-idf-opentherm] (confidence: high, cited)
- Same SoC radio supports Zigbee **or** Thread — concurrent dual use is a platform/design limit not researched here. — [ASSUMPTION] (confidence: low)
- Compliance / safety: heating control is safety-adjacent; incumbent OTGateway explicitly markets “emergency mode” when Wi‑Fi/MQTT/sensors fail. — [source: `knowledge/opentherm/opentherm-protocol.md`; https://github.com/Laxilef/OTGateway] (confidence: medium, cited)
- No volume metrics (install base, boiler SKUs, Thread BR penetration) were available. — [ASSUMPTION] (confidence: low)

## Evidence Against the Idea

- **Crowded Wi‑Fi MQTT + official socket integration**: OTGW-firmware / OTGateway already deliver MQTT↔HA; HA Core already ships `opentherm_gw` over serial/`socket://`. Building another OT↔HA bridge adds little unless OpenThread (or no-Wi‑Fi-at-boiler) is a hard requirement. — [source: https://github.com/rvdbreemen/OTGW-firmware ; https://github.com/Laxilef/OTGateway ; https://www.home-assistant.io/integrations/opentherm_gw/] (confidence: high, cited)
- **Repo already invested in Zigbee bridge design**: constitution and bridge playbooks encode Zigbee endpoints, ZCL routing, and discovery — switching to MQTT/OpenThread may discard or fork that design investment without a clear demand case. — [source: `.specify/memory/constitution.md`, `knowledge/bridge/`] (confidence: high, cited)
- **Hard dependency on external Thread infrastructure**: official OpenThread docs state a Thread network **requires a Border Router** for off-mesh IP; MQTT to a typical IPv4 HA/Mosquitto broker further needs NAT64 or an IPv6-reachable broker. More failure domains than Wi‑Fi MQTT or Zigbee coordinator paths. — [source: https://openthread.io/guides/border-router ; https://openthread.io/codelabs/openthread-border-router-nat64] (confidence: high, cited)
- **MQTT-over-Thread is less proven for this product class**: public artifacts show questions and tooling friction (IPv6 resolution, BR path) rather than a polished OT-gateway reference; GitHub search found no `opentherm`+`thread` repos. — [source: https://github.com/espressif/esp-idf/issues/18355 ; https://github.com/esphome/esphome/issues/13643 ; GitHub Search API] (confidence: medium, cited)
- **No first-party demand evidence** beyond the operator’s stated preference; risk of building for a transport preference rather than an unmet user job. — [source: intake.md] (confidence: medium, cited)

## Gaps & Open Questions

- [NEEDS CLARIFICATION: intended hardware target—confirm WeAct ESP32-C6-A / ESP32-C6, or another board?]
- [NEEDS CLARIFICATION: is this a replacement for, parallel to, or evolution of the documented Zigbee bridge design?]
- [NEEDS CLARIFICATION: which boiler sensor Data IDs and HA commands are in scope for an initial release?]
- [NEEDS CLARIFICATION: MQTT topic model / Home Assistant discovery expectations (MQTT Discovery vs manual YAML)—official HA MQTT Discovery is available if chosen]
- [NEEDS CLARIFICATION: OpenThread role and provisioning (FTD vs MTD/FED; join via external commissioner / BR; who provides the OTBR in the home?)]
- [NEEDS CLARIFICATION: is OpenThread a hard requirement (no Wi‑Fi at install site / Thread-only mesh policy), or a preference vs Wi‑Fi MQTT?]
- [NEEDS CLARIFICATION: where does the MQTT broker live (on-Thread IPv6, LAN IPv6 via BR, or IPv4 via NAT64 / HA Mosquitto add-on)?]
- [NEEDS CLARIFICATION: fail-safe heating behavior when Thread/MQTT/BR is down (compare OTGateway emergency mode)?]
- [NEEDS CLARIFICATION: preferred OpenTherm stack on ESP-IDF—Melnyk (Arduino-oriented, documented) vs Sazanof (native IDF, C6 unconfirmed)?]

## Sources

- `.specify/assessments/opentherm-gateway/intake.md` (host: local, policy: local-artifact)
- `.specify/memory/constitution.md` (host: local, policy: local-artifact)
- `knowledge/bridge/*`, `knowledge/opentherm/*`, `knowledge/esp32/*` (host: local, policy: local-artifact)
- https://github.com/rvdbreemen/OTGW-firmware (host: github.com, policy: allowlisted)
- https://raw.githubusercontent.com/rvdbreemen/OTGW-firmware/dev/README.md (host: raw.githubusercontent.com, policy: allowlisted via github.com content)
- https://github.com/Laxilef/OTGateway (host: github.com, policy: allowlisted)
- https://github.com/ihormelnyk/opentherm_library (host: github.com, policy: allowlisted; also via local knowledge)
- https://github.com/sazanof/esp-idf-opentherm (host: github.com, policy: allowlisted; also via local knowledge)
- https://github.com/espressif/esp-idf/issues/18355 (host: github.com, policy: allowlisted)
- https://github.com/esphome/esphome/issues/13643 (host: github.com, policy: allowlisted)
- https://api.github.com/search/repositories?q=opentherm+mqtt (host: api.github.com, policy: allowlisted)
- https://api.github.com/search/repositories?q=opentherm+thread (host: api.github.com, policy: allowlisted)
- https://www.home-assistant.io/integrations/opentherm_gw/ (host: www.home-assistant.io, policy: confirmed-by-user)
- https://www.home-assistant.io/integrations/mqtt/ (host: www.home-assistant.io, policy: confirmed-by-user)
- https://docs.espressif.com/projects/esp-idf/en/stable/esp32c6/api-guides/openthread.html (host: docs.espressif.com, policy: confirmed-by-user)
- https://docs.espressif.com/projects/esp-idf/en/stable/esp32c6/api-reference/protocols/mqtt.html (host: docs.espressif.com, policy: confirmed-by-user)
- https://openthread.io/guides/border-router (host: openthread.io, policy: confirmed-by-user)
- https://openthread.io/guides/thread-primer/node-roles-and-types (host: openthread.io, policy: confirmed-by-user)
- https://openthread.io/codelabs/openthread-border-router-nat64 (host: openthread.io, policy: confirmed-by-user)
