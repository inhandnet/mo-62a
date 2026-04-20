BUILD_DIR  := build
LOCAL_BIN  := $(APP_INSTALL_DIR)/../local/bin

CMAKE_ARGS := \
	-DCMAKE_C_COMPILER=$(CROSS_COMPILE)gcc \
	-DCMAKE_SYSTEM_NAME=Linux \
	-DCMAKE_SYSTEM_PROCESSOR=aarch64 \
	-Dstatic_exe=ON

.PHONY: all clean install

all:
	mkdir -p $(BUILD_DIR)
	cd $(BUILD_DIR) && cmake $(CMAKE_ARGS) ..
	$(MAKE) -C $(BUILD_DIR)

install: all
	install -m 4755 -D $(BUILD_DIR)/k3conf $(LOCAL_BIN)/k3conf

clean:
	rm -rf $(BUILD_DIR)
