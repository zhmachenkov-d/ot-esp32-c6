---
name: okf-knowledge
description: Build and maintain the project's OKF knowledge bundle under knowledge/ from immutable wiki/raw/ sources. Use for ingest, query, lint, OKF bundle maintenance, or when the user mentions knowledge/, Open Knowledge Format, or OKF concepts.
---

# OKF Knowledge

Compile and maintain the agent-owned **Knowledge bundle** under `knowledge/` from immutable **Raw source** files in `wiki/raw/`. Humans curate sources and ask questions; you compile concepts, update cross-links, and run tooling.

Prefer **Python tooling** over reading the bundle into context. From repo root (devcontainer installs `okf` via `uv tool install -e tools/okf`):

| Need | Command |
|------|---------|
| Refresh indexes | `okf index knowledge/` |
| Conformance | `okf validate knowledge/` |
| Lint (+ autofix) | `okf lint knowledge/` · `--fix` |
| Find concepts | `okf search knowledge/ '<query>'` |
| Export / graph | `okf export knowledge/ -o dist/` · `okf visualize knowledge/` |

After writes to `knowledge/`, run `index` then `validate`.

## Architecture

| Layer | Path | Role |
|-------|------|------|
| Raw source | `wiki/raw/` | Immutable inputs. Read only. |
| Knowledge bundle | `knowledge/` | Compiled concepts, `index.md`, `log.md`. You own this. |
| Tooling | `tools/okf/` | Python package above. |

**Concept types (T1):** `Protocol`, `Reference`, `Hardware`, `Data ID`, `Attribute`, `Playbook`

**Frontmatter** (required `type`; list `raw:` to repo-root paths; `# Citations` at bottom mirroring `raw:` + URLs):

```yaml
---
type: Protocol
title: Human-readable title
description: One-line summary for indexes.
tags: [opentherm]
timestamp: 2026-07-02T00:00:00Z
raw:
  - wiki/raw/opentherm/2003-02-07-opentherm-protocol-v2-2.md
---
```

**Links:** bundle-absolute in concepts: `[Data ID 1 TSet](/opentherm/data-id-1-tset.md)`. In chat, cite `knowledge/opentherm/data-id-1-tset.md`.

## Initialization

On first ingest only, create what is missing: `knowledge/index.md`, `knowledge/log.md` (`# Directory Update Log`), topic dirs. On query/lint without a bundle, tell the user to ingest first.

## Ingest

Always: fetch → `wiki/raw/`, then compile → `knowledge/`.

**Fetch:** English if needed; reuse close `wiki/raw/<topic>/`; save `YYYY-MM-DD-descriptive-slug.md` (drop date if unknown); preserve text, clean formatting noise only.

**Compile (G2 medium):** split major reference areas into linked concepts. Third-party → topic dir; project mappings/procedures → `bridge/` as `Playbook`. Then cross-link, append `knowledge/log.md` (newest ISO date first), `okf index knowledge/` + `okf validate`.

```markdown
## 2026-07-02
* **Creation**: Added [Data ID 1 TSet](/opentherm/data-id-1-tset.md).
* **Update**: Linked [OpenTherm Frame Format](/opentherm/opentherm-frame-format.md) to timing rules.
```

## Query

1. `okf search knowledge/ '<terms>'` — shortlist paths (do not load the whole tree).
2. Read only the hit concepts; prefer bundle over training knowledge.
3. Cite project-root paths. Write files only if asked to archive.

**Archive (when asked):** new concept (`Reference`/`Playbook`), log, `index` + `validate`.

## Lint

```bash
okf lint knowledge/
okf lint knowledge/ --fix
```

Tooling covers conformance, raw provenance, citation mirroring, index staleness, broken links, orphans; `--fix` regenerates indexes and rewrites links when exactly one basename matches. You still do heuristic review (contradictions, stale vs newer raw, missing G2 splits, prose mentions without a page) — report only.

```markdown
## 2026-07-02
* **Lint**: 2 warnings, 1 auto-fixed link.
```
