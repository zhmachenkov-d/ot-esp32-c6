#!/usr/bin/env bash
set -euo pipefail

SSH_DIR="${HOME}/.ssh"

WS="${containerWorkspaceFolder:-/workspaces/ot-esp32-c6}"

git config --global --add safe.directory "${WS}"

source /opt/esp/idf/export.sh

if [[ -f CMakeLists.txt ]]; then
  idf.py set-target esp32c6
fi

echo "ESP-IDF: $(idf.py --version)"
echo "Default target: esp32c6 (set via idf.py set-target when CMakeLists.txt exists)"

# Install specify-cli
uv tool install specify-cli --from git+https://github.com/github/spec-kit.git@v1.0.1

# OKF knowledge-bundle tooling (okf …) via uv user tools — not the root-owned IDF venv.
uv tool install -e "${WS}/tools/okf"
