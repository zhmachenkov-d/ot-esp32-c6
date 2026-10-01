# Deferred backlog triage (ot-esp32-c6)

## Done
- «Закрыть весь deferred одним срезом» — **нет**. Triage по пунктам; keeps = отдельные срезы.
- **Keep / next:** OTA **cancel ≠ failed** — cancel/`fail_abort`-from-cancel не ставит `s_failed`; реальные begin/perform/finish errors — ставят. Не clear-on-failsafe-exit, не LED-исключение. Host-test: pure helper `ota_failed_after_download_abort(prior_failed, cancel_caused_abort)` (cancel сохраняет prior; error → true); download task только передаёт два bool. SPEC: `_bmad-output/specs/spec-ota-cancel-neq-failed/`.
- **Keep / after:** release gate — tag SemVer **строго >** stub `APP_FW_VERSION` в `firmware/main/app_config.h`. SPEC: `_bmad-output/specs/spec-release-tag-gt-stub/` (slice 2/3).
- **Keep / last:** короткий note GPIO8 bootstrap/strapping (docs/header), не часть OTA-среза. SPEC: `_bmad-output/specs/spec-gpio8-bootstrap-docs/` (slice 3/3).
- Порядок: cancel ≠ failed → tag>stub → GPIO8 docs.

## Rejected
- Один PR/срез «deferred cleanup».
- Formal exclusive predicates в `bring-up-ladder.md` (контракт = C + host tests).
- Автопроверка placement `status_led_init` vs SoftAP return (cost > value).

## Surviving cracks
- Stub bump после релиза всё ещё вручную; gate не чинит hygiene stub.
