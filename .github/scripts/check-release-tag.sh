#!/usr/bin/env bash
# Validate release tag shape and optional main-ancestry.
#
# Usage:
#   check-release-tag.sh --tag vX.Y.Z [--sha <commit>] [--main-ref <ref>] [--github-output]
#   check-release-tag.sh --self-test
#
# With --sha and --main-ref, requires the commit to be an ancestor of main-ref
# (git merge-base --is-ancestor). With --github-output, writes version= and tag=
# to $GITHUB_OUTPUT when set.
set -euo pipefail

die() {
  echo "check-release-tag: $*" >&2
  exit 1
}

is_strict_semver_tag() {
  [[ "$1" =~ ^v[0-9]+\.[0-9]+\.[0-9]+$ ]]
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

  local tmpdir
  tmpdir="$(mktemp -d)"
  # shellcheck disable=SC2064
  trap "rm -rf '$tmpdir'" EXIT

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
  ); then
    die "self-test: side commit should fail ancestry"
  fi

  echo "self-test passed"
}

TAG=""
SHA=""
MAIN_REF=""
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
    --github-output)
      WRITE_GITHUB_OUTPUT=1
      shift
      ;;
    *)
      die "unknown arg: $1"
      ;;
  esac
done

[[ -n "$TAG" ]] || die "usage: $0 --tag vX.Y.Z [--sha SHA --main-ref REF] [--github-output] | $0 --self-test"
is_strict_semver_tag "$TAG" || die "Refusing non-strict SemVer tag: $TAG (need vMAJOR.MINOR.PATCH)"
VERSION="${TAG#v}"

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

echo "tag=$TAG version=$VERSION ok"
