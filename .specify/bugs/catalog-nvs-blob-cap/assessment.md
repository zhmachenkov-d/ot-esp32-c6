# Bug Assessment: Catalog NVS blob exceeds cap

- **Slug**: catalog-nvs-blob-cap
- **Created**: 2026-08-30
- **Source**: pasted text (Bugbot review, branch changes)
- **Verdict**: valid
- **Severity**: high

## Report (verbatim or summarized)

Bugbot: `ot_catalog_save_nvs` always needs 517 bytes (5-byte header + 128×4-byte entries) but `NVS_CATALOG_BLOB_MAX` is 512, so the save loop returns `ESP_ERR_NO_MEM` and catalog persistence never succeeds. Cited `firmware/main/ot_catalog.c:194-196`.

## Symptom

After catalog discovery/validation, `ot_catalog_save_nvs` fails with `ESP_ERR_NO_MEM`. Catalog never persists to NVS; every boot re-runs full discovery against the boiler.

Expected: a successful compact blob write within the NVS size limit.

## Reproduction

1. Build/flash firmware and let `ot_catalog_discover` complete (or call `ot_catalog_save_nvs` with a full `ot_catalog_t`).
2. Observe return `ESP_ERR_NO_MEM` from the size check in the entry loop (or missing/empty catalog blob in NVS afterward).
3. Confirm `NVS_CATALOG_BLOB_MAX` is 512 while serialized size is 4 + 1 + 128×4 = 517.

## Suspected Code Paths

- `firmware/main/nvs_store.h:20` — `#define NVS_CATALOG_BLOB_MAX 512`
- `firmware/main/ot_catalog.c:177-214` — `ot_catalog_save_nvs` writes 5-byte header then 4 bytes × `OT_CATALOG_MAX_IDS` (128)
- `firmware/main/nvs_store.c:264` — save rejects `len > NVS_CATALOG_BLOB_MAX`

## Root Cause Hypothesis

Header (version u32 + validated flag) is 5 bytes, not 4. The stack buffer and NVS cap were sized at 512, but the format needs 517. Confidence: **high** (arithmetic and code paths match).

## Proposed Remediation

**Preferred**: Raise `NVS_CATALOG_BLOB_MAX` to at least 517 (prefer a round value such as 520 or 544), and keep save/load format unchanged. Add a host-side or compile-time assert that `5 + OT_CATALOG_MAX_IDS * 4 <= NVS_CATALOG_BLOB_MAX`.

**Alternatives**:
- Drop `poll_tier` or pack flags more tightly to fit in 512 — saves flash budget but changes the on-disk format and is more risky.
- Shrink header by packing `validated` into a version nibble — format change, unnecessary if the cap can grow.

**Files likely to change**:
- `firmware/main/nvs_store.h`
- `firmware/main/ot_catalog.c` (optional static assert)
- `firmware/tests/host/` (if catalog NVS round-trip tests exist or are added)

**Tests to add or update**:
- Host unit test: save then load a full catalog; assert `ESP_OK` and byte-for-byte round-trip of flags/`last_raw`/`poll_tier`.

## Risks & Considerations

- Larger NVS blob uses a few extra bytes of NVS partition space (negligible).
- Existing devices with no successful save are unaffected by format growth; any partial/corrupt blobs should already fail load.

## Open Questions

- None; size mismatch is definitive from source.
