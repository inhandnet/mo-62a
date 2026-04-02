-include DIRS
-include ../../Rules.make

all: debug release

debug:
	for dir in $(DIRS); do \
                make -C $$dir debug; \
	done

release:
	for dir in $(DIRS); do \
                make -C $$dir release; \
	done

clean:
	for dir in $(DIRS); do \
                make -C $$dir clean; \
	done

install:
	for dir in $(DIRS); do \
                make -C $$dir install; \
	done

install_debug:
	for dir in $(DIRS); do \
                make -C $$dir install_debug; \
	done
