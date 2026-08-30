---
description: "Task list for OpenTherm Wi‑Fi MQTT Gateway implementation"
---

# Tasks: OpenTherm Wi‑Fi MQTT Gateway

**Input**: Design documents from `/specs/001-wifi-mqtt-opentherm/`

**Prerequisites**: plan.md, spec.md, research.md, data-model.md, contracts/, quickstart.md

**Tests**: Included — plan Testing section and constitution II require host-side tests for encoding, catalog classification, reject bounds, discovery payloads, and fail-safe FSM before treating behavior done.

**Organization**: Tasks are grouped by user story to enable independent implementation and testing of each story.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[Story]**: Which user story this task belongs to (e.g., US1, US2, US3)
- Include exact file paths in descriptions

## Path Conventions

- ESP-IDF application root: `firmware/`
- App sources: `firmware/main/`
- Host tests: `firmware/tests/host/`
- HIL checklists: `firmware/tests/hil/`

---

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: Greenfield ESP-IDF project skeleton and tooling so firmware and host tests can build

- [ ] T001 Create ESP-IDF project skeleton under `firmware/` (`CMakeLists.txt`, `sdkconfig.defaults`, `main/CMakeLists.txt`, empty `main/main.c`) per plan.md
- [ ] T002 [P] Add IDF Component Manager deps in `firmware/idf_component.yml` for `sazanof/opentherm` (^1.0.3+) and `espressif/mqtt`
- [ ] T003 [P] Create compile-time defaults in `firmware/main/app_config.h` (GPIO in=2 / out=3, SoftAP button GPIO9 ≥5 s, CH min/max 10.0/90.0, boiler-link failure threshold 3, topic root `otc6/`)
- [ ] T004 [P] Scaffold host test harness under `firmware/tests/host/` (CMake/`idf.py` host-test entry so Unity/CMocka suites can run)
- [ ] T005 [P] Add HIL scenario stubs in `firmware/tests/hil/` that mirror quickstart.md V1–V9 checklists

**Checkpoint**: `idf.py set-target esp32c6` and empty app build succeed; host harness is invokable

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: NVS, OpenTherm master/poll/catalog, and MQTT client plumbing that every user story needs

**⚠️ CRITICAL**: No user story work can begin until this phase is complete

- [ ] T006 Implement NVS persistence for credentials, SoftAP CH bounds, device identity, and catalog blob in `firmware/main/nvs_store.c` and `firmware/main/nvs_store.h`
- [ ] T007 [P] Implement OpenTherm f8.8 / typed encode-decode helpers used by poll and MQTT in `firmware/main/ot_codec.c` and `firmware/main/ot_codec.h`
- [ ] T008 Bring up `sazanof/opentherm` as master on GPIO2/3 with documented adapter assumptions in `firmware/main/ot_poll.c` and `firmware/main/ot_poll.h` (init + single exchange API)
- [ ] T009 Implement Data ID 0–127 discovery/classification (READ-ACK / DATA-INVALID → available; UNKNOWN-DATAID → unsupported), **writable flags per research §3 / data-model** (directory write-class + known write-safe set {0,1,…} or safe echo write-probe), and NVS catalog cache in `firmware/main/ot_catalog.c` and `firmware/main/ot_catalog.h`
- [ ] T010 Extend `firmware/main/ot_poll.c` / `firmware/main/ot_poll.h` with ≥1 Hz Status keepalive, 120 ms inter-frame gap, fast/slow/promoted tiers, and a serialized write slot that never drops keepalive
- [ ] T011 Implement ESP-MQTT client with birth/`LWT` on `otc6/<device_id>/status` (`online`/`offline`) in `firmware/main/mqtt_ha.c` and `firmware/main/mqtt_ha.h`
- [ ] T012 Wire boot sequence in `firmware/main/main.c`: load NVS → start OT poll task → connect MQTT when credentials exist (stubs OK for SoftAP/discovery/commands/fail-safe)
- [ ] T013 [P] Add host unit tests for `ot_codec` and catalog classification fixtures (including writable true/false cases: known-safe IDs, non-writable directory class, failed/skipped probe) in `firmware/tests/host/test_ot_codec.c` and `firmware/tests/host/test_ot_catalog.c`
- [ ] T014 Confirm delivered image has no Zigbee/OpenThread features in `firmware/sdkconfig.defaults` and `firmware/CMakeLists.txt` (SC-006)

**Checkpoint**: Foundation ready — OT keepalive runs on device; MQTT can connect with injected NVS credentials; host codec/catalog tests pass

---

## Phase 3: User Story 1 - See boiler Data IDs in Home Assistant (Priority: P1) 🎯 MVP

**Goal**: Operator SoftAP-commissions Wi‑Fi+MQTT; gateway discovers boiler-supported readable Data IDs and publishes MQTT Discovery + live state plus availability and boiler-link health

**Independent Test**: Unconfigured board → SoftAP portal → STA+broker; HA shows availability, boiler-link health, and one entity per readable ID in known set S; values update within 5 s of successful OT read (quickstart V1–V3, V8 partial)

### Tests for User Story 1

- [ ] T015 [P] [US1] Add host tests for MQTT Discovery JSON shape (device block, availability, boiler-link, per-ID sensor) in `firmware/tests/host/test_mqtt_discovery.c`
- [ ] T016 [P] [US1] Add host tests for SoftAP form validation (required Wi‑Fi/MQTT host/port, optional MQTT user/password/TLS, `ch_min_c` < `ch_max_c`) in `firmware/tests/host/test_provision_validate.c`

### Implementation for User Story 1

- [ ] T017 [US1] Implement SoftAP + captive-portal HTTP UI per `contracts/softap-provisioning.md` (Wi‑Fi SSID/password, MQTT host/port/username/password/TLS, CH min/max) and NVS save/exit in `firmware/main/provision_softap.c` and `firmware/main/provision_softap.h`
- [ ] T018 [US1] Implement GPIO9 ≥5 s long-press re-provision (clear Wi‑Fi/MQTT credentials, force SoftAP) in `firmware/main/provision_softap.c`
- [ ] T019 [P] [US1] Implement boiler-link health state machine (threshold 3) publishing `otc6/<device_id>/boiler_link` in `firmware/main/mqtt_ha.c` / `firmware/main/ot_poll.c` as needed
- [ ] T020 [US1] Implement HA MQTT Discovery publisher for meta entities + every catalog-readable Data ID in `firmware/main/mqtt_discovery.c` and `firmware/main/mqtt_discovery.h` per `contracts/mqtt-ha-discovery.md`
- [ ] T021 [US1] Publish retained/live readable Data ID state on `otc6/<device_id>/ot/<N>/state` from poll results in `firmware/main/mqtt_discovery.c` (or thin glue in `firmware/main/mqtt_ha.c`) meeting SC-001
- [ ] T022 [US1] On MQTT reconnect / catalog validation, re-publish discovery configs so HA recovers entities without manual YAML in `firmware/main/mqtt_discovery.c`
- [ ] T023 [US1] Integrate SoftAP → STA → MQTT → catalog validate → discovery into `firmware/main/main.c` end-to-end path
- [ ] T024 [US1] Document SoftAP SSID `OTC6-XXXX` and commissioning steps against quickstart V1 in `firmware/tests/hil/v1_commissioning.md`

**Checkpoint**: User Story 1 independently demonstrable — SoftAP commission yields full readable catalog in HA

---

## Phase 4: User Story 2 - Change writable OpenTherm Data IDs from Home Assistant (Priority: P1)

**Goal**: Every boiler-supported writable Data ID has an HA control; in-range writes reach the boiler; out-of-range CH setpoint is rejected with an explicit signal; CH enable uses Status write when supported

**Independent Test**: From HA, write in-range ID 1 and Status/CH-enable (if in S); reflected state within 2 s; out-of-range ID 1 leaves reflected value unchanged and fires rejection (quickstart V4–V5, V6)

### Tests for User Story 2

