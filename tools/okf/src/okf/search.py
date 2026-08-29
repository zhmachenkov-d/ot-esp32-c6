"""Frontmatter search — cheap progressive disclosure without loading bodies."""

from __future__ import annotations

from pathlib import Path

from .bundle import load_bundle


def search(root: Path, query: str) -> list[str]:
    """Return compact lines: path | type | title — description (matched on meta + path)."""
    q = query.lower().strip()
    if not q:
        return []
    bundle = load_bundle(root)
    lines: list[str] = []
    for c in bundle.concepts:
        title = str(c.meta.get("title") or "")
        desc = str(c.meta.get("description") or "")
        ctype = str(c.meta.get("type") or "")
        tags = c.meta.get("tags") or []
        tag_s = " ".join(str(t) for t in tags) if isinstance(tags, list) else str(tags)
        hay = " ".join([c.rel, title, desc, ctype, tag_s]).lower()
        if q not in hay:
            continue
        bit = f"{c.rel} | {ctype} | {title}"
        if desc:
            bit += f" — {desc}"
        lines.append(bit)
    return lines
