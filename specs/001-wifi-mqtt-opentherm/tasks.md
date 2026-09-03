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

- [X] T001 Create ESP-IDF project skeleton under `firmware/` (`CMakeLists.txt`, `sdkconfig.defaults`, `main/CMakeLists.txt`, empty `main/main.c`) per plan.md
- [X] T002 [P] Add IDF Component Manager deps in `firmware/idf_component.yml` for `sazanof/opentherm` (^1.0.3+) and `espressif/mqtt`
- [X] T003 [P] Create compile-time defaults in `firmware/main/app_config.h` (GPIO in=2 / out=3, SoftAP button GPIO9 ≥5 s, CH min/max 10.0/90.0, DHW/MaxTSet fallbacks for ID 56/57 when ID 48/49 absent, ID 7/14 **0..100**, boiler-link failure threshold 3, **fail-safe entry timer 10 000 ms**, **link-up debounce 2000 ms**, topic root `otc6/`)
- [X] T004 [P] Scaffold host test harness under `firmware/tests/host/` (CMake/`idf.py` host-test entry for **Unity** suites)
- [X] T005 [P] Add HIL scenario stubs in `firmware/tests/hil/` that mirror quickstart.md V1–V9 checklists

**Checkpoint**: `idf.py set-target esp32c6` and empty app build succeed; host harness is invokable

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: NVS, OpenTherm master/poll/catalog, and MQTT client plumbing that every user story needs

**⚠️ CRITICAL**: No user story work can begin until this phase is complete

- [X] T006 Implement NVS persistence for credentials, SoftAP CH bounds, device identity, catalog blob, and **last-accepted CH setpoint** (`last_accepted_ch_setpoint_c` per data-model) in `firmware/main/nvs_store.c` and `firmware/main/nvs_store.h`
- [X] T007 [P] Implement OpenTherm f8.8 / typed encode-decode helpers used by poll and MQTT in `firmware/main/ot_codec.c` and `firmware/main/ot_codec.h`
- [X] T008 Bring up `sazanof/opentherm` as master on GPIO2/3 with documented adapter assumptions in `firmware/main/ot_poll.c` and `firmware/main/ot_poll.h` (init + single exchange API); **C6 validation gate**: confirm framing/IRQ on WeAct Mini. If validation **passes**, mark T008 done and skip T008b. If it **fails**, do **not** treat OT bring-up as done—execute T008b before T009+
- [X] T008b **[Conditional — only if T008 C6 validation fails]** Port Melnyk OpenTherm master timing onto ESP-IDF GPIO + `esp_timer` (pins in=2 / out=3; no Arduino core) behind the same `ot_poll` exchange API in `firmware/main/ot_poll.c` / `firmware/main/ot_poll.h`; document the chosen stack in `firmware/README.md` (or a short note under `firmware/main/`); re-run the C6 framing check before continuing Phase 2
- [X] T009 Implement Data ID 0–127 discovery/classification (READ-ACK / DATA-INVALID → available; UNKNOWN-DATAID → unsupported), **writable flags per research §3 / data-model** (directory write-class + known write-safe set **0, 1, plus IDs listed under `firmware/tests/host/fixtures/`**, or safe echo write-probe; **ID 0 writable = master Status flags via Status READ exchange, never WRITE-DATA(id=0)**), and NVS catalog cache in `firmware/main/ot_catalog.c` and `firmware/main/ot_catalog.h`
- [X] T009b [P] Add host fixtures under `firmware/tests/host/fixtures/` covering OpenTherm mandatory Data IDs **0, 1, 3, 14, 17, 25** (read classification; writable only where directory/write-safe rules allow) plus any bound/write-safe override lists referenced by SetpointBounds
- [X] T010 Extend `firmware/main/ot_poll.c` / `firmware/main/ot_poll.h` with ≥1 Hz Status keepalive, 120 ms inter-frame gap, fast/slow/promoted tiers, and a serialized write slot that never drops keepalive
- [X] T011 Implement ESP-MQTT client with birth/`LWT` on `otc6/<device_id>/status` (`online`/`offline`) in `firmware/main/mqtt_ha.c` and `firmware/main/mqtt_ha.h`; honor NVS `mqtt_tls` (TLS when true; plain TCP when false/default) using broker host/port/username/password from NVS; expose APIs to publish retained birth `online` / force `offline` (and reconnect birth policy). **Ownership (Option A)**: T011 owns birth/LWT plumbing and publish helpers only—**do not** implement the fail-safe entry timer here (that is T035); T038 calls these helpers for timer-running=`online` / active=`offline`
- [X] T012 Wire boot sequence in `firmware/main/main.c`: load NVS → start OT poll task → connect MQTT when credentials exist (stubs OK for SoftAP/discovery/commands/fail-safe)
- [X] T013 [P] Add host unit tests for `ot_codec` and catalog classification fixtures (including writable true/false cases: known-safe IDs, non-writable directory class, failed/skipped probe; load from `firmware/tests/host/fixtures/`) in `firmware/tests/host/test_ot_codec.c` and `firmware/tests/host/test_ot_catalog.c`
- [X] T014 Confirm delivered image has no Zigbee/OpenThread features in `firmware/sdkconfig.defaults` and `firmware/CMakeLists.txt` (SC-006)

**Checkpoint**: Foundation ready — OT keepalive runs on device (C6 framing validated on `sazanof/opentherm`, or Melnyk-port T008b completed); MQTT can connect with injected NVS credentials; host codec/catalog tests pass

---

## Phase 3: User Story 1 - See boiler Data IDs in Home Assistant (Priority: P1) 🎯 MVP

**Goal**: Operator SoftAP-commissions Wi‑Fi+MQTT; gateway discovers boiler-supported readable Data IDs and publishes MQTT Discovery + live state plus availability and boiler-link health

**Independent Test**: Unconfigured board → SoftAP portal → STA+broker; HA shows availability, boiler-link health, and one entity per readable ID in known set S; values update within 5 s of successful OT read (quickstart V1–V3, V8 partial)

### Tests for User Story 1

- [X] T015 [P] [US1] Add host tests for MQTT Discovery JSON shape (device block, availability, boiler-link, per-ID sensor) in `firmware/tests/host/test_mqtt_discovery.c`
- [X] T015b [P] [US1] Add host tests for Data ID 0 Status flag additive discovery/state projections (`binary_sensor` / `switch` for **fault, CH active, DHW active, flame, CH enable** only; ID 0 entity still present) in `firmware/tests/host/test_status_projections.c` (fixtures under `firmware/tests/host/fixtures/` aligned with `knowledge/opentherm/data-id-0-status.md`)
- [X] T016 [P] [US1] Add host tests for SoftAP form validation (required Wi‑Fi/MQTT host/port, optional MQTT user/password/TLS, `ch_min_c` < `ch_max_c`) in `firmware/tests/host/test_provision_validate.c`

### Implementation for User Story 1

- [X] T017 [US1] Implement SoftAP + captive-portal HTTP UI per `contracts/softap-provisioning.md` (**open** SoftAP; Wi‑Fi SSID/password, MQTT host/port/username/password/TLS, CH min/max) and NVS save/exit in `firmware/main/provision_softap.c` and `firmware/main/provision_softap.h`
- [X] T018 [US1] Implement GPIO9 ≥5 s long-press re-provision (clear Wi‑Fi/MQTT credentials, force SoftAP) in `firmware/main/provision_softap.c`
- [X] T019 [P] [US1] Implement boiler-link health state machine (unhealthy after 3 consecutive **keepalive/status** failures only; **healthy after one successful** keepalive/status exchange; tiered catalog reads do not count) publishing `otc6/<device_id>/boiler_link` in `firmware/main/mqtt_ha.c` / `firmware/main/ot_poll.c` as needed
- [X] T020 [US1] Implement HA MQTT Discovery publisher for meta entities + every catalog-readable Data ID in `firmware/main/mqtt_discovery.c` and `firmware/main/mqtt_discovery.h` per `contracts/mqtt-ha-discovery.md` (no custom HA Core integration — FR-009)
- [X] T021 [US1] Publish retained/live readable Data ID state on `otc6/<device_id>/ot/<N>/state` from poll results in `firmware/main/mqtt_discovery.c` (or thin glue in `firmware/main/mqtt_ha.c`) meeting SC-001; use **empty-string** state when no valid sample (contracts unavailable sentinels)
- [X] T021b [US1] When Data ID 0 is catalog-available, publish additive HA `binary_sensor` (and writable `switch` for master **CH enable**) projections for Status flag8 bits **fault, CH active, DHW active, flame** (slave) and **CH enable** (master) **without** removing the underlying ID 0 entity, in `firmware/main/mqtt_discovery.c` per FR-002 / data-model Encoding notes / `knowledge/opentherm/data-id-0-status.md` (depends on T020/T021 discovery + state paths in the same files—not parallel)
- [X] T022 [US1] On MQTT reconnect / catalog validation, re-publish discovery configs so HA recovers entities without manual YAML in `firmware/main/mqtt_discovery.c`
- [X] T023 [US1] Integrate SoftAP → STA → MQTT → catalog validate → discovery into `firmware/main/main.c` end-to-end path; after first successful OT+MQTT link, run `idf.py size` (or equivalent) and note app `.bin` vs soft budgets (≤1.5 MiB / warn if >1.2 MiB) so size regressions surface before polish T043
- [X] T024 [US1] Document SoftAP SSID `OTC6-XXXX` and commissioning steps against quickstart V1 in `firmware/tests/hil/v1_commissioning.md`
- [X] T024b [P] [US1] Add HIL steps for boiler-link vs MQTT availability (quickstart V8) in `firmware/tests/hil/v8_boiler_link.md`

**Checkpoint**: User Story 1 independently demonstrable — SoftAP commission yields full readable catalog in HA

---

## Phase 4: User Story 2 - Change writable OpenTherm Data IDs from Home Assistant (Priority: P1)

**Goal**: Every boiler-supported writable Data ID has an HA control; in-range writes reach the boiler; out-of-range CH setpoint is rejected with an explicit signal; CH enable uses Status **exchange** (master flags) when supported—not `WRITE-DATA(id=0)`

**Independent Test**: From HA, write in-range ID 1 and Status/CH-enable (if in S); reflected state within 2 s; out-of-range ID 1 leaves reflected value unchanged and fires rejection (quickstart V4–V5, V6)

### Tests for User Story 2

- [X] T025 [P] [US2] Add host tests for SetpointBounds / range resolution and reject-not-clamp for v1 range-checked IDs (**1, 8** if available, **7, 14, 56, 57**) in `firmware/tests/host/test_setpoint_bounds.c`
- [X] T026 [P] [US2] Add host tests for WritableCommand outcomes (`accepted`, `rejected_range`, `rejected_failsafe`, `ot_failed`) including **outcome→wire `reason` mapping** (`rejected_range`→`out_of_range`; `rejected_failsafe`/`ot_failed` unchanged), **boiler-link unhealthy still attempts write** (success→`accepted` / fail→`ot_failed` + `ot/<N>/rejection`, never a link pre-reject; writes do not clear BoilerLink counters), and **rejection topic for every writable N** in `firmware/tests/host/test_mqtt_commands.c`

### Implementation for User Story 2

- [X] T027 [P] [US2] Implement effective bounds for v1 range-checked IDs in `firmware/main/ot_catalog.c` / `firmware/main/ot_catalog.h` (owner module; `mqtt_commands` consumes the API): ID **1**/**8** via `SetpointBounds` (prefer boiler max-limit ID e.g. 57 for max; **v1 min = SoftAP/NVS** unless fixture min-limit); ID **56**←ID 48; ID **57**←ID 49; ID **7**/**14**←0..100 unless fixture override
- [X] T028 [US2] Implement MQTT command subscriptions for every catalog-writable ID and enqueue serialized OT writes in `firmware/main/mqtt_commands.c` and `firmware/main/mqtt_commands.h`
- [X] T029 [US2] Publish Discovery for writable controls (`number` / `switch` / etc.) including command topics in `firmware/main/mqtt_discovery.c`
- [X] T030 [US2] Implement per-ID rejection path in `firmware/main/mqtt_commands.c`: on any non-success outcome publish JSON on `otc6/<device_id>/ot/<N>/rejection` per `contracts/mqtt-ha-discovery.md` — map `rejected_range`→`reason=out_of_range`, `rejected_failsafe`→`rejected_failsafe`, `ot_failed`→`ot_failed` (range-checked out-of-range keeps last accepted reflected state; **same topic pattern for every writable N** — FR-004/013); optional HA `event`/diagnostic discovery MAY be added later without replacing the topic
- [X] T031 [US2] Implement Status (ID 0) CH-enable command path when supported: update pending master Status flags and apply on next Status **`READ-DATA(id=0)`** exchange (not only zeroing setpoint; **never** `WRITE-DATA(id=0)`) in `firmware/main/mqtt_commands.c` / `firmware/main/ot_poll.c`
- [X] T032 [US2] Reflect write success to HA state within SC-002; on failure publish `ot/<N>/rejection` and leave reflected state unchanged (no false success) in `firmware/main/mqtt_commands.c`
- [X] T033 [US2] Ensure bursty HA writes cannot starve ≥1 Hz keepalive (command queue + poll budget) in `firmware/main/ot_poll.c` and verify via `firmware/tests/hil/v6_keepalive_under_load.md`

**Checkpoint**: User Stories 1 and 2 both work — full read/write catalog with reject diagnostics

---

## Phase 5: User Story 3 - Heating stays defined when Home Assistant is unreachable (Priority: P2)

**Goal**: On Wi‑Fi/MQTT loss, after the **10 000 ms** entry timer, enter fail-safe: keep OT alive, hold last CH setpoint, refuse remote writes; on recovery, resume cleanly without retained write spirals

**Independent Test**: Drop Wi‑Fi or stop broker during demand; confirm fail-safe after **10 000 ms** entry timer and OT keepalive; restore link and accept fresh commands (quickstart V7)

### Tests for User Story 3

- [X] T034 [P] [US3] Add host tests for fail-safe FSM transitions, write refusal, and **Option A availability**: entry timer running → treat app availability as `online` / writes allowed; `active` → present as `offline` (helper called or LWT/birth policy stub) + `remote_writes_allowed=false` in `firmware/tests/host/test_failsafe.c`

### Implementation for User Story 3

- [X] T035 [US3] Implement fail-safe detection in `firmware/main/failsafe.c` and `firmware/main/failsafe.h`: Wi‑Fi STA disconnect / lost-IP **or** MQTT disconnect/client error starts the **10 000 ms** entry timer from `app_config.h`; enter fail-safe when the timer expires while link still down (SC-004); clear/cancel the timer on Wi‑Fi+MQTT healthy again; **remote writes remain allowed until fail-safe becomes active**. **Ownership (Option A)**: T035 owns the entry timer and `active` / `remote_writes_allowed` state only—call T011 publish helpers from T038 for availability (FR-006)
- [X] T036 [US3] While fail-safe active: continue OT keepalive/polling, hold last accepted CH setpoint on the wire, set `remote_writes_allowed=false` in `firmware/main/failsafe.c` integrated with `firmware/main/ot_poll.c` and `firmware/main/mqtt_commands.c`
- [X] T037 [US3] On link recovery: wait **2 s** continuous Wi‑Fi+MQTT healthy (link-up debounce from `app_config.h`), apply at most one retained ID 1 if present, ignore retained storms for other writables, then accept live commands in `firmware/main/failsafe.c` / `firmware/main/mqtt_commands.c`
- [X] T038 [US3] Wire fail-safe into `firmware/main/main.c`: on T035 transitions call T011 helpers so **active** → application MQTT presents as **`offline`** (retained publish when possible; LWT + next-connect birth `offline` when publish path already dead) and **entry timer running** → keep/publish **`online`** (Option A)—not `online` with silent write drops; no invented heat demand; command refusals publish `ot/<N>/rejection` with `reason=rejected_failsafe`
- [X] T039 [US3] Add HIL steps for fail-safe and recovery in `firmware/tests/hil/v7_failsafe.md`

**Checkpoint**: All three user stories independently functional

---

## Phase 6: Polish & Cross-Cutting Concerns

**Purpose**: Contract alignment, optional UX, validation pass, governance note

- [x] T040 [P] **(Documentation gate — complete pre-firmware; not a feature-done signal)** Verify `specs/001-wifi-mqtt-opentherm/contracts/opentherm-master.md` GPIO table matches WeAct Mini defaults (in=2 / out=3); amend only if drift reappears
- [X] T041 [P] Optional additive HA `climate` convenience for ID 1 / related status in `firmware/main/mqtt_discovery.c` without removing per-ID entities (FR-002)
- [X] T042 [P] Add brief `firmware/README.md` with build/flash and pointer to `specs/001-wifi-mqtt-opentherm/quickstart.md`
- [X] T043 Run full quickstart.md host + HIL validation pass (V1–V9 including V8); record results under `firmware/tests/hil/results/` (or checklist sign-off); include `idf.py size` (or equivalent) and confirm soft budgets: app `.bin` ≤ 1.5 MiB, free heap after OT+MQTT ≥ 64 KiB (plan Constraints)
- [x] T044 [P] **(Documentation gate — complete pre-firmware; not a feature-done signal)** Confirm constitution v2.0.0 Wi‑Fi MQTT wording matches this feature (Zigbee/Thread remain out of scope); no further constitution edit required unless drift returns

---

## Dependencies & Execution Order

### Phase Dependencies

- **Setup (Phase 1)**: No dependencies — start immediately
- **Foundational (Phase 2)**: Depends on Setup — **BLOCKS** all user stories
- **User Story 1 (Phase 3)**: Depends on Foundational — MVP path
- **User Story 2 (Phase 4)**: Depends on Foundational; practically builds on US1 discovery/MQTT plumbing but remains independently testable for write paths once catalog+MQTT exist
- **User Story 3 (Phase 5)**: Depends on Foundational **and User Story 2 command gate** (`mqtt_commands` + `remote_writes_allowed` / fail-safe refusal)—needs OT hold path (foundational poll) plus US2 write path to refuse/recover cleanly
- **Polish (Phase 6)**: After desired user stories complete

### User Story Dependencies

- **User Story 1 (P1)**: After Phase 2 — SoftAP + read discovery/state
- **User Story 2 (P1)**: After Phase 2 — ideally after US1 discovery publisher exists; can stub discovery for write-only HIL if needed
- **User Story 3 (P2)**: After Phase 2 **and after US2 command module** (T028/T036 integration)—command refusal hook + OT hold (foundational poll)

### Within Each User Story

- Host tests listed first SHOULD fail before implementation lands
- Models/catalog before MQTT projections
- Core path before HIL docs

### Parallel Opportunities

- Phase 1: T002–T005 in parallel after T001
- Phase 2: T007 parallel with T006; T009b parallel with T009; T013 parallel with T011–T012 once APIs exist; T008b only after T008 fails (sequential, not parallel)
- US1: T015–T016 parallel (incl. T015b); T019 parallel with T017–T018; T021b **after** T020/T021 (same files); T024b parallel with T024
- US2: T025–T026 and T027 parallel before command wiring
- US3: T034 parallel with early failsafe skeleton
- Polish: T040, T041, T042, T044 parallel

---

## Parallel Example: User Story 1

