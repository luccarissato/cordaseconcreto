CC = gcc
CFLAGS = -Ilibs/raylib/include -Isrc
LDFLAGS = -Llibs/raylib/lib -lraylib -lopengl32 -lgdi32 -lwinmm -lm

SRC = $(wildcard src/*.c src/*/*.c src/*/*/*.c)
OUT = cordaseconcreto.exe

all:
	$(CC) $(SRC) -o $(OUT) $(CFLAGS) $(LDFLAGS)

run: all
	.\$(OUT)

clean:
	del $(OUT)