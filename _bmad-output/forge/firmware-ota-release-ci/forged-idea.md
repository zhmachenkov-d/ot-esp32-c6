# Firmware release CI (ot-esp32-c6)

## Done
- Automate GitHub Release assets for existing device OTA; **no** firmware OTA feature work.
- **Release workflow:** push tag `vX.Y.Z` → ESP-IDF build (+ size gate) → draft Release → upload `otc6_gateway.bin` + generated `manifest.json` → publish.
- **Version:** tag wins (CI injects into build); binary + manifest match tag.
- **Manifest:** always `version`, `url`, `sha256`, `size`, `release_url` (existing device contract; no hand-edited JSON).
- **Host tests workflow:** every PR to `main` + every push to `main` (not a tag gate).

## Rejected
- Device OTA changes / new OTA channels.
- Release-event trigger (vs tag-push).
- Match-or-fail / honor-system version policies.
- Publish-before-assets.
- Prerelease / suffixed-tag OTA channel.
- Host tests as a release/tag gate; release-pipeline-only “done” (chose wide).

## Surviving cracks
- Tagged releases are not test-gated.
- Local/dev `APP_FW_VERSION` may never match a Release.
