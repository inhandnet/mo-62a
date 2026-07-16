#!/usr/bin/env bash
set -uo pipefail

# MO-62A first-boot helper: grow the root partition to fill the disk and
# expand the ext4 filesystem online. Safe to re-run (growpart returns NOCHANGE
# and resize2fs is a no-op once the filesystem already fills the partition).
# Invoked by mo-62a-firstboot-install.sh; can also be run standalone for tests.

log() { echo "[mo-62a-resize] $*"; }

if [ "$(id -u)" -ne 0 ]; then
    log "ERROR: must run as root" >&2
    exit 1
fi

log "Resizing root partition to fill disk..."

ROOT_PART=$(findmnt -n -o SOURCE /)
log "  root partition: $ROOT_PART"

# Parse disk device and partition number from the root partition path
# (handles both /dev/mmcblk1p3 and /dev/sda3 styles).
DISK=""
PART_NUM=""
if [[ "$ROOT_PART" =~ ^(/dev/mmcblk[0-9]+)p([0-9]+)$ ]]; then
    DISK="${BASH_REMATCH[1]}"
    PART_NUM="${BASH_REMATCH[2]}"
elif [[ "$ROOT_PART" =~ ^(/dev/[a-z]+)([0-9]+)$ ]]; then
    DISK="${BASH_REMATCH[1]}"
    PART_NUM="${BASH_REMATCH[2]}"
else
    log "ERROR: cannot parse root partition: $ROOT_PART" >&2
    exit 1
fi
log "  disk: $DISK  partition: $PART_NUM"

# Safety guard: root must be on partition 2 (legacy) or 3 (BOOT+SWAP+rootfs).
if [ "$PART_NUM" != "2" ] && [ "$PART_NUM" != "3" ]; then
    log "ERROR: root is on partition $PART_NUM (expected 2 or 3); refusing to resize." >&2
    exit 1
fi

# Grow the partition entry. Prefer growpart for clean NOCHANGE handling.
if command -v growpart >/dev/null 2>&1; then
    GROWPART_RC=0
    growpart "$DISK" "$PART_NUM" || GROWPART_RC=$?
    case $GROWPART_RC in
        0) log "  growpart: partition expanded" ;;
        1) log "  growpart: partition already at max size (NOCHANGE)" ;;
        *) log "ERROR: growpart failed (rc=$GROWPART_RC)" >&2; exit 1 ;;
    esac
else
    parted -s "$DISK" resizepart "$PART_NUM" 100%
    log "  parted resizepart done"
fi

# Refresh the kernel's view of the partition table before resizing the fs.
if command -v partprobe >/dev/null 2>&1; then
    partprobe "$DISK" || true
fi

log "Expanding root ext4 filesystem..."
resize2fs "$ROOT_PART"
log "  resize2fs complete"
