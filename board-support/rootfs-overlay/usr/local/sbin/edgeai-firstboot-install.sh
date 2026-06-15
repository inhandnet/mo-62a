#!/usr/bin/env bash
set -euo pipefail

# EdgeAI first-boot disk expansion
#
# Phase 1 (first boot):  resize root partition to fill the disk,
#                        write resize-pending sentinel, reboot.
# Phase 2 (second boot): online-resize root ext4 filesystem (resize2fs),
#                        write done sentinel, self-disable.
# Subsequent boots:      ConditionPathExists in the .service skips this entirely.

SENTINEL_DONE=/opt/ti/.edgeai-installed
SENTINEL_RESIZE=/opt/ti/.edgeai-resize-pending
LOG=/var/log/edgeai-firstboot-install.log

# Mirror all output to log file (journald also captures via service)
exec > >(tee -a "$LOG") 2>&1

echo "[edgeai-firstboot] Starting ($(date -Iseconds))"

# ── Phase 2: online filesystem resize ─────────────────────────────────────────
if [ -f "$SENTINEL_RESIZE" ]; then
    echo "[edgeai-firstboot] Phase 2: online resize of root ext4 filesystem"

    ROOT_PART=$(findmnt -n -o SOURCE /)
    echo "[edgeai-firstboot]   root partition: $ROOT_PART"

    resize2fs "$ROOT_PART"

    rm -f "$SENTINEL_RESIZE"
    touch "$SENTINEL_DONE"
    systemctl disable edgeai-firstboot-install.service || true
    echo "[edgeai-firstboot] Filesystem resize complete. All done ($(date -Iseconds))."
    exit 0
fi

# ── Phase 1: partition resize ──────────────────────────────────────────────────
echo "[edgeai-firstboot] Resizing root partition to fill disk..."

ROOT_PART=$(findmnt -n -o SOURCE /)
echo "[edgeai-firstboot]   root partition: $ROOT_PART"

# Parse disk device and partition number from root partition path.
# Handles both /dev/mmcblk1p2 and /dev/sda2 style names.
DISK=""
PART_NUM=""
if [[ "$ROOT_PART" =~ ^(/dev/mmcblk[0-9]+)p([0-9]+)$ ]]; then
    DISK="${BASH_REMATCH[1]}"
    PART_NUM="${BASH_REMATCH[2]}"
elif [[ "$ROOT_PART" =~ ^(/dev/[a-z]+)([0-9]+)$ ]]; then
    DISK="${BASH_REMATCH[1]}"
    PART_NUM="${BASH_REMATCH[2]}"
else
    echo "[edgeai-firstboot] ERROR: cannot parse root partition: $ROOT_PART" >&2
    exit 1
fi
echo "[edgeai-firstboot]   disk: $DISK  partition: $PART_NUM"

# Safety guard: root must be on partition 2 (legacy: no swap) or 3 (with swap partition).
if [ "$PART_NUM" != "2" ] && [ "$PART_NUM" != "3" ]; then
    echo "[edgeai-firstboot] ERROR: root is on partition $PART_NUM (expected 2 or 3); refusing to resize." >&2
    exit 1
fi

# Resize the partition entry to fill the disk.
# Prefer growpart (cloud-utils) for clean NOCHANGE handling; fall back to parted.
if command -v growpart >/dev/null 2>&1; then
    GROWPART_RC=0
    growpart "$DISK" "$PART_NUM" || GROWPART_RC=$?
    case $GROWPART_RC in
        0) echo "[edgeai-firstboot]   growpart: partition expanded" ;;
        1) echo "[edgeai-firstboot]   growpart: partition already at max size (NOCHANGE)" ;;
        *) echo "[edgeai-firstboot] ERROR: growpart failed (rc=$GROWPART_RC)" >&2; exit 1 ;;
    esac
else
    parted -s "$DISK" resizepart "$PART_NUM" 100%
    echo "[edgeai-firstboot]   parted resizepart done"
fi

# Flush writes before reboot so the sentinel survives.
sync
touch "$SENTINEL_RESIZE"
sync

echo "[edgeai-firstboot] Partition resized. Rebooting to complete filesystem resize..."
systemctl reboot
