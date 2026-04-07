#!/bin/bash
# imx219-preview.sh — IMX219 camera live preview via KMS/HDMI
#
# Usage:  [sudo] imx219-preview.sh [fps]
#   fps: 5 | 8 | 10 | 15 (default) | 30
#
# Environment variables:
#   WB_R=0.5    White-balance red   channel reference (0~1; lower = boost red)
#   WB_B=0.6    White-balance blue  channel reference (0~1; lower = boost blue)
#   GAIN=150    Analogue gain  0~232  (low-light recommended: 200~232)
#   DGAIN=256   Digital  gain  256~4095  (low-light: 512~1024, increases noise)
#   EXPOSURE    Exposure lines (defaults to maximum for the chosen FPS)
#
# Examples:
#   imx219-preview.sh              # 15 fps, default white-balance
#   imx219-preview.sh 10           # 10 fps
#   GAIN=232 imx219-preview.sh 5   # low-light, 5 fps, maximum exposure
#   WB_R=0.4 WB_B=0.5 imx219-preview.sh   # warmer white-balance
#
# Pipeline:
#   IMX219 (SRGGB8) -> cdns_csi2rx -> ti_csi2rx -> /dev/video2
#   -> v4l2src -> bayer2rgb -> frei0r-filter-white-balance -> kmssink
#
# Notes:
#   - Requires frei0r-plugins:  sudo apt-get install -y frei0r-plugins
#   - Any running display manager (lightdm/gdm3/sddm) is stopped automatically
#     while the preview is running and restarted on exit.
#   - Must be run as root (or with sudo) because kmssink requires DRM master.

# ── Defaults ──────────────────────────────────────────────────────────────────
WB_R=${WB_R:-0.5}
WB_G=${WB_G:-1.0}
WB_B=${WB_B:-0.6}
GAIN=${GAIN:-150}
DGAIN=${DGAIN:-256}
FPS=${1:-15}

# ── Preflight checks ──────────────────────────────────────────────────────────
if [[ "$(id -u)" != "0" ]]; then
    echo "ERROR: This script requires root (kmssink needs DRM master)."
    echo "       Run: sudo imx219-preview.sh"
    exit 1
fi

command -v gst-launch-1.0 >/dev/null 2>&1 || {
    echo "ERROR: gst-launch-1.0 not found. Install gstreamer1.0-tools."
    exit 1
}
command -v media-ctl >/dev/null 2>&1 || {
    echo "ERROR: media-ctl not found. Install v4l-utils."
    exit 1
}
command -v v4l2-ctl >/dev/null 2>&1 || {
    echo "ERROR: v4l2-ctl not found. Install v4l-utils."
    exit 1
}
# Auto-detect the CSI capture video device from the j721e-csi2rx driver.
# The device number can shift depending on which other drivers are loaded
# (e.g. wave5 codec, JPEG encoder), so we discover it dynamically.
VIDEO_DEV=$(v4l2-ctl --list-devices 2>/dev/null \
    | awk '/j721e-csi2rx/{found=1; next} found && /\/dev\/video/{print $1; exit}')
[[ -n "$VIDEO_DEV" && -e "$VIDEO_DEV" ]] || {
    echo "ERROR: CSI capture device not found (j721e-csi2rx) — camera not detected or driver not loaded."
    exit 1
}
echo "[imx219] CSI capture device: ${VIDEO_DEV}"
[[ -e /dev/media0 ]] || {
    echo "ERROR: /dev/media0 not found — media controller not available."
    exit 1
}
gst-inspect-1.0 frei0r-filter-white-balance >/dev/null 2>&1 || {
    echo "ERROR: GStreamer frei0r plugin not found."
    echo "       Install with: sudo apt-get install -y frei0r-plugins"
    exit 1
}

# ── FPS → vertical-blanking / max-exposure table ──────────────────────────────
# Derived from: pixel_rate=182 MHz, line_length=3448 (hblank=1528 + width=1920)
# max_exp = height + vblank - 4  (IMX219 datasheet constraint)
case "$FPS" in
  30) VBLANK=679;  MAX_EXP=1751  ;;
  15) VBLANK=2438; MAX_EXP=3514  ;;
  10) VBLANK=4198; MAX_EXP=5274  ;;
   8) VBLANK=5518; MAX_EXP=6594  ;;
   5) VBLANK=9476; MAX_EXP=10552 ;;
   *)
    echo "ERROR: Unsupported FPS '$FPS'. Valid values: 5 8 10 15 30"
    exit 1
    ;;
