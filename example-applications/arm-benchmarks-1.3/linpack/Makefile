#   ----------------------------------------------------------------------------
#  @file   Makefile
#
#  @path   
#
#  @desc   Makefile for linpack (Debug and Release versions)
#
#  @ver    1.10
#   ----------------------------------------------------------------------------
#  Copyright (c) Texas Instruments Incorporated 2002-2009
#
#  Use of this software is controlled by the terms and conditions found in the
#  license agreement under which this software has been supplied or provided.
#

#   ----------------------------------------------------------------------------
#   Included defined variables
#   ----------------------------------------------------------------------------
-include ../../../Rules.make

#   ----------------------------------------------------------------------------
#   Variables passed in externally
#   ----------------------------------------------------------------------------
CFLAGS ?=
CROSS_COMPILE ?=

#   ----------------------------------------------------------------------------
#   Name of the Linux compiler
#   ----------------------------------------------------------------------------
CC := $(CROSS_COMPILE)gcc

#   ----------------------------------------------------------------------------
#   General options, sources and libraries
#   ----------------------------------------------------------------------------
NAME := linpack
SRCS := linpack.c
HDRS :=
OBJS :=
DEBUG :=
BIN := linpack


#   ----------------------------------------------------------------------------
#   Compiler and Linker flags for Debug
#   ----------------------------------------------------------------------------
OBJDIR_D := Debug
BINDIR_D := $(OBJDIR_D)
LIBS_D := $(LIBS)
OBJS_D := $(SRCS:%.c=$(OBJDIR_D)/%.o)
ALL_DEBUG := -g3 -gdwarf-2

#   ----------------------------------------------------------------------------
#   Compiler and Linker flags for Release
#   ----------------------------------------------------------------------------
OBJDIR_R := Release
BINDIR_R := $(OBJDIR_R)
LIBS_R := $(LIBS)
OBJS_R := $(SRCS:%.c=$(OBJDIR_R)/%.o)
ALL_RELEASE := -O3

#   ----------------------------------------------------------------------------
#   Compiler include directories 
#   ----------------------------------------------------------------------------
INCLUDES := 

#   ----------------------------------------------------------------------------
#   All compiler options to be passed to the command line
#   ----------------------------------------------------------------------------
ALL_CFLAGS := $(INCLUDES)                  \
              -fmessage-length=0           \
              -c                           \
              $(CFLAGS)

ALL_LDFLAGS := -Wl,-O1                     \
               -Wl,--hash-style=gnu        \
               -lm $(LDFLAGS)

#   ----------------------------------------------------------------------------
#   Compiler symbol definitions 
#   ----------------------------------------------------------------------------
DEFS := -DUNROLL                          \
        -DSP

#   ----------------------------------------------------------------------------
#   Compiler and Linker procedure
#   From this point and on changes are very unlikely.
#   ----------------------------------------------------------------------------
.PHONY: all
all: debug release

#   ----------------------------------------------------------------------------
#   Building Debug... 
#   ----------------------------------------------------------------------------
.PHONY: debug
debug: $(BINDIR_D)/$(BIN)

$(BINDIR_D)/$(BIN): $(OBJS_D)
	@echo Compiling Debug...
	$(CC) -o $@ $(OBJS_D) $(LIBS_D) $(ALL_LDFLAGS)  -Wl,-Map,$(BINDIR_D)/$(NAME).map

$(OBJDIR_D)/%.o : %.c $(HDRS)
	@mkdir -p $(OBJDIR_D)
	$(CC) $(ALL_DEBUG) $(DEFS) $(ALL_CFLAGS) -o$@ $<

#   ----------------------------------------------------------------------------
#   Building Release... 
#   ----------------------------------------------------------------------------
.PHONY: release
release: $(BINDIR_R)/$(BIN)

$(BINDIR_R)/$(BIN): $(OBJS_R)
	@echo Compiling Release...
	$(CC) -o $@ $(OBJS_R) $(LIBS_R) $(ALL_LDFLAGS) -Wl,-Map,$(BINDIR_R)/$(NAME).map

$(OBJDIR_R)/%.o : %.c $(HDRS)
	@mkdir -p $(OBJDIR_R)
	$(CC) $(DEFS) $(ALL_RELEASE) $(ALL_CFLAGS) -o$@ $<

.PHONY: clean
clean:
	@rm -rf $(OBJDIR_D)
	@rm -rf $(OBJDIR_R)

.PHONY: install
install:
	@if [ ! -d $(DESTDIR) ] ; then \
		echo "The extracted target filesystem directory doesn't exist."; \
		echo "Please run setup.sh in the SDK's root directory and then try again."; \
		exit 1; \
	fi
	@install -d $(DESTDIR)/usr/bin
	@if [ -e Release/linpack ] ; then \
		install Release/linpack $(DESTDIR)/usr/bin/ ; \
		echo "linpack release version installed."; \
	else \
		echo "linpack release version not built - nothing to install!"; \
	fi
	@install ../scripts/runLinpack $(DESTDIR)/usr/bin

.PHONY: install_debug
install_debug:
	@if [ ! -d $(DESTDIR) ] ; then \
		echo "The extracted target filesystem directory doesn't exist."; \
		echo "Please run setup.sh in the SDK's root directory and then try again."; \
		exit 1; \
	fi
	@install -d $(DESTDIR)/usr/bin
	@if [ -e Debug/linpack ] ; then \
		install Debug/linpack $(DESTDIR)/usr/bin/ ; \
		echo "linpack debug version installed."; \
		echo "NOTE: DEBUG VERSION WILL NOT YIELD MEANINGFUL BENCHMARK RESULTS."; \
	else \
		echo "linpack debug version not built - nothing to install!"; \
	fi
	@install ../scripts/runLinpack $(DESTDIR)/usr/bin