- [ ] T025 [P] [US2] Add host tests for SetpointBounds resolution and ID 1 reject-not-clamp in `firmware/tests/host/test_setpoint_bounds.c`
- [ ] T026 [P] [US2] Add host tests for WritableCommand outcomes (`accepted`, `rejected_range`, `rejected_failsafe`, `rejected_link`, `ot_failed`) in `firmware/tests/host/test_mqtt_commands.c`

### Implementation for User Story 2

- [ ] T027 [P] [US2] Implement effective CH min/max (`SetpointBounds`: prefer boiler limit IDs e.g. 57, else SoftAP/NVS) in `firmware/main/ot_catalog.c` or `firmware/main/mqtt_commands.c` with matching header
- [ ] T028 [US2] Implement MQTT command subscriptions for every catalog-writable ID and enqueue serialized OT writes in `firmware/main/mqtt_commands.c` and `firmware/main/mqtt_commands.h`
- [ ] T029 [US2] Publish Discovery for writable controls (`number` / `switch` / etc.) including command topics in `firmware/main/mqtt_discovery.c`
- [ ] T030 [US2] Implement ID 1 out-of-range reject path: no OT write, keep last accepted reflected state, publish explicit rejection on status/event topic per FR-013 in `firmware/main/mqtt_commands.c`
- [ ] T031 [US2] Implement Status (ID 0) CH-enable write path when supported (not only zeroing setpoint) in `firmware/main/mqtt_commands.c` / `firmware/main/ot_poll.c`
- [ ] T032 [US2] Reflect write success/failure to HA state within SC-002 and surface OT-link failures without false success in `firmware/main/mqtt_commands.c`
- [ ] T033 [US2] Ensure bursty HA writes cannot starve ≥1 Hz keepalive (command queue + poll budget) in `firmware/main/ot_poll.c` and verify via `firmware/tests/hil/v6_keepalive_under_load.md`

**Checkpoint**: User Stories 1 and 2 both work — full read/write catalog with reject diagnostics

---

## Phase 5: User Story 3 - Heating stays defined when Home Assistant is unreachable (Priority: P2)

**Goal**: On Wi‑Fi/MQTT loss within 10 s, enter fail-safe: keep OT alive, hold last CH setpoint, refuse remote writes; on recovery, resume cleanly without retained write spirals

**Independent Test**: Drop Wi‑Fi or stop broker during demand; confirm fail-safe within 10 s and OT keepalive; restore link and accept fresh commands (quickstart V7)

### Tests for User Story 3

- [ ] T034 [P] [US3] Add host tests for fail-safe FSM transitions and write refusal in `firmware/tests/host/test_failsafe.c`

### Implementation for User Story 3

- [ ] T035 [US3] Implement fail-safe detection (Wi‑Fi or MQTT down → active within 10 s) in `firmware/main/failsafe.c` and `firmware/main/failsafe.h`
- [ ] T036 [US3] While fail-safe active: continue OT keepalive/polling, hold last accepted CH setpoint on the wire, set `remote_writes_allowed=false` in `firmware/main/failsafe.c` integrated with `firmware/main/ot_poll.c` and `firmware/main/mqtt_commands.c`
- [ ] T037 [US3] On link recovery: debounce, apply at most one retained ID 1 if present, ignore retained storms for other writables, then accept live commands in `firmware/main/failsafe.c` / `firmware/main/mqtt_commands.c`
- [ ] T038 [US3] Wire fail-safe into `firmware/main/main.c` and MQTT availability so HA sees offline/unavailable correctly without inventing heat demand
- [ ] T039 [US3] Add HIL steps for fail-safe and recovery in `firmware/tests/hil/v7_failsafe.md`

**Checkpoint**: All three user stories independently functional

---

## Phase 6: Polish & Cross-Cutting Concerns

**Purpose**: Contract alignment, optional UX, validation pass, governance note

- [x] T040 [P] Verify `specs/001-wifi-mqtt-opentherm/contracts/opentherm-master.md` GPIO table matches WeAct Mini defaults (in=2 / out=3); amend only if drift reappears
- [ ] T041 [P] Optional additive HA `climate` convenience for ID 1 / related status in `firmware/main/mqtt_discovery.c` without removing per-ID entities (FR-002)
- [ ] T042 [P] Add brief `firmware/README.md` with build/flash and pointer to `specs/001-wifi-mqtt-opentherm/quickstart.md`
- [ ] T043 Run full quickstart.md host + HIL validation pass; record results under `firmware/tests/hil/results/` (or checklist sign-off)
- [x] T044 [P] Confirm constitution v2.0.0 Wi‑Fi MQTT wording matches this feature (Zigbee/Thread remain out of scope); no further constitution edit required unless drift returns

