.NOTPARALLEL:

### Include Config ###
include Rules.make

### Include MakeRules ###
include makerules/Makefile_*

### Define Global Makerules
all: $(MAKE_ALL_TARGETS)
clean: $(patsubst %,%_clean,$(MAKE_ALL_TARGETS))
install: $(patsubst %,%_install,$(MAKE_ALL_TARGETS))