```bash
# Host tests in parallel:
Task: "Add host tests for MQTT Discovery JSON in firmware/tests/host/test_mqtt_discovery.c"
Task: "Add host tests for Status flag projections in firmware/tests/host/test_status_projections.c"
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
- SC-001 / SC-002 latency floors are measured in HIL (quickstart V3/V4 via T043); **no host wall-clock assert** is required in unit tests
- T040 / T044 are documentation alignment gates already checked off; Phase 1–5 firmware work remains open
- v1 range-checked writables: **1, 8** (if available), **7, 14, 56, 57** (FR-013)

---

## Phase 7: Convergence

**Purpose**: Close gaps between checked-off tasks and current code vs spec/plan/constitution

- [X] T045 CRITICAL Reflect OpenTherm write exchange success/failure before publishing accepted HA state; on bus failure publish `ot/<N>/rejection` with `reason=ot_failed` and leave reflected state unchanged (no enqueue-as-success) in `firmware/main/mqtt_commands.c` and `firmware/main/ot_poll.c`; add host tests that fail without completion wiring per FR-004, US2/AC3, Constitution II (contradicts)
- [X] T046 Implement post-recovery retained-write policy: after 2 s link-up debounce apply at most one retained `ot/1/set` if present and ignore retained storms for other writables in `firmware/main/mqtt_commands.c` / `firmware/main/main.c` / `firmware/main/failsafe.c`; add host tests per US3/AC3, T037, `contracts/mqtt-ha-discovery.md` (missing)
- [X] T047 On fail-safe entry, hold the live last-accepted CH setpoint from NVS (or shared store updated by `nvs_store_set_last_ch_setpoint`), not the boot-time `s_cfg` copy in `firmware/main/main.c`, per FR-006 (partial)
- [X] T048 Complete remaining T043 evidence: measure and record free heap after OT+MQTT (≥ 64 KiB) and sign off open HIL V1–V9 under `firmware/tests/hil/results/` per plan Constraints / SC-001–SC-005 (partial)
- [X] T049 Wire climate `mode` command/state topics to Status CH-enable (or remove dead `climate/mode/{set,state}` from discovery) in `firmware/main/mqtt_discovery.c` / `firmware/main/mqtt_commands.c` per T041 (partial)
- [X] T050 Compile production SoftAP `provision_validate` into host tests instead of the duplicated stub in `firmware/tests/host/host_provision_validate.c` per Constitution II, T016 (partial)

---

## Phase 8: Convergence

**Purpose**: Close remaining gaps between checked-off Phase 7 work and current code vs spec/plan/constitution

- [X] T051 Wire retained-write gate + **2 s** link-up debounce on **every** MQTT (re)connect / first subscribe—not only fail-safe clear—so at most one retained `ot/1/set` applies and other retained `ot/<N>/set` are dropped in `firmware/main/main.c` / `firmware/main/mqtt_commands.c` / `firmware/main/mqtt_ha.c`; extend host tests per US3/AC3, Assumptions, `contracts/mqtt-ha-discovery.md` Retained writes, T037/T046 (partial)
- [X] T052 Reflect Status CH-enable (ID 0) only after the next keepalive/status `READ-DATA(id=0)` succeeds; on failure publish `ot/0/rejection` with `reason=ot_failed` and leave reflected state unchanged (no immediate accept) in `firmware/main/mqtt_commands.c` / `firmware/main/ot_poll.c`; update host tests that currently assert immediate `ACCEPTED` per FR-004, US2/AC3, US2/AC4, Constitution II (contradicts)
- [X] T053 Re-publish MQTT Discovery configs (and additive climate if still enabled) on plain MQTT reconnect / `MQTT_EVENT_CONNECTED`, not only boot and fail-safe recovery, in `firmware/main/mqtt_ha.c` / `firmware/main/main.c` / `firmware/main/mqtt_discovery.c` per T022, research § discovery, US1/AC4 (partial)
- [X] T054 Update catalog `ids[0].last_raw` / `has_raw` from keepalive Status exchanges and stop publishing CH-enable accept as `"0"`/`"1"` on `ot/0/state` (keep raw Status on the ID 0 entity; projections/climate carry CH enable) in `firmware/main/ot_poll.c` / `firmware/main/mqtt_commands.c` per FR-002, FR-003, Constitution III (partial)
- [X] T055 Stop advertising HA `number` min/max **0..100** for non–range-checked writables (omit bounds or use encoding-appropriate limits) so HA UI does not invent a gateway range gate in `firmware/main/mqtt_discovery.c` per FR-013 (partial)

---

## Phase 9: Convergence

**Purpose**: Close remaining gaps between checked-off Phase 8 work and current code vs spec/plan/constitution

- [X] T056 Expand OpenTherm directory write-class in `firmware/main/ot_catalog.c` (and `firmware/tests/host/fixtures/`) beyond the hard-coded subset **0,1,7,8,14,56,57** so boiler-supported W/R-W IDs (e.g. **16**, **2**, **4**, **23**, **24**, **58**, and other directory master-write IDs from `knowledge/opentherm/opentherm-data-ids.md`) enter the safe echo write-probe path and expose HA write controls when probe ACK succeeds per FR-002, FR-015, SC-007, research §3 (partial)
- [X] T057 Complete remaining T043/T048 evidence: capture measured free heap after OT+MQTT (≥ 64 KiB) on WeAct Mini and sign off HIL V1–V8 under `firmware/tests/hil/results/` (or record explicit hardware deferral with owner) per plan Constraints / SC-001–SC-005 (partial)
- [X] T058 Add host tests for boiler-link consecutive-fail FSM (unhealthy after 3 keepalive/status failures only; healthy after one success; tiered catalog reads do not count) covering `firmware/main/ot_poll.c` `note_keepalive` behavior per FR-012, Constitution II (partial)
- [X] T059 Fix `firmware/README.md` SoftAP security wording from **(open)** to **WPA2-PSK** (+ Setup PIN pointer) to match `contracts/softap-provisioning.md` and `provision_softap.c` (contradicts)

---

## Phase 10: Convergence

**Purpose**: Close remaining gaps between checked-off Phase 9 work and current code vs spec/plan/constitution

- [ ] T060 Drive MQTT state decode and WRITE-DATA encode from OpenTherm directory encoding class (`f8.8` / flag8 / packed u8 / …) instead of hardcoded ID allowlists in `firmware/main/main.c`, `firmware/main/mqtt_discovery.c`, and `firmware/main/mqtt_commands.c`; cover mandatory ID **17** and directory-writable f8.8 IDs (e.g. **16**, **23**, **24**, **58**); add host tests per FR-003, FR-004, US1/AC3, US2/AC1, data-model Encoding notes, Constitution II (partial)
- [ ] T061 Align `firmware/tests/hil/v1_commissioning.md` SoftAP steps and expect with **WPA2-PSK** + serial **Setup PIN** (not open SoftAP) per FR-005, `contracts/softap-provisioning.md` (contradicts)
