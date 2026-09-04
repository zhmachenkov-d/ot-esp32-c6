# Deferred Work

- source_spec: `/workspaces/ot-esp32-c6/skills/implementation-artifacts/spec-ota-updates.md`
  summary: SoftAP HTTP firmware upload as an alternate OTA channel
  evidence: Split from OTA updates to keep a single remote path (HA Update entity); SoftAP remains provision-only.

- source_spec: `/workspaces/ot-esp32-c6/skills/implementation-artifacts/spec-ota-updates.md`
  summary: Raw MQTT command/status topics for OTA without a Home Assistant Update entity
  evidence: Split — TRIGGER_CHANNEL=B makes HA discovery the control plane; a parallel MQTT-only API would duplicate surface area.

- source_spec: `/workspaces/ot-esp32-c6/skills/implementation-artifacts/spec-ota-updates.md`
  summary: SoftAP/NVS-provisioned OTA CA certificate (vs public CA roots)
  evidence: Split — IMAGE_TRUST=C uses public CA roots for GitHub Releases HTTPS; NVS/SoftAP-provisioned custom OTA CA remains deferred.

- source_spec: `/workspaces/ot-esp32-c6/skills/implementation-artifacts/spec-ota-updates.md`
  summary: In-repo CI/release CDN service for hosting firmware binaries
  evidence: Split — UPDATE_SOURCE=C hosts on GitHub Releases (manifest + bin); a separate in-repo CDN service stays independently shippable / out of scope.

- source_spec: `skills/implementation-artifacts/spec-ota-updates.md`
  summary: Host-test SoftAP start/stop wiring of ota_update_set_softap_active
  evidence: Review defer — provision_softap.c not in host harness; pure SoftAP policy helpers already unit-tested.

- source_spec: `skills/implementation-artifacts/spec-ota-updates.md`
  summary: Execute HIL v10 OTA checklist (e2e Install, TLS fail, power-loss, confirm-timeout rollback)
  evidence: Review defer — hardware-only matrix rows; checklist added but not run this session.

- source_spec: `skills/implementation-artifacts/spec-ota-updates.md`
  summary: Host test covering failsafe activation → ota_update_cancel
  evidence: Review defer — cancel is wired in main.c failsafe path; no host harness for that integration.

- source_spec: `skills/implementation-artifacts/spec-ota-updates.md`
  summary: Harden image_pending_verify when esp_ota_get_state_partition fails
  evidence: Review maybe-false — would be medium if state query can silently skip confirm timeout; needs device confirmation.
