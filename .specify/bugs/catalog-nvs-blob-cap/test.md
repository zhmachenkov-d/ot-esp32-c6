# Bug Verification: Catalog NVS blob exceeds cap

- **Slug**: catalog-nvs-blob-cap
- **Tested**: 2026-08-30
- **Assessment**: ./assessment.md
- **Fix**: ./fix.md
- **Result**: verified

## Summary

Post-fix, a full catalog save no longer returns `ESP_ERR_NO_MEM`: host `test_catalog_nvs_full_round_trip` gets `ESP_OK` and round-trips all 128 entries. Cap is 520 (≥ 517); compile-time assert is present. No host regressions.

## Checks Performed

| Check | Command / Action | Result | Notes |
|-------|------------------|--------|-------|
| Reproduction (post-fix) | `./build/test_ot_catalog` (`test_catalog_nvs_full_round_trip`) | pass | Assessment allows calling `ot_catalog_save_nvs` with a full catalog; save returns `ESP_OK`, load matches |
| Cap / arithmetic | Inspect `NVS_CATALOG_BLOB_MAX` + `5 + 128*4` | pass | Main + host stub = 520; need 517 ≤ 520 |
| Static assert | Present in `ot_catalog.c` | pass | `_Static_assert(5 + OT_CATALOG_MAX_IDS * 4 <= NVS_CATALOG_BLOB_MAX)` |
| New / updated tests | `./build/test_ot_catalog` | pass | 7/7 including round-trip |
| Regression suite | `firmware/tests/host/./run.sh` | pass | 8/8 host tests |
| Lint / type-check | — | skipped | No project lint gate for these headers/C sources |
| On-device flash / reboot | Flash + discover + NVS inspect | skipped | Needs hardware; assessment’s host save path covers the size-cap failure |

## Output Excerpts

```
test_catalog_nvs_full_round_trip:PASS
7 Tests 0 Failures 0 Ignored
OK
```

```
100% tests passed, 0 tests failed out of 8
```

## Residual Risks

- On-device persistence across reboot was not exercised here; host stub NVS is in-memory only.
- Real `nvs_set_blob` partition budget was not re-measured on target (520 bytes is still negligible).

## Recommendation

Close the bug — verified via the assessment’s automated reproduction path (`ot_catalog_save_nvs` on a full catalog) plus host regression. Optional follow-up: one-device smoke that catalog survives reboot without re-discovery.
