"""Export a knowledge bundle as tar.gz + manifest."""

from __future__ import annotations

import hashlib
import json
import tarfile
from datetime import datetime, timezone
from pathlib import Path

from .bundle import load_bundle


def export(root: Path, out_dir: Path) -> tuple[Path, Path]:
    out_dir.mkdir(parents=True, exist_ok=True)
    bundle = load_bundle(root)
    archive = out_dir / "knowledge.tar.gz"
    manifest_path = out_dir / "knowledge-manifest.json"

    files: list[dict] = []
    with tarfile.open(archive, "w:gz") as tar:
        for path in sorted(root.rglob("*")):
            if not path.is_file():
                continue
            if any(p.name == ".git" for p in path.parents):
                continue
            rel = path.relative_to(root).as_posix()
            tar.add(path, arcname=f"knowledge/{rel}")
            data = path.read_bytes()
            files.append(
                {
                    "path": rel,
                    "sha256": hashlib.sha256(data).hexdigest(),
                    "bytes": len(data),
                }
            )

    manifest = {
        "created": datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ"),
        "bundle": root.as_posix(),
        "concepts": len(bundle.concepts),
        "files": files,
    }
    manifest_path.write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    return archive, manifest_path
