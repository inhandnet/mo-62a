# Makefile for imx219-preview — shell script, no compilation needed.
# Called by the flash tool with:
#   make install APP_INSTALL_DIR=$ROOTFS_MNT/usr/bin

.PHONY: all clean install

# Derive /usr/local/bin inside the rootfs from APP_INSTALL_DIR ($ROOTFS_MNT/usr/bin)
LOCAL_BIN := $(APP_INSTALL_DIR)/../local/bin

all:
	@echo "[imx219-preview] Nothing to build (shell script only)"

clean:
	@echo "[imx219-preview] Nothing to clean"

install:
	@echo "[imx219-preview] Installing to $(LOCAL_BIN)/imx219-preview.sh"
	install -m 0755 -D imx219-preview.sh $(LOCAL_BIN)/imx219-preview.sh
	@echo "[imx219-preview] Done."
