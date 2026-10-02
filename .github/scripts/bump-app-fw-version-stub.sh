#!/usr/bin/env bash
# Rewrite the local APP_FW_VERSION string-literal stub in app_config.h.
#
# Usage:
#   bump-app-fw-version-stub.sh --version X.Y.Z [--header <path>] [--github-output]
#   bump-app-fw-version-stub.sh --self-test
#
# Sets the #define APP_FW_VERSION "…" stub to --version. No-op (exit 0) when
# already equal. Refuses to move the stub backwards. With --github-output,
# writes bumped=true|false to $GITHUB_OUTPUT when set.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
DEFAULT_STUB_HEADER="$REPO_ROOT/firmware/main/app_config.h"

die() {
  echo "bump-app-fw-version-stub: $*" >&2
  exit 1
}

is_strict_semver_version() {
  [[ "$1" =~ ^[0-9]+\.[0-9]+\.[0-9]+$ ]]
}

# Exit 0 if $1 > $2 (numeric major.minor.patch). Both must be X.Y.Z.
semver_gt() {
  local a="$1" b="$2"
  local a1 a2 a3 b1 b2 b3
  IFS=. read -r a1 a2 a3 <<<"$a"
  IFS=. read -r b1 b2 b3 <<<"$b"
  if ((10#$a1 > 10#$b1)); then return 0; fi
  if ((10#$a1 < 10#$b1)); then return 1; fi
  if ((10#$a2 > 10#$b2)); then return 0; fi
  if ((10#$a2 < 10#$b2)); then return 1; fi
  if ((10#$a3 > 10#$b3)); then return 0; fi
  return 1
}

read_app_fw_version_stub() {
  local header="$1"
  local line stub
  [[ -f "$header" ]] || die "stub header missing: $header"
  line="$(
    grep -E '^[[:space:]]*#define[[:space:]]+APP_FW_VERSION[[:space:]]+"' "$header" | head -n1 || true
  )"
  [[ -n "$line" ]] || die "no #define APP_FW_VERSION \"…\" in $header"
  if [[ "$line" =~ \"([0-9]+\.[0-9]+\.[0-9]+)\" ]]; then
    stub="${BASH_REMATCH[1]}"
  else
    die "APP_FW_VERSION stub is not strict X.Y.Z in $header"
  fi
  is_strict_semver_version "$stub" || die "APP_FW_VERSION stub is not strict X.Y.Z: $stub"
  printf '%s\n' "$stub"
}

write_app_fw_version_stub() {
  local header="$1"
  local version="$2"
  local tmp
  tmp="$(mktemp)"
  if ! awk -v ver="$version" '
    BEGIN { done = 0 }
    /^[[:space:]]*#define[[:space:]]+APP_FW_VERSION[[:space:]]+"/ && !done {
      if ($0 !~ /"[0-9]+\.[0-9]+\.[0-9]+"/) {
        exit 2
      }
      sub(/"[0-9]+\.[0-9]+\.[0-9]+"/, "\"" ver "\"")
      done = 1
    }
    { print }
    END { if (!done) exit 3 }
  ' "$header" >"$tmp"; then
    rm -f "$tmp"
    die "failed to rewrite stub in $header"
  fi
  mv "$tmp" "$header"
}

bump_stub() {
  local header="$1"
  local version="$2"
  local stub
  is_strict_semver_version "$version" || die "version is not strict X.Y.Z: $version"
  stub="$(read_app_fw_version_stub "$header")"
  if [[ "$stub" == "$version" ]]; then
    echo "stub already at $version"
    return 1
  fi
  if semver_gt "$stub" "$version"; then
    die "refusing to move stub backwards: $stub → $version"
  fi
  write_app_fw_version_stub "$header" "$version"
  [[ "$(read_app_fw_version_stub "$header")" == "$version" ]] || die "post-write stub mismatch"
  echo "stub bumped $stub → $version"
  return 0
}

self_test() {
  local tmpdir fixture
  tmpdir="$(mktemp -d)"
  # shellcheck disable=SC2064
  trap "rm -rf '$tmpdir'" EXIT

  fixture="$tmpdir/app_config.h"
  cat >"$fixture" <<'EOF'
#ifndef APP_FW_VERSION
#define APP_FW_VERSION              "1.2.3"
#endif
EOF

  [[ "$(read_app_fw_version_stub "$fixture")" == "1.2.3" ]] || die "self-test: stub parse"

  bump_stub "$fixture" "1.2.4" || die "self-test: bump should succeed"
  [[ "$(read_app_fw_version_stub "$fixture")" == "1.2.4" ]] || die "self-test: after bump"

  if bump_stub "$fixture" "1.2.4"; then
    die "self-test: equal bump must report already-at (non-zero)"
  fi
  [[ "$(read_app_fw_version_stub "$fixture")" == "1.2.4" ]] || die "self-test: equal must leave stub"

  # die() exits; run expected failure in a subshell so self_test continues.
  if (bump_stub "$fixture" "1.2.3") 2>/dev/null; then
    die "self-test: backwards bump must fail"
  fi
  [[ "$(read_app_fw_version_stub "$fixture")" == "1.2.4" ]] || die "self-test: backwards must leave stub"

  # Drive CLI via a fresh process.
  cat >"$fixture" <<'EOF'
#ifndef APP_FW_VERSION
#define APP_FW_VERSION              "0.1.0"
#endif
EOF
  bash "$SCRIPT_DIR/$(basename "${BASH_SOURCE[0]}")" --version 0.1.1 --header "$fixture" >/dev/null
  [[ "$(read_app_fw_version_stub "$fixture")" == "0.1.1" ]] || die "self-test: CLI bump"
  if bash "$SCRIPT_DIR/$(basename "${BASH_SOURCE[0]}")" --version 0.1.1 --header "$fixture" >/dev/null 2>&1; then
    : # already-at still exits 0 at CLI (idempotent)
  fi
  if bash "$SCRIPT_DIR/$(basename "${BASH_SOURCE[0]}")" --version 0.0.9 --header "$fixture" >/dev/null 2>&1; then
    die "self-test: CLI backwards must fail"
  fi

  echo "self-test passed"
}

VERSION=""
STUB_HEADER="$DEFAULT_STUB_HEADER"
WRITE_GITHUB_OUTPUT=0

while [[ $# -gt 0 ]]; do
  case "$1" in
    --self-test)
      self_test
      exit 0
      ;;
    --version)
      VERSION="${2:-}"
      shift 2
      ;;
    --header)
      STUB_HEADER="${2:-}"
      shift 2
      ;;
    --github-output)
      WRITE_GITHUB_OUTPUT=1
      shift
      ;;
    *)
      die "unknown arg: $1"
      ;;
  esac
done

[[ -n "$VERSION" ]] || die "usage: $0 --version X.Y.Z [--header PATH] [--github-output] | $0 --self-test"
is_strict_semver_version "$VERSION" || die "version is not strict X.Y.Z: $VERSION"

BUMPED=false
if bump_stub "$STUB_HEADER" "$VERSION"; then
  BUMPED=true
fi

if [[ "$WRITE_GITHUB_OUTPUT" -eq 1 ]]; then
  [[ -n "${GITHUB_OUTPUT:-}" ]] || die "GITHUB_OUTPUT not set"
  echo "bumped=${BUMPED}" >>"$GITHUB_OUTPUT"
fi

exit 0
