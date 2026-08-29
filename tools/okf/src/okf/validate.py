"""Conformance + project checks for a knowledge bundle."""

from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path

from .bundle import Bundle, load_bundle, md_links, resolve_link
from .index_cmd import build_indexes


@dataclass
class Finding:
    severity: str  # error | warning
    rule: str
    path: str
    message: str

    def format(self) -> str:
        return f"{self.severity}: [{self.rule}] {self.path}: {self.message}"


def validate(root: Path) -> list[Finding]:
    bundle = load_bundle(root)
    findings: list[Finding] = []
    repo = root.parent  # assume knowledge/ sits at repo root

    for c in bundle.concepts:
        if not c.meta or c.meta.get("_invalid"):
            findings.append(Finding("error", "frontmatter", c.rel, "missing or invalid YAML frontmatter"))
            continue
        ctype = c.meta.get("type")
        if not ctype or not str(ctype).strip():
            findings.append(Finding("error", "concept-type", c.rel, "frontmatter type is required"))

        raw = c.meta.get("raw") or []
        if not isinstance(raw, list):
            findings.append(Finding("error", "raw", c.rel, "raw must be a list of paths"))
            raw = []
        for r in raw:
            rp = Path(str(r))
            abs_r = rp if rp.is_absolute() else repo / rp
            if not abs_r.is_file():
                findings.append(Finding("error", "raw-provenance", c.rel, f"raw path missing: {r}"))

        if raw and not c.citations:
            findings.append(Finding("warning", "citations", c.rel, "has raw: but no # Citations section"))
        for r in raw:
            if c.citations and str(r) not in c.citations:
                findings.append(
                    Finding("warning", "citations-mirror", c.rel, f"raw path not mirrored in Citations: {r}")
                )

        full = c.body + ("\n" + c.citations if c.citations else "")
        for _label, target in md_links(full):
            if resolve_link(bundle, c.rel, target) is None:
                findings.append(Finding("warning", "link-target", c.rel, f"broken link: ({target})"))

    expected = build_indexes(bundle)
    for path, text in expected.items():
        rel = path.relative_to(bundle.root).as_posix()
        if not path.exists():
            findings.append(Finding("warning", "index-missing", rel, "index.md missing; run okf index"))
        elif path.read_text(encoding="utf-8") != text:
            findings.append(Finding("warning", "index-stale", rel, "index.md out of date; run okf index"))

    if bundle.log is None:
        findings.append(Finding("warning", "log-missing", "log.md", "bundle log.md missing"))

    return findings


def report(findings: list[Finding]) -> int:
    """Print findings; return exit code (1 if any error)."""
    if not findings:
        print("OK")
        return 0
    for f in findings:
        print(f.format())
    return 1 if any(f.severity == "error" for f in findings) else 0
