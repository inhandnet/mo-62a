#!/bin/bash
# imx219-preview.sh — IMX219 camera live preview via VPAC ISP + KMS/HDMI
#
# Usage:  [sudo] imx219-preview.sh [fps]
#   fps: 5 | 8 | 10 | 15 (default) | 30
#
# Pipeline:
#   IMX219 (RGGB8) -> cdns_csi2rx -> ti_csi2rx -> /dev/video2
#   -> v4l2src -> tiovxisp (VPAC ISP) -> NV12 -> kmssink (tidss)
#
# Notes:
#   - Requires tiovxisp DCC tuning files at /opt/imaging/imx219/linear/
#   - Any running display manager (lightdm/gdm3/sddm) is stopped automatically
#     while the preview is running and restarted on exit.
#   - Must be run as root (or with sudo) because kmssink requires DRM master.

# ── Defaults ──────────────────────────────────────────────────────────────────
FPS=${1:-15}

# ── FPS → vertical-blanking / max-exposure table ──────────────────────────────
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
GAIN=${GAIN:-150}
DGAIN=${DGAIN:-256}

DCC_VISS=/opt/imaging/imx219/linear/dcc_viss.bin
DCC_2A=/opt/imaging/imx219/linear/dcc_2a.bin

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
[[ -e /dev/media0 ]] || {
    echo "ERROR: /dev/media0 not found — media controller not available."
    exit 1
}
[[ -f "$DCC_VISS" && -f "$DCC_2A" ]] || {
    echo "ERROR: DCC tuning files missing at $DCC_VISS or $DCC_2A"
    echo "       Run: init-imx219"
    exit 1
}

VIDEO_DEV=$(v4l2-ctl --list-devices 2>/dev/null \
    | awk '/j721e-csi2rx/{found=1; next} found && /\/dev\/video/{print $1; exit}')
[[ -n "$VIDEO_DEV" && -e "$VIDEO_DEV" ]] || {
    echo "ERROR: CSI capture device not found (j721e-csi2rx) — camera not detected or driver not loaded."
    exit 1
}
echo "[imx219] CSI capture device: ${VIDEO_DEV}"

# ── Display-manager handling ───────────────────────────────────────────────────
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
    pkill -f "gst-launch.*kmssink" 2>/dev/null || true
    if [[ "$DM_WAS_ACTIVE" -eq 1 && -n "$DM_NAME" ]]; then
        echo "[imx219] Restarting $DM_NAME..."
        systemctl start "$DM_NAME" || true
    fi
}
trap _cleanup EXIT

if [[ "$DM_WAS_ACTIVE" -eq 1 ]]; then
    echo "[imx219] Stopping $DM_NAME (will restart on exit)..."
    systemctl stop "$DM_NAME"
    sleep 1
fi

# ── Configure CSI media pipeline ──────────────────────────────────────────────
echo "[imx219] Configuring CSI pipeline: 1920x1080 RGGB8 @ ${FPS} fps"
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
echo "[imx219] Starting KMS preview — press Ctrl+C to exit"
gst-launch-1.0 \
    v4l2src device="${VIDEO_DEV}" io-mode=5 ! \
    queue max-size-buffers=2 leaky=downstream ! \
    "video/x-bayer,format=rggb,width=1920,height=1080,framerate=${FPS}/1" ! \
    tiovxisp sensor-name=SENSOR_SONY_IMX219_RPI \
        dcc-isp-file="${DCC_VISS}" \
        sink_0::dcc-2a-file="${DCC_2A}" \
        format-msb=7 ! \
    "video/x-raw,format=NV12,width=1920,height=1080" ! \
    kmssink sync=false driver-name=tidss force-modesetting=true
