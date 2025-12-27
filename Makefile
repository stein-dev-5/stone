CC=gcc
CFLAGS=-std=c11 -O2 -Wall
LIBS=-lws2_32
SRC=src/server.c src/game.c
OUT=build/stone_game.exe

all: $(OUT)

$(OUT): $(SRC)
	@mkdir -p build
	$(CC) $(CFLAGS) $(SRC) -o $(OUT) $(LIBS)

clean:
	rm -rf build

run: $(OUT)
	./$(OUT)

.PHONY: all clean run

