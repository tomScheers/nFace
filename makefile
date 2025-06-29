PROGRAM = nface

CC := gcc
CFLAGS := -Wall -Wextra -Iinclude -g -O3
LDFLAGS := -lncurses

SRC := $(shell find src -name '*.c')
OBJ := $(patsubst src/%.c, build/%.o, $(SRC))
BIN := bin/$(PROGRAM)

VERSION := $(shell git describe --tags --abbrev=0)

SRCMAN = man/
MANPAGE = nface.1
COMPMAN = nface.1.gz 
MANDIR = /usr/share/man/man1/

.PHONY: all clean install uninstall

$(BIN): $(OBJ) | bin
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

build/%.o: src/%.c version.h
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

version.h:
	echo '#ifndef VERSION_H' > include/version.h; \
	echo '#define VERSION_H' >> include/version.h; \
	echo '' >> include/version.h; \
	echo '#define PROJECT_VERSION "'$$(git describe --tags --abbrev=0 2>/dev/null || echo dev)'"' >> include/version.h; \
	echo '' >> include/version.h; \
	echo '#endif // VERSION_H' >> include/version.h

build:
	mkdir -p build

bin:
	mkdir -p bin

clean:
	rm -rf bin build
	$(RM) $(SRCMAN)$(COMPMAN)

# Install the binary to /usr/local/bin with proper permissions
install: $(BIN)
	install -Dm755 $(BIN) /usr/local/bin/$(PROGRAM)
	gzip < $(SRCMAN)$(MANPAGE) > $(SRCMAN)$(COMPMAN)
	cp -f -r $(SRCMAN)$(COMPMAN) $(MANDIR)
	mandb

# Uninstall the binary from /usr/local/bin
uninstall:
	rm -f /usr/local/bin/$(PROGRAM)
	rm -f $(MANDIR)/$(COMPMAN)
	mandb
