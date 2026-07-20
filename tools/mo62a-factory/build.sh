#!/usr/bin/env bash
# Build the offline factory production-test bundle from ./pkg.
#
#   bash build.sh [VERSION]          -> dist/mo62a-factory-V<ver>.deb
#
# Version sources (precedence): $1 arg > $FACTORY_VERSION env > default V1.0.0.
#   - Direct build here:  bash build.sh V1.0.7
#   - From mo-62a-flash.sh: it passes its interactive "V1.0.x" version in.
#
# The .deb FILENAME uses the V-prefixed style (mo62a-factory-V1.0.0.deb) to match
# the firmware image naming; the package's internal Version: field is the numeric
# form (1.0.0) as required by Debian policy.
#
# Install on the board (offline):  sudo dpkg -i mo62a-factory-V*.deb
# Remove after production test:     sudo apt remove mo62a-factory   (or dpkg -r)
set -euo pipefail

HERE="$(cd "$(dirname "$0")" && pwd)"
PKG="$HERE/pkg"
CTRL="$PKG/DEBIAN/control"
[[ -f "$CTRL" ]] || { echo "missing $CTRL" >&2; exit 1; }

RAW_VERSION="${1:-${FACTORY_VERSION:-V1.0.0}}"
VER_NUM="${RAW_VERSION#[Vv]}"          # numeric form for control Version:
VER_TAG="V${VER_NUM}"                  # V-prefixed form for the filename
OUT="$HERE/dist/mo62a-factory-${VER_TAG}.deb"
mkdir -p "$HERE/dist"

# Stage a copy so the source tree stays pristine; stamp the requested version in.
STAGE="$(mktemp -d)"
trap 'rm -rf "$STAGE"' EXIT
cp -a "$PKG"/. "$STAGE"/

# Never ship Python bytecode caches (stale/host-specific).
find "$STAGE" -name '__pycache__' -type d -prune -exec rm -rf {} + 2>/dev/null || true
find "$STAGE" -name '*.pyc' -delete 2>/dev/null || true

# Stamp Version: into the staged control.
sed -i "s/^Version:.*/Version: ${VER_NUM}/" "$STAGE/DEBIAN/control"

# Maintainer scripts + shipped tools must be executable.
for f in preinst postinst prerm postrm; do
  [[ -f "$STAGE/DEBIAN/$f" ]] && chmod 0755 "$STAGE/DEBIAN/$f"
done
find "$STAGE/usr" -type f -exec chmod 0755 {} + 2>/dev/null || true

# --root-owner-group forces root:root inside the .deb without needing sudo here.
dpkg-deb --root-owner-group --build "$STAGE" "$OUT"

echo "== built: $OUT  (package Version: ${VER_NUM}) =="
dpkg-deb -I "$OUT" | sed -n '1,20p'
dpkg-deb -c "$OUT"
