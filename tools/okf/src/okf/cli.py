"""CLI entry: okf index|validate|lint|search|export|visualize."""

from __future__ import annotations

import argparse
import sys
from pathlib import Path

from .export import export
from .index_cmd import write_indexes
from .lint import lint
from .search import search
from .validate import report, validate
from .visualize import visualize


def main(argv: list[str] | None = None) -> int:
    p = argparse.ArgumentParser(prog="okf", description="OKF knowledge-bundle tooling")
    sub = p.add_subparsers(dest="cmd", required=True)

    for name, help_ in (
        ("index", "Regenerate index.md files from frontmatter"),
        ("validate", "Conformance + provenance + link checks"),
        ("lint", "Validate plus orphan warnings; optional --fix"),
    ):
        sp = sub.add_parser(name, help=help_)
        sp.add_argument("bundle", type=Path, help="Path to knowledge/ bundle")
        if name == "lint":
            sp.add_argument("--fix", action="store_true", help="Rewrite stale indexes and unique basename links")

    sp = sub.add_parser("search", help="Search concept frontmatter (path/type/title/description/tags)")
    sp.add_argument("bundle", type=Path)
    sp.add_argument("query", type=str)

    sp = sub.add_parser("export", help="Write knowledge.tar.gz + manifest under -o")
    sp.add_argument("bundle", type=Path)
    sp.add_argument("-o", "--out", type=Path, default=Path("dist"))

    sp = sub.add_parser("visualize", help="Write interactive graph HTML")
    sp.add_argument("bundle", type=Path)
    sp.add_argument("-o", "--out", type=Path, default=Path("dist/knowledge-viz.html"))

    args = p.parse_args(argv)
    root = args.bundle

    if args.cmd == "index":
        written = write_indexes(root)
        if written:
            for w in written:
                print(f"wrote {w}")
        else:
            print("indexes up to date")
        return 0

    if args.cmd == "validate":
        return report(validate(root))

    if args.cmd == "lint":
        findings, fixes = lint(root, fix=args.fix)
        if fixes:
            print(f"auto-fixed: {fixes}")
        return report(findings)

    if args.cmd == "search":
        lines = search(root, args.query)
        if not lines:
            print("(no matches)")
            return 0
        print("\n".join(lines))
        return 0

    if args.cmd == "export":
        archive, manifest = export(root, args.out)
        print(archive)
        print(manifest)
        return 0

    if args.cmd == "visualize":
        out = visualize(root, args.out)
        print(out)
        return 0

    return 2


if __name__ == "__main__":
    sys.exit(main())
