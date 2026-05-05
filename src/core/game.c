#include "raylib.h"
#include "state.h"
#include "../ui/menu.h"

void initGame() {
    InitWindow(1920, 1080, "Cordas & Concreto");
    SetTargetFPS(60);

    initMenu();
}

void updateGame() {
    switch (currentGameState) {
        case STATE_MENU:
            updateMenu();
            break;

        case STATE_EXPLORATION:
            // ainda vazio
            break;
    }
}

void drawGame() {
    BeginDrawing();
    ClearBackground(BLACK);

    switch (currentGameState) {
        case STATE_MENU:
            drawMenu();
            break;

        case STATE_EXPLORATION:
            DrawText("JOGO INICIADO", 300, 300, 20, WHITE);
            break;
    }

    EndDrawing();
}

void closeGame() {
    unloadMenu();
    CloseWindow();
}