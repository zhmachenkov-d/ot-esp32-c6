#!/usr/bin/env bash
# Flash OTC6 firmware (and optionally open the serial monitor).
# Restores the caller's working directory when the session ends.
set -euo pipefail

ORIG_DIR="$(pwd)"
FIRMWARE_DIR="$(cd "$(dirname "$0")" && pwd)"
PORT="${PORT:-/dev/ttyACM0}"

cleanup() {
  cd "$ORIG_DIR" || true
}
trap cleanup EXIT

cd "$FIRMWARE_DIR"
idf.py -p "$PORT" flash monitor "$@"
