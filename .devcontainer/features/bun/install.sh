#!/usr/bin/env bash
set -euo pipefail

VERSION="${VERSION:-latest}"

echo "Installing Bun (${VERSION})..."

if command -v apt-get >/dev/null 2>&1; then
  export DEBIAN_FRONTEND=noninteractive
  apt-get update -y
  apt-get install -y --no-install-recommends curl unzip ca-certificates
  rm -rf /var/lib/apt/lists/*
fi

INSTALL_USER="${_REMOTE_USER:-root}"
INSTALL_HOME="${_REMOTE_USER_HOME:-/root}"

install_for_user() {
  local user="$1"
  local home="$2"
  local cmd

  if [[ "${VERSION}" == "latest" ]]; then
    cmd='curl -fsSL https://bun.sh/install | bash'
  else
    local clean="${VERSION#v}"
    cmd="curl -fsSL https://bun.sh/install | bash -s \"bun-v${clean}\""
  fi

  if [[ "${user}" == "root" ]]; then
    bash -lc "${cmd}"
  else
    su - "${user}" -c "${cmd}"
  fi

  if [[ -x "${home}/.bun/bin/bun" ]]; then
    ln -sf "${home}/.bun/bin/bun" /usr/local/bin/bun
  else
    echo "error: bun binary not found at ${home}/.bun/bin/bun" >&2
    exit 1
  fi
}

install_for_user "${INSTALL_USER}" "${INSTALL_HOME}"

bun --version
echo "Bun feature install complete."
