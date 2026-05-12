#include "core/game.h"
#include "raylib.h"

int main() {
    initGame();

    while (!WindowShouldClose()) {
        updateGame();
        drawGame();
    }

    closeGame();
    return 0;
}