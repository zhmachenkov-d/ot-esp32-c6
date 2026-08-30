# Bug Fix: Catalog NVS blob exceeds cap

- **Slug**: catalog-nvs-blob-cap
- **Fixed**: 2026-08-30
- **Assessment**: ./assessment.md
- **Status**: applied

## Summary

Raised `NVS_CATALOG_BLOB_MAX` from 512 to 520 so the 517-byte compact catalog blob fits, added a compile-time size assert, and pinned the fix with a full-catalog NVS round-trip host test.

## Changes

| File | Change | Notes |
|------|--------|-------|
| `firmware/main/nvs_store.h` | modified | `NVS_CATALOG_BLOB_MAX` 512 → 520 |
| `firmware/tests/host/stubs/nvs_store.h` | modified | Same cap for host stub |
| `firmware/main/ot_catalog.c` | modified | `_Static_assert(5 + OT_CATALOG_MAX_IDS * 4 <= NVS_CATALOG_BLOB_MAX)` |
| `firmware/tests/host/test_ot_catalog.c` | modified | Added `test_catalog_nvs_full_round_trip` |

## Diff Highlights (optional)

```c
#define NVS_CATALOG_BLOB_MAX    520

_Static_assert(5 + OT_CATALOG_MAX_IDS * 4 <= NVS_CATALOG_BLOB_MAX,
               "NVS_CATALOG_BLOB_MAX too small for catalog blob");
```

## Tests Added or Updated

- `firmware/tests/host/test_ot_catalog.c::test_catalog_nvs_full_round_trip` — save then load a full 128-entry catalog; asserts `ESP_OK` and round-trip of version, validated, support, writable, has_raw, last_raw, poll_tier

## Local Verification

- Commands run: `firmware/tests/host/./run.sh` → 8/8 passed (including `test_ot_catalog`)
- Manual checks: none (host unit coverage sufficient for this size-cap bug)

## Deviations from Assessment

None.

## Follow-ups

- Run `/speckit-bug-test slug=catalog-nvs-blob-cap` for formal verification recording.
- On-device smoke: after discovery, confirm catalog blob is present in NVS and survives reboot without re-discovery.
