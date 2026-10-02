# Docs tails: AGENTS tag>stub + GPIO9 strapping (ot-esp32-c6)

## Done
- Triage: два независимых пункта (не «docs hygiene» как одна идея по умолчанию).
- **A KEEP:** в `AGENTS.md` Policy Release-bullet — tag `vX.Y.Z` должен быть **строго >** stub `APP_FW_VERSION`; детали → `firmware/README.md`.
- **B KEEP:** comment только у `APP_SOFTAP_BUTTON_GPIO` в `app_config.h` — GPIO9 = ESP32-C6 **strapping**; early drive / boot levels. Не hardware-remap / «кнопка на плате».
- **Ship:** один маленький chore-PR только A+B (не deferred mega-cleanup).

## Rejected
- Один docs-hygiene «пакет или ничего» как рамка triage.
- Kill/Later для A; Kill/Later для B (фордж предлагал слабый failure mode — пользователь оставил KEEP).
- B = WeAct hardware binding / don't-remap; B в README SoftAP; B только README.
- A только в Running stub-bullet или только Known pitfalls.
- Обязательные два PR; «только A сейчас».

## Surviving cracks
- LED-comment у GPIO8 уже упоминает GPIO9 в boot-mode паре — B-note не копипаста.
- Stub bump после релиза по-прежнему вручную (из прошлого forge; этот срез не чинит).