esac

EXPOSURE=${EXPOSURE:-$MAX_EXP}

# ── Display-manager handling ───────────────────────────────────────────────────
# kmssink takes DRM master; any running display manager must be stopped first.
DM_WAS_ACTIVE=0
DM_NAME=""
for _dm in lightdm gdm3 sddm weston; do
    if systemctl is-active --quiet "$_dm" 2>/dev/null; then
        DM_WAS_ACTIVE=1
        DM_NAME="$_dm"
        break
    fi
done

_cleanup() {
    if [[ "$DM_WAS_ACTIVE" -eq 1 && -n "$DM_NAME" ]]; then
        echo "[imx219] Restarting $DM_NAME..."
        systemctl start "$DM_NAME" || true
    fi
}
trap _cleanup EXIT

if [[ "$DM_WAS_ACTIVE" -eq 1 ]]; then
    echo "[imx219] Stopping $DM_NAME (will restart on exit)..."
    systemctl stop "$DM_NAME"
    sleep 1   # wait for DRM master to be released
fi

# ── Configure CSI media pipeline ──────────────────────────────────────────────
echo "[imx219] Configuring CSI pipeline: 1920x1080 SRGGB8 @ ${FPS} fps"
media-ctl -d /dev/media0 --set-v4l2 \
    '"imx219 2-0010":0 [fmt:SRGGB8_1X8/1920x1080]'
media-ctl -d /dev/media0 --set-v4l2 \
    '"cdns_csi2rx.30101000.csi-bridge":0 [fmt:SRGGB8_1X8/1920x1080]'
media-ctl -d /dev/media0 --set-v4l2 \
    '"cdns_csi2rx.30101000.csi-bridge":1 [fmt:SRGGB8_1X8/1920x1080]'
media-ctl -d /dev/media0 --set-v4l2 \
    '"30102000.ticsi2rx":0 [fmt:SRGGB8_1X8/1920x1080]'
media-ctl -d /dev/media0 --set-v4l2 \
    '"30102000.ticsi2rx":1 [fmt:SRGGB8_1X8/1920x1080]'

# ── Sensor controls ────────────────────────────────────────────────────────────
# Find the IMX219 subdev node dynamically via media-ctl.
IMX219_SUBDEV=$(media-ctl -d /dev/media0 --print-topology 2>/dev/null \
    | awk -F'[()"\"]' '/imx219/{for(i=1;i<=NF;i++) if($i ~ /\/dev\/v4l-subdev/) {print $i; exit}}')
[[ -n "$IMX219_SUBDEV" ]] || IMX219_SUBDEV=/dev/v4l-subdev2
echo "[imx219] Sensor subdev: ${IMX219_SUBDEV}"
echo "[imx219] Sensor: exposure=${EXPOSURE} analogue_gain=${GAIN} digital_gain=${DGAIN}"
v4l2-ctl -d "${IMX219_SUBDEV}" --set-ctrl=vertical_blanking="${VBLANK}"
v4l2-ctl -d "${IMX219_SUBDEV}" --set-ctrl=exposure="${EXPOSURE}"
v4l2-ctl -d "${IMX219_SUBDEV}" --set-ctrl=analogue_gain="${GAIN}"
v4l2-ctl -d "${IMX219_SUBDEV}" --set-ctrl=digital_gain="${DGAIN}"

# ── Launch GStreamer pipeline ──────────────────────────────────────────────────
echo "[imx219] White-balance: WB_R=${WB_R}  WB_G=${WB_G}  WB_B=${WB_B}"
echo "[imx219] Starting KMS preview — press Ctrl+C to exit"
gst-launch-1.0 \
    v4l2src device="${VIDEO_DEV}" io-mode=mmap ! \
    "video/x-bayer,format=rggb,width=1920,height=1080,framerate=${FPS}/1" ! \
    queue max-size-buffers=2 leaky=downstream ! \
    bayer2rgb ! \
    videoconvert ! 'video/x-raw,format=RGBA' ! \
    frei0r-filter-white-balance \
        neutral-color-r="${WB_R}" \
        neutral-color-g="${WB_G}" \
        neutral-color-b="${WB_B}" \
        green-tint=0.0 ! \
    videoconvert ! \
    kmssink sync=false
