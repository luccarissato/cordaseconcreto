CC = gcc
CFLAGS = -Ilibs/raylib/include
LDFLAGS = -Llibs/raylib/lib -lraylib -lopengl32 -lgdi32 -lwinmm -lm

SRC = src/main.c
OUT = ecosdomangue.exe

all:
	$(CC) $(SRC) -o $(OUT) $(CFLAGS) $(LDFLAGS)

run: all
	./$(OUT)

clean:
	del $(OUT)