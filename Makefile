CC = gcc
CFLAGS = -I C:\SDL2\SDL2-2.32.10\x86_64-w64-mingw32\include\SDL2
LDFLAGS = -L C:\SDL2\SDL2-2.32.10\x86_64-w64-mingw32\lib -lmingw32 -lSDL2main -lSDL2
SRC = src/main.c src/chip8.c
OBJ = chip8_emulator.exe
all: $(SRC)
	$(CC) $(SRC) $(CFLAGS) $(LDFLAGS) -o $(OBJ)