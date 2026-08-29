"""Heuristic + autofix lint over a knowledge bundle."""

from __future__ import annotations

import re
from pathlib import Path

from .bundle import load_bundle, md_links, resolve_link
from .index_cmd import write_indexes
from .validate import Finding, validate

BASENAME_LINK = re.compile(r"\]\((/?([^/#)]+\.md))(#[^)]*)?\)")


def lint(root: Path, *, fix: bool = False) -> tuple[list[Finding], int]:
    """Run validate + orphan checks. If fix, rewrite indexes and unique basename links.

    Returns (findings after work, autofix_count).
    """
    fixes = 0
    if fix:
        fixes += len(write_indexes(root))
        bundle = load_bundle(root)
        by_base = bundle.by_basename()
        for c in bundle.concepts:
            text = c.path.read_text(encoding="utf-8")
            new, n = _rewrite_unique_basenames(text, by_base, bundle.root)
            if n:
                c.path.write_text(new, encoding="utf-8")
                fixes += n

    findings = validate(root)
    findings.extend(_orphan_findings(root))
    return findings, fixes


def _bundle_rel(bundle_root: Path, hit: Path) -> str | None:
    try:
        return hit.resolve().relative_to(bundle_root.resolve()).as_posix()
    except ValueError:
        return None


def _orphan_findings(root: Path) -> list[Finding]:
    bundle = load_bundle(root)
    inbound: set[str] = set()
    for idx in bundle.indexes.values():
        text = idx.read_text(encoding="utf-8")
        from_rel = idx.relative_to(bundle.root).as_posix()
        for _a, t in md_links(text):
            hit = resolve_link(bundle, from_rel, t)
            if hit and (rel := _bundle_rel(bundle.root, hit)):
                inbound.add(rel)
    for c in bundle.concepts:
        full = c.body + ("\n" + c.citations if c.citations else "")
        for _a, t in md_links(full):
            hit = resolve_link(bundle, c.rel, t)
            if hit and (rel := _bundle_rel(bundle.root, hit)):
                inbound.add(rel)
    return [
        Finding("warning", "orphan", c.rel, "no inbound links from indexes/concepts")
        for c in bundle.concepts
        if c.rel not in inbound
    ]


def _rewrite_unique_basenames(text: str, by_base: dict, root: Path) -> tuple[str, int]:
    count = 0

    def repl(m: re.Match) -> str:
        nonlocal count
        full = m.group(1)
        name = Path(m.group(2)).name
        frag = m.group(3) or ""
        if (root / full.lstrip("/")).is_file():
            return m.group(0)
        matches = by_base.get(name, [])
        if len(matches) != 1:
            return m.group(0)
        target = matches[0].rel
        count += 1
        return f"](/{target}{frag})"

    return BASENAME_LINK.sub(repl, text), count
