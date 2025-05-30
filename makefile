PROGRAM = nface

CC := gcc
CFLAGS := -Wall -Wextra -Iinclude -g -O3
LDFLAGS := -lncurses

SRC := $(shell find src -name '*.c')
OBJ := $(patsubst src/%.c, build/%.o, $(SRC))
BIN := bin/$(PROGRAM)

.PHONY: all clean install uninstall

$(BIN): $(OBJ) | bin
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

build/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

build:
	mkdir -p build

bin:
	mkdir -p bin

# Install the binary to /usr/local/bin with proper permissions
install: $(BIN)
	install -Dm755 $(BIN) /usr/local/bin/$(PROGRAM)

# Uninstall the binary from /usr/local/bin
uninstall:
	rm -f /usr/local/bin/$(PROGRAM)
