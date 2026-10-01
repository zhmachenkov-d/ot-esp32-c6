#!/usr/bin/env bash
# Validate release tag shape, stub-newer-than gate, and optional main-ancestry.
#
# Usage:
#   check-release-tag.sh --tag vX.Y.Z [--sha <commit>] [--main-ref <ref>]
#                        [--stub-header <path>] [--github-output]
#   check-release-tag.sh --self-test
#
# With --sha and --main-ref, requires the commit to be an ancestor of main-ref
# (git merge-base --is-ancestor). With --github-output, writes version= and tag=
# to $GITHUB_OUTPUT when set.
#
# Tag version (without leading v) must be strictly greater than the string-literal
# APP_FW_VERSION stub default in firmware/main/app_config.h (or --stub-header).
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
DEFAULT_STUB_HEADER="$REPO_ROOT/firmware/main/app_config.h"

die() {
  echo "check-release-tag: $*" >&2
  exit 1
}

is_strict_semver_tag() {
  [[ "$1" =~ ^v[0-9]+\.[0-9]+\.[0-9]+$ ]]
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

# Extract the first #define APP_FW_VERSION "X.Y.Z" string-literal stub from a header.
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

require_tag_newer_than_stub() {
  local tag="$1"
  local header="$2"
  local version stub
  is_strict_semver_tag "$tag" || die "Refusing non-strict SemVer tag: $tag (need vMAJOR.MINOR.PATCH)"
  version="${tag#v}"
  stub="$(read_app_fw_version_stub "$header")"
  if ! semver_gt "$version" "$stub"; then
    die "tag $tag is not strictly newer than APP_FW_VERSION stub $stub in $header"
  fi
}

require_main_ancestor() {
  local sha="$1"
  local main_ref="$2"
  git rev-parse --verify "$sha^{commit}" >/dev/null 2>&1 || die "not a commit: $sha"
  git rev-parse --verify "$main_ref^{commit}" >/dev/null 2>&1 || die "main ref missing: $main_ref"
  if ! git merge-base --is-ancestor "$sha" "$main_ref"; then
    die "commit $sha is not reachable from $main_ref"
  fi
}

self_test() {
  local tag
  for tag in v0.0.0 v1.2.3 v10.20.30; do
    is_strict_semver_tag "$tag" || die "expected accept: $tag"
  done
  for tag in v1.2.3-rc1 v1.2 v1.2.3.4 1.2.3 vv1.2.3 latest v01.2.3-beta; do
    if is_strict_semver_tag "$tag"; then
      die "expected reject: $tag"
    fi
  done

  semver_gt "1.2.4" "1.2.3" || die "self-test: 1.2.4 should be > 1.2.3"
  semver_gt "1.3.0" "1.2.9" || die "self-test: 1.3.0 should be > 1.2.9"
  semver_gt "2.0.0" "1.9.9" || die "self-test: 2.0.0 should be > 1.9.9"
  if semver_gt "1.2.3" "1.2.3"; then
    die "self-test: equal must not be gt"
  fi
  if semver_gt "1.2.2" "1.2.3"; then
    die "self-test: older must not be gt"
  fi

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

  require_tag_newer_than_stub "v1.2.4" "$fixture"
  require_tag_newer_than_stub "v1.3.0" "$fixture"
  require_tag_newer_than_stub "v2.0.0" "$fixture"
  # die() exits; run expected failures in a subshell so self_test continues.
  if (require_tag_newer_than_stub "v1.2.3" "$fixture") 2>/dev/null; then
    die "self-test: equal tag must fail"
  fi
  if (require_tag_newer_than_stub "v1.2.2" "$fixture") 2>/dev/null; then
    die "self-test: older tag must fail"
  fi
  if (require_tag_newer_than_stub "v0.9.9" "$fixture") 2>/dev/null; then
    die "self-test: much older tag must fail"
  fi

  # Drive CLI --stub-header path (same fixture) via a fresh process.
  bash "$SCRIPT_DIR/$(basename "${BASH_SOURCE[0]}")" --tag v1.2.4 --stub-header "$fixture" >/dev/null
  if bash "$SCRIPT_DIR/$(basename "${BASH_SOURCE[0]}")" --tag v1.2.3 --stub-header "$fixture" >/dev/null 2>&1; then
    die "self-test: --stub-header equal tag must fail"
  fi

  git -C "$tmpdir" init -q -b main
  git -C "$tmpdir" config user.email "ci@example.com"
  git -C "$tmpdir" config user.name "ci"
  printf 'a\n' >"$tmpdir/f"
  git -C "$tmpdir" add f
  git -C "$tmpdir" commit -q -m a
  local main_sha
  main_sha="$(git -C "$tmpdir" rev-parse HEAD)"

  git -C "$tmpdir" checkout -q -b side
  printf 'b\n' >"$tmpdir/f"
  git -C "$tmpdir" add f
  git -C "$tmpdir" commit -q -m b
  local side_sha
  side_sha="$(git -C "$tmpdir" rev-parse HEAD)"
  git -C "$tmpdir" checkout -q main

  # Run ancestry checks with GIT_DIR so we don't depend on cwd.
  if ! git -C "$tmpdir" merge-base --is-ancestor "$main_sha" main; then
    die "self-test: main tip should be ancestor of main"
  fi
  if git -C "$tmpdir" merge-base --is-ancestor "$side_sha" main; then
    die "self-test: side tip must NOT be ancestor of main"
  fi

  # Drive require_main_ancestor via a subshell with -C by wrapping git.
  (
    cd "$tmpdir"
    require_main_ancestor "$main_sha" main
  )
  if (
    cd "$tmpdir"
    require_main_ancestor "$side_sha" main
  ) 2>/dev/null; then
    die "self-test: side commit should fail ancestry"
  fi

  echo "self-test passed"
}

TAG=""
SHA=""
MAIN_REF=""
STUB_HEADER="$DEFAULT_STUB_HEADER"
WRITE_GITHUB_OUTPUT=0

while [[ $# -gt 0 ]]; do
  case "$1" in
    --self-test)
      self_test
      exit 0
      ;;
    --tag)
      TAG="${2:-}"
      shift 2
      ;;
    --sha)
      SHA="${2:-}"
      shift 2
      ;;
    --main-ref)
      MAIN_REF="${2:-}"
      shift 2
      ;;
    --stub-header)
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

[[ -n "$TAG" ]] || die "usage: $0 --tag vX.Y.Z [--sha SHA --main-ref REF] [--stub-header PATH] [--github-output] | $0 --self-test"
is_strict_semver_tag "$TAG" || die "Refusing non-strict SemVer tag: $TAG (need vMAJOR.MINOR.PATCH)"
VERSION="${TAG#v}"

require_tag_newer_than_stub "$TAG" "$STUB_HEADER"

if [[ -n "$SHA" || -n "$MAIN_REF" ]]; then
  [[ -n "$SHA" && -n "$MAIN_REF" ]] || die "--sha and --main-ref must be used together"
  require_main_ancestor "$SHA" "$MAIN_REF"
fi

if [[ "$WRITE_GITHUB_OUTPUT" -eq 1 ]]; then
  [[ -n "${GITHUB_OUTPUT:-}" ]] || die "GITHUB_OUTPUT not set"
  {
    echo "version=${VERSION}"
    echo "tag=${TAG}"
  } >>"$GITHUB_OUTPUT"
fi

echo "tag=$TAG version=$VERSION ok (newer than stub)"
