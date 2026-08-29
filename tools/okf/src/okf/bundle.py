"""Scan an OKF knowledge bundle (markdown + YAML frontmatter)."""

from __future__ import annotations

import re
from dataclasses import dataclass, field
from pathlib import Path

import yaml

RESERVED = frozenset({"index.md", "log.md"})
FM_RE = re.compile(r"\A---\n(.*?)\n---\n?", re.DOTALL)
LINK_RE = re.compile(r"\[([^\]]*)\]\(([^)]+)\)")
HEADING_RE = re.compile(r"^#\s+Citations\s*$", re.MULTILINE)


@dataclass
class Concept:
    path: Path  # absolute
    rel: str  # posix path relative to bundle root
    meta: dict
    body: str
    citations: str = ""


@dataclass
class Bundle:
    root: Path
    concepts: list[Concept] = field(default_factory=list)
    indexes: dict[str, Path] = field(default_factory=dict)  # dir rel → path
    log: Path | None = None

    def by_basename(self) -> dict[str, list[Concept]]:
        out: dict[str, list[Concept]] = {}
        for c in self.concepts:
            out.setdefault(Path(c.rel).name, []).append(c)
        return out


def load_bundle(root: Path) -> Bundle:
    root = root.resolve()
    if not root.is_dir():
        raise FileNotFoundError(f"not a directory: {root}")
    bundle = Bundle(root=root)
    for path in sorted(root.rglob("*.md")):
        if any(p.name == ".git" for p in path.parents):
            continue
        rel = path.relative_to(root).as_posix()
        if path.name == "log.md" and path.parent == root:
            bundle.log = path
            continue
        if path.name == "index.md":
            bundle.indexes[path.parent.relative_to(root).as_posix() or "."] = path
            continue
        text = path.read_text(encoding="utf-8")
        meta, body = _split_frontmatter(text)
        citations = ""
        m = HEADING_RE.search(body)
        if m:
            citations = body[m.end() :].strip()
            body = body[: m.start()].rstrip()
        bundle.concepts.append(
            Concept(path=path, rel=rel, meta=meta or {}, body=body, citations=citations)
        )
    return bundle


def _split_frontmatter(text: str) -> tuple[dict | None, str]:
    m = FM_RE.match(text)
    if not m:
        return None, text
    data = yaml.safe_load(m.group(1)) or {}
    if not isinstance(data, dict):
        data = {"_invalid": True}
    return data, text[m.end() :]


def md_links(text: str) -> list[tuple[str, str]]:
    return [(a, t) for a, t in LINK_RE.findall(text) if not t.startswith(("http://", "https://", "mailto:"))]


def resolve_link(bundle: Bundle, from_rel: str, target: str) -> Path | None:
    """Resolve a markdown link to an existing file (bundle or repo-root), or None."""
    t = target.split("#", 1)[0].strip()
    if not t or t.startswith("`"):
        return None
    # Strip optional surrounding backticks left in some citations
    t = t.strip("`")
    t = t.lstrip("/")
    repo = bundle.root.parent
    candidates = [
        bundle.root / t,
        (bundle.root / Path(from_rel).parent / t),
        repo / t,
        (bundle.root / Path(from_rel).parent / t).resolve(),
    ]
    for c in candidates:
        try:
            c = c.resolve()
        except OSError:
            continue
        if c.is_file():
            return c
    return None
