#!/bin/bash
# build.sh - Compile app_edgeai (C++ EdgeAI runtime) for Mo-62A via qemu-aarch64 chroot.
# Host x86 builds aarch64 binary using base rootfs as sysroot. Only app_edgeai binary
# is produced; sources stay out of firmware (device needs no compilation).
#   sudo bash build.sh                    # standalone -> out/app_edgeai
#   sudo bash build.sh --install <ROOTFS> # also install to <ROOTFS>/usr/local/bin/
#   KEEP_CHROOT=1 sudo -E bash build.sh   # keep chroot for debug
set -euo pipefail

SELF_DIR="$(cd "$(dirname "$0")" && pwd)"
SDK_ROOT="$(cd "$SELF_DIR/../../.." && pwd)"
BASE_TAR="$SDK_ROOT/filesystem/debian-13.5-edgeai-base-arm64.tar.xz"
CHROOT="$SELF_DIR/.build-chroot"
OUT="$SELF_DIR/out"
QEMU="$(command -v qemu-aarch64-static || echo /usr/bin/qemu-aarch64-static)"

INSTALL_ROOTFS=""
[ "${1:-}" = "--install" ] && INSTALL_ROOTFS="${2:-}"

[ "$(id -u)" -eq 0 ] || { echo "ERROR: must run as root (sudo)"; exit 1; }
[ -f "$BASE_TAR" ] || { echo "ERROR: base tar not found: $BASE_TAR"; exit 1; }
[ -x "$QEMU" ] || { echo "ERROR: qemu-aarch64-static missing"; exit 1; }
[ -z "$INSTALL_ROOTFS" ] || [ -d "$INSTALL_ROOTFS/usr/local/bin" ] || { echo "ERROR: invalid ROOTFS"; exit 1; }

cleanup() {
  for m in tmp dev/pts dev proc sys; do umount -l "$CHROOT/$m" 2>/dev/null || true; done
  [ -n "${KEEP_CHROOT:-}" ] || rm -rf "$CHROOT"
}
trap cleanup EXIT

echo "[1/6] Extract base rootfs"
rm -rf "$CHROOT"; mkdir -p "$CHROOT"
tar -C "$CHROOT" --xattrs --xattrs-include='*' --numeric-owner -xpf "$BASE_TAR"

echo "[2/6] Set up qemu chroot"
cp "$QEMU" "$CHROOT/usr/bin/"
printf '#!/bin/sh\nexit 101\n' > "$CHROOT/usr/sbin/policy-rc.d"; chmod +x "$CHROOT/usr/sbin/policy-rc.d"
mount --bind /proc "$CHROOT/proc"; mount --bind /sys "$CHROOT/sys"
mount --bind /dev "$CHROOT/dev"; mount --bind /dev/pts "$CHROOT/dev/pts"
mount -t tmpfs tmpfs "$CHROOT/tmp"

echo "[3/6] Copy sources into chroot"
rm -rf "$CHROOT/opt/ecpp"; mkdir -p "$CHROOT/opt/ecpp"
cp -a "$SELF_DIR/src" "$CHROOT/opt/ecpp/"
cp -a "$SELF_DIR/include" "$CHROOT/opt/ecpp/"
cp -a "$SELF_DIR/prebuilt" "$CHROOT/opt/ecpp/"

echo "[4/6] Compile inside chroot"
chroot "$CHROOT" /bin/bash -euo pipefail -c '
export SOC=am62a
cp -a /opt/ecpp/include/. /usr/local/include/
ORT=/usr/local/include/onnxruntime/core/session/onnxruntime_c_api.h
if [ -f "$ORT" ]; then
  ln -sf "$ORT" /usr/local/include/onnxruntime_c_api.h
  mkdir -p /usr/local/include/core/session
  ln -sf "$ORT" /usr/local/include/core/session/onnxruntime_c_api.h
fi
cp -f /opt/ecpp/prebuilt/tflite/*.a /usr/local/lib/
ar rcs /usr/local/lib/libXNNPACK.a
CF="-DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=/usr/bin/gcc -DCMAKE_CXX_COMPILER=/usr/bin/g++ -DUSE_TENSORFLOW_RT=ON -DUSE_ONNX_RT=ON -DUSE_DLR_RT=OFF -Wno-dev"
echo "  -> edgeai-dl-inferer"
cd /opt/ecpp/src/edgeai-dl-inferer && rm -rf build && mkdir build && cd build
cmake .. $CF
make -j2 edgeai_dl_inferer edgeai_pre_process edgeai_post_process
cp -f ../lib/Release/libedgeai_dl_inferer.a /usr/local/lib/
cp -f ../lib/Release/libedgeai_pre_process.a /usr/local/lib/
cp -f ../lib/Release/libedgeai_post_process.a /usr/local/lib/
echo "  -> app_edgeai"
cd /opt/ecpp/src/apps_cpp && rm -rf build && mkdir build && cd build
cmake .. $CF
make -j2 app_edgeai
file /opt/ecpp/src/apps_cpp/bin/Release/app_edgeai
'

echo "[5/6] Extract product"
APP_BIN="$CHROOT/opt/ecpp/src/apps_cpp/bin/Release/app_edgeai"
[ -f "$APP_BIN" ] || { echo "ERROR: build product missing"; exit 1; }
mkdir -p "$OUT"; cp "$APP_BIN" "$OUT/app_edgeai"
echo "  product: $OUT/app_edgeai ($(du -h "$OUT/app_edgeai" | cut -f1))"
[ -z "$INSTALL_ROOTFS" ] || { install -m755 "$APP_BIN" "$INSTALL_ROOTFS/usr/local/bin/app_edgeai"; echo "  installed to $INSTALL_ROOTFS"; }

echo "[6/6] Cleanup"
cleanup; trap - EXIT
echo "=== DONE ==="
