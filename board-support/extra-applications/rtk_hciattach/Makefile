SYSROOT ?=
CFLAGS  := -Wall -g $(if $(SYSROOT),--sysroot=$(SYSROOT),)
CC      := $(CROSS_COMPILE)gcc
all: rtk_hciattach
OBJS := hciattach.o hciattach_rtk.o hciattach_h4.o rtb_fwc.o

rtk_hciattach: $(OBJS)
	$(CC) $(CFLAGS) -o rtk_hciattach $(OBJS)

install: rtk_hciattach
	@echo "[rtk_hciattach] Installing..."

	@install -m 0700 -D rtk_hciattach $(APP_INSTALL_DIR)/rtk_hciattach

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)

clean:
	rm -f $(OBJS)  rtk_hciattach

tags: FORCE
	ctags -R
	find ./ -name "*.h" -o -name "*.c" -o -name "*.cc" -o -name "*.cpp" > cscope.files
	cscope -bkq -i cscope.files
PHONY += FORCE
FORCE:
