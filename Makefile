CC      ?= gcc
CFLAGS  ?= -std=c11 -Wall -Wextra -Wpedantic -D_POSIX_C_SOURCE=200809L
INC      = -Isrc/core -Isrc/util -Isrc/ui -Isrc/player -Isrc/battle \
           -Isrc/gacha -Isrc/inventory -Isrc/shop -Isrc/storage \
           -Isrc/dungeon -Isrc/platform
SRC      = main.c $(wildcard src/*/*.c)
BIN      = bin/game

all: $(BIN)

$(BIN): $(SRC)
	@mkdir -p bin
	$(CC) $(CFLAGS) $(INC) -o $@ $(SRC)

start: $(BIN)
	./$(BIN)

test: $(BIN)
	sh tests/run_tests.sh

clean:
	rm -rf bin tests/out_last.txt

.PHONY: all start test clean
