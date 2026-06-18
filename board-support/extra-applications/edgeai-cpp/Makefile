# Makefile for edgeai-cpp — C/C++ Edge AI runtime + on-device SDK for MO-62A.
# Invoked by mo-62a-flash.sh's install_external_apps_into_rootfs(), which calls
# `make`, then `make install APP_INSTALL_DIR=<rootfs>/usr/bin ROOTFS_TARBALL=<tar>`.
#
# Compilation is delegated to build.sh, which uses a qemu-aarch64 chroot inside
# the selected Debian base rootfs. The `install` target builds and installs the
# whole SDK (binary + headers + libs + CMake package + examples) into the target
# rootfs, so the heavy chroot build runs once (at install), not twice.
#
# Standalone (no flash): run `sudo bash build.sh` directly.

BUILD_SCRIPT := $(CURDIR)/build.sh
# flash passes APP_INSTALL_DIR=<rootfs>/usr/bin; strip it to get the rootfs root.
ROOTFS       := $(APP_INSTALL_DIR:/usr/bin=)
BASE_TAR_ARG := $(if $(ROOTFS_TARBALL),--base-tar $(ROOTFS_TARBALL),)

.PHONY: all clean install

all:
	@echo "[edgeai-cpp] Build is performed by the 'install' target (it needs the"
	@echo "             target ROOTFS). For a standalone build run: sudo bash build.sh"

install:
	@[ -n "$(ROOTFS)" ] || { echo "[edgeai-cpp] ERROR: APP_INSTALL_DIR not set"; exit 1; }
	@echo "[edgeai-cpp] Building + installing SDK into $(ROOTFS) ..."
	bash $(BUILD_SCRIPT) --install "$(ROOTFS)" $(BASE_TAR_ARG)
	@echo "[edgeai-cpp] Done."

clean:
	@echo "[edgeai-cpp] Cleaning build artifacts..."
	rm -rf $(CURDIR)/out $(CURDIR)/.build-chroot
	@echo "[edgeai-cpp] Clean complete."
