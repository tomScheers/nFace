PROGRAM = nface

CC := gcc
CFLAGS := -Wall -Wextra -Iinclude -g -O3
LDFLAGS := -lncurses

SRC := $(wildcard src/*.c)
OBJ := $(patsubst src/%.c, build/%.o, $(SRC))
BIN := bin/$(PROGRAM)

<<<<<<< HEAD
all: $(BIN)
=======
.PHONY: all clean install uninstall compile_commands.json

all: $(BIN) compile_commands.json
>>>>>>> aa0803905a6940df0528bbece9ef5ad67dda4a08

$(BIN): $(OBJ) | bin
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

build/%.o: src/%.c | build
	$(CC) $(CFLAGS) -c $< -o $@

build:
	mkdir -p build

bin:
	mkdir -p bin

clean:
	rm -rf build bin compile_commands.json

# Install the binary to /usr/local/bin with proper permissions
install: $(BIN)
	install -Dm755 $(BIN) /usr/local/bin/$(PROGRAM)

# Uninstall the binary from /usr/local/bin
uninstall:
	rm -f /usr/local/bin/$(PROGRAM)
