CC         = $(CROSS_COMPILE)gcc
TARGET     = mo-version
VERSION    ?= unknown
BUILD_DATE ?= $(shell date +%Y-%m-%d)
LOCAL_BIN  := $(APP_INSTALL_DIR)/../local/bin

CFLAGS = -O2 -DVERSION=\"$(VERSION)\" -DBUILD_DATE=\"$(BUILD_DATE)\"

.PHONY: all clean install

all: $(TARGET)

$(TARGET): mo-version.c
	$(CC) $(CFLAGS) -o $@ $<

install: $(TARGET)
	install -m 0755 -D $(TARGET) $(LOCAL_BIN)/$(TARGET)

clean:
	rm -f $(TARGET)
