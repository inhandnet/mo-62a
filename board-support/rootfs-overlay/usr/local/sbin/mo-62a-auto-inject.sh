#!/usr/bin/env bash
set -uo pipefail

# MO-62A first-boot helper: install (inject) the prebuilt .deb packages shipped
# in the rootfs overlay into the running system. Idempotent (dpkg -i can be
# re-run). Invoked by mo-62a-firstboot-install.sh; can also be run standalone.
#     mo-62a-auto-inject.sh [/path/to/deb-dir]

DEB_DIR="${1:-/usr/local/share/mo-62a/prebuilt-deb}"

log() { echo "[mo-62a-inject] $*"; }

if [ "$(id -u)" -ne 0 ]; then
    log "ERROR: must run as root" >&2
    exit 1
fi

if [ -d "$DEB_DIR" ] && [ -n "$(ls -A "$DEB_DIR"/*.deb 2>/dev/null)" ]; then
    log "Installing prebuilt packages from $DEB_DIR"
    dpkg -i "$DEB_DIR"/*.deb || log "WARNING: some prebuilt packages failed to install"
    log "Package install done"
else
    log "No prebuilt packages to install"
fi
