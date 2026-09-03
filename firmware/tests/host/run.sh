#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")" && pwd)"
cmake -S "$ROOT" -B "$ROOT/build"
cmake --build "$ROOT/build"
ctest --test-dir "$ROOT/build" --output-on-failure
