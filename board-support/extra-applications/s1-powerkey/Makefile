# Makefile for s1-powerkey — Python daemon, no compilation needed.
# Called by the flash tool with:
#   make install APP_INSTALL_DIR=$ROOTFS_MNT/usr/bin

.PHONY: all clean install

LOCAL_BIN := $(APP_INSTALL_DIR)/../local/bin

all:
	@echo "[s1-powerkey] Nothing to build (Python script)"

clean:
	@echo "[s1-powerkey] Nothing to clean"

install:
	@echo "[s1-powerkey] Installing to $(LOCAL_BIN)/s1-powerkey"
	install -m 0755 -D s1-powerkey.py $(LOCAL_BIN)/s1-powerkey
	@echo "[s1-powerkey] Done."
