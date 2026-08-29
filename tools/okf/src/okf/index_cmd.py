"""Regenerate index.md files from concept frontmatter."""

from __future__ import annotations

from collections import defaultdict
from pathlib import Path

from .bundle import Bundle, load_bundle


def build_indexes(bundle: Bundle) -> dict[Path, str]:
    """Return path → new index body for root and each topic dir that has concepts."""
    by_dir: dict[str, list] = defaultdict(list)
    for c in bundle.concepts:
        parent = Path(c.rel).parent.as_posix()
        by_dir[parent].append(c)

    out: dict[Path, str] = {}
    # Root index: list topic directories
    dirs = sorted(d for d in by_dir if d != ".")
    lines = [
        "# Knowledge Bundle",
        "",
        "Compiled Open Knowledge Format concepts for the esp32-c6-opentherm project.",
        "",
        "# Directories",
        "",
    ]
    for d in dirs:
        title = d.replace("-", " ").title() if d != "esp-idf" else "ESP-IDF"
        if d == "esp32":
            title = "ESP32"
        elif d == "opentherm":
            title = "OpenTherm"
        elif d == "zigbee":
            title = "Zigbee"
        elif d == "bridge":
            title = "Bridge"
        elif d == "esp-idf":
            title = "ESP-IDF"
        lines.append(f"* [{title}]({d}/)")
    lines.append("")
    out[bundle.root / "index.md"] = "\n".join(lines) + "\n"

    for d, concepts in sorted(by_dir.items()):
        if d == ".":
            continue
        title = {
            "esp32": "ESP32",
            "esp-idf": "ESP-IDF",
            "opentherm": "OpenTherm",
            "zigbee": "Zigbee",
            "bridge": "Bridge",
        }.get(d, d.replace("-", " ").title())
        entries = []
        for c in sorted(concepts, key=lambda x: (x.meta.get("title") or x.rel).lower()):
            ct = c.meta.get("title") or Path(c.rel).stem
            desc = (c.meta.get("description") or "").strip()
            link = c.rel  # bundle-root-relative, matches existing indexes
            if desc:
                entries.append(f"* [{ct}]({link}) - {desc}")
            else:
                entries.append(f"* [{ct}]({link})")
        body = f"# {title}\n\n" + "\n".join(entries) + "\n"
        out[bundle.root / d / "index.md"] = body
    return out


def write_indexes(root: Path) -> list[Path]:
    bundle = load_bundle(root)
    written: list[Path] = []
    for path, text in build_indexes(bundle).items():
        path.parent.mkdir(parents=True, exist_ok=True)
        if path.exists() and path.read_text(encoding="utf-8") == text:
            continue
        path.write_text(text, encoding="utf-8")
        written.append(path)
    return written
