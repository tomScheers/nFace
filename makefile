PROGRAM = facetime

CC := gcc
CFLAGS := -Wall -Wextra -Iinclude -Ivendor
SRC := $(wildcard src/*.c)
OBJ := $(patsubst src/%.c, build/%.o, $(SRC))
BIN := bin/$(PROGRAM)

all: $(BIN) compile_commands.json

$(BIN): $(OBJ) | bin
	$(CC) $(CFLAGS) $^ -o $@

build/%.o: src/%.c | build
	$(CC) $(CFLAGS) -c $< -o $@

build:
	mkdir -p build

bin:
	mkdir -p bin

compile_commands.json: clean
	bear -- make $(BIN)

clean:
	rm -rf build bin compile_commands.json
