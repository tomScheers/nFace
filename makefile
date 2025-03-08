PROGRAM = nface
CC := gcc
CFLAGS := -Wall -Wextra -Iinclude -g -O3
LDFLAGS := -lncurses
SRC := $(wildcard src/*.c)
OBJ := $(patsubst src/%.c, build/%.o, $(SRC))
BIN := bin/$(PROGRAM)

all: $(BIN)

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