---

## Dependencies & Execution Order

### Phase Dependencies

- **Setup (Phase 1)**: No dependencies — start immediately
- **Foundational (Phase 2)**: Depends on Setup — **BLOCKS** all user stories
- **User Story 1 (Phase 3)**: Depends on Foundational — MVP path
- **User Story 2 (Phase 4)**: Depends on Foundational; practically builds on US1 discovery/MQTT plumbing but remains independently testable for write paths once catalog+MQTT exist
- **User Story 3 (Phase 5)**: Depends on Foundational; needs OT hold path and command gate (integrates with US2 command module)
- **Polish (Phase 6)**: After desired user stories complete

### User Story Dependencies

- **User Story 1 (P1)**: After Phase 2 — SoftAP + read discovery/state
- **User Story 2 (P1)**: After Phase 2 — ideally after US1 discovery publisher exists; can stub discovery for write-only HIL if needed
- **User Story 3 (P2)**: After Phase 2 — needs command refusal hook (US2) and OT hold (foundational poll)

### Within Each User Story

- Host tests listed first SHOULD fail before implementation lands
- Models/catalog before MQTT projections
- Core path before HIL docs

### Parallel Opportunities

- Phase 1: T002–T005 in parallel after T001
- Phase 2: T007 parallel with T006; T013 parallel with T011–T012 once APIs exist
- US1: T015–T016 parallel; T019 parallel with T017–T018
- US2: T025–T026 and T027 parallel before command wiring
- US3: T034 parallel with early failsafe skeleton
- Polish: T040, T041, T042, T044 parallel

---

## Parallel Example: User Story 1

```bash
# Host tests in parallel:
Task: "Add host tests for MQTT Discovery JSON in firmware/tests/host/test_mqtt_discovery.c"
Task: "Add host tests for SoftAP form validation in firmware/tests/host/test_provision_validate.c"

# After SoftAP core:
Task: "Implement boiler-link health publishing"
Task: "Implement MQTT Discovery publisher for readable IDs"
```

---

## Parallel Example: User Story 2

```bash
Task: "Host tests for SetpointBounds / reject in firmware/tests/host/test_setpoint_bounds.c"
Task: "Host tests for WritableCommand outcomes in firmware/tests/host/test_mqtt_commands.c"
Task: "Implement SetpointBounds resolution (prefer boiler limit IDs)"
```

---

## Implementation Strategy

### MVP First (User Story 1 Only)

1. Complete Phase 1: Setup
2. Complete Phase 2: Foundational
3. Complete Phase 3: User Story 1
4. **STOP and VALIDATE**: SoftAP → HA readable catalog (quickstart V1–V3)
5. Demo monitoring MVP before writes/fail-safe

### Incremental Delivery

1. Setup + Foundational → OT keepalive + MQTT connect
2. US1 → monitoring MVP
3. US2 → full write catalog + reject signal
4. US3 → fail-safe under link loss
5. Polish → contract GPIO verify, optional climate, quickstart sign-off, constitution alignment check

### Parallel Team Strategy

1. Together: Phase 1–2
2. Then: Dev A US1 SoftAP/discovery; Dev B host tests + ot_codec/catalog hardening; Dev C prepares US2 command stubs against catalog API
3. Integrate US2 → US3 sequentially on shared `mqtt_commands` / `failsafe` modules

---

## Notes

- [P] = different files, no dependency on incomplete sibling tasks
- [USn] maps to spec user stories for traceability
- Target board: WeAct ESP32-C6 Mini; OT adapter GPIO2 in / GPIO3 out; SoftAP button GPIO9
- Secrets only via SoftAP → NVS; never commit credentials
- Zigbee and OpenThread remain out of scope (T014 / T044)
