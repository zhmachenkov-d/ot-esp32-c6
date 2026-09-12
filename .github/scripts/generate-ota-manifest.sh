#!/usr/bin/env bash
# Generate OTA manifest.json for a GitHub Release asset pair.
#
# Usage:
#   generate-ota-manifest.sh <bin-path> <version> <owner/repo> <tag> <out-path>
#   generate-ota-manifest.sh --self-test
#
# Emits manifest_version=1 with firmware_id, version, url, sha256, size, release_url.
set -euo pipefail

FIRMWARE_ID="otc6_gateway"
BIN_NAME="otc6_gateway.bin"

die() {
  echo "generate-ota-manifest: $*" >&2
  exit 1
}

validate_manifest_file() {
  local path="$1"
  local expect_version="${2:-}"
  command -v python3 >/dev/null 2>&1 || die "python3 required to validate manifest"

  python3 - "$path" "$expect_version" <<'PY'
import json, re, sys

path, expect_version = sys.argv[1], sys.argv[2]
with open(path, encoding="utf-8") as f:
    data = json.load(f)

required = (
    "manifest_version",
    "firmware_id",
    "version",
    "url",
    "sha256",
    "size",
    "release_url",
)
missing = [k for k in required if k not in data]
if missing:
    raise SystemExit(f"missing keys: {missing}")

if data["manifest_version"] != 1:
    raise SystemExit(f"manifest_version must be 1, got {data['manifest_version']!r}")
if data["firmware_id"] != "otc6_gateway":
    raise SystemExit(f"firmware_id must be otc6_gateway, got {data['firmware_id']!r}")
if not re.fullmatch(r"[0-9]+\.[0-9]+\.[0-9]+", str(data["version"])):
    raise SystemExit(f"version must be X.Y.Z, got {data['version']!r}")
if expect_version and data["version"] != expect_version:
    raise SystemExit(f"version {data['version']!r} != expected {expect_version!r}")
if not str(data["url"]).startswith("https://github.com/"):
    raise SystemExit(f"url must be https://github.com/..., got {data['url']!r}")
if not str(data["release_url"]).startswith("https://github.com/"):
    raise SystemExit(f"release_url must be https://github.com/..., got {data['release_url']!r}")
sha = data["sha256"]
if not isinstance(sha, str) or not re.fullmatch(r"[0-9a-f]{64}", sha):
    raise SystemExit(f"sha256 must be 64 lowercase hex, got {sha!r}")
if not isinstance(data["size"], int) or data["size"] <= 0:
    raise SystemExit(f"size must be positive int, got {data['size']!r}")
print("manifest ok")
PY
}

self_test() {
  local tmpdir
  tmpdir="$(mktemp -d)"
  # Expand path at trap-set time; locals are gone when EXIT fires after return.
  # shellcheck disable=SC2064
  trap "rm -rf '$tmpdir'" EXIT

  printf 'ota-manifest-self-test\n' >"$tmpdir/$BIN_NAME"
  "$0" "$tmpdir/$BIN_NAME" "9.8.7" "example/ot-esp32-c6" "v9.8.7" "$tmpdir/manifest.json"
  validate_manifest_file "$tmpdir/manifest.json" "9.8.7"

  local expect_sha expect_size
  expect_sha="$(sha256sum "$tmpdir/$BIN_NAME" | awk '{print tolower($1)}')"
  expect_size="$(wc -c <"$tmpdir/$BIN_NAME" | tr -d ' ')"

  python3 - "$tmpdir/manifest.json" "$expect_sha" "$expect_size" <<'PY'
import json, sys
path, expect_sha, expect_size = sys.argv[1], sys.argv[2], int(sys.argv[3])
with open(path, encoding="utf-8") as f:
    data = json.load(f)
assert data["sha256"] == expect_sha, (data["sha256"], expect_sha)
assert data["size"] == expect_size, (data["size"], expect_size)
assert data["url"] == "https://github.com/example/ot-esp32-c6/releases/download/v9.8.7/otc6_gateway.bin"
assert data["release_url"] == "https://github.com/example/ot-esp32-c6/releases/tag/v9.8.7"
print("self-test passed")
PY
}

if [[ "${1:-}" == "--self-test" ]]; then
  self_test
  exit 0
fi

if [[ $# -ne 5 ]]; then
  die "usage: $0 <bin-path> <version> <owner/repo> <tag> <out-path> | $0 --self-test"
fi

BIN_PATH="$1"
VERSION="$2"
REPO="$3"
TAG="$4"
OUT_PATH="$5"

[[ -f "$BIN_PATH" ]] || die "bin not found: $BIN_PATH"
[[ "$VERSION" =~ ^[0-9]+\.[0-9]+\.[0-9]+$ ]] || die "version must be X.Y.Z, got: $VERSION"
[[ "$TAG" =~ ^v[0-9]+\.[0-9]+\.[0-9]+$ ]] || die "tag must be vX.Y.Z, got: $TAG"
[[ "$TAG" == "v${VERSION}" ]] || die "tag $TAG must equal v${VERSION}"
[[ "$REPO" =~ ^[A-Za-z0-9_.-]+/[A-Za-z0-9_.-]+$ ]] || die "repo must be owner/name, got: $REPO"

SIZE="$(wc -c <"$BIN_PATH" | tr -d ' ')"
[[ "$SIZE" -gt 0 ]] || die "bin empty: $BIN_PATH"
SHA256="$(sha256sum "$BIN_PATH" | awk '{print tolower($1)}')"
[[ "$SHA256" =~ ^[0-9a-f]{64}$ ]] || die "sha256sum produced unexpected digest: $SHA256"

URL="https://github.com/${REPO}/releases/download/${TAG}/${BIN_NAME}"
RELEASE_URL="https://github.com/${REPO}/releases/tag/${TAG}"

umask 022
cat >"$OUT_PATH" <<EOF
{
  "manifest_version": 1,
  "firmware_id": "${FIRMWARE_ID}",
  "version": "${VERSION}",
  "url": "${URL}",
  "sha256": "${SHA256}",
  "size": ${SIZE},
  "release_url": "${RELEASE_URL}"
}
EOF

validate_manifest_file "$OUT_PATH" "$VERSION"
echo "wrote $OUT_PATH"
