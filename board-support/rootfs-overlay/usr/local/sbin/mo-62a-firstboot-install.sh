#!/usr/bin/env bash
set -uo pipefail

# MO-62A first-boot orchestrator (runs once, then self-disables).
#   1. install prebuilt .deb packages from the overlay  -> mo-62a-auto-inject.sh
#   2. grow the root partition + expand the filesystem   -> mo-62a-auto-resize.sh
#   3. apply /boot/firmware/sysconfig.txt                -> mo-62a-auto-config.sh
# Each step is best-effort: a failure is logged but does not block the others,
# so the board always reaches a usable state on first boot.

SENTINEL_DONE=/opt/ti/.edgeai-installed
SBIN=/usr/local/sbin
LOG=/var/log/mo-62a-firstboot-install.log

# Mirror all output to a log file (journald also captures via the service).
exec > >(tee -a "$LOG") 2>&1

log() { echo "[mo-62a-firstboot] $*"; }

log "Starting ($(date -Iseconds))"

# ── 1. Install prebuilt .deb packages ─────────────────────────────────────────
if [ -x "$SBIN/mo-62a-auto-inject.sh" ]; then
    log "Installing prebuilt packages"
    "$SBIN/mo-62a-auto-inject.sh" || log "WARNING: package install reported an error"
else
    log "WARNING: $SBIN/mo-62a-auto-inject.sh not found; skipping package install"
fi

# ── 2. Grow root partition + filesystem ───────────────────────────────────────
if [ -x "$SBIN/mo-62a-auto-resize.sh" ]; then
    log "Running rootfs resize"
    "$SBIN/mo-62a-auto-resize.sh" || log "WARNING: resize reported an error"
else
    log "WARNING: $SBIN/mo-62a-auto-resize.sh not found; skipping resize"
fi

# ── 3. Apply user configuration (sysconfig.txt on the BOOT partition) ────────
if [ -x "$SBIN/mo-62a-auto-config.sh" ]; then
    log "Applying user configuration"
    "$SBIN/mo-62a-auto-config.sh" || log "WARNING: config apply reported an error"
else
    log "WARNING: $SBIN/mo-62a-auto-config.sh not found; skipping user configuration"
fi

# ── Self-disable ──────────────────────────────────────────────────────────────
touch "$SENTINEL_DONE"
systemctl disable mo-62a-firstboot-install.service || true
log "All done ($(date -Iseconds))."
