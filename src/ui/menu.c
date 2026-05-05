#include "menu.h"
#include "raylib.h"
#include "../core/state.h"

static Texture2D background;
static Texture2D arrow;

static int selectedOption = 0;
static const int totalOptions = 2;

// posições FIXAS baseadas na imagem
static int optionY[2] = {
    628, // jogar
    714  // sair
};

static int arrowX = 848;

void initMenu() {
    background = LoadTexture("assets/menus/menu_background.png");
    arrow = LoadTexture("assets/menus/seta.png");
}

void updateMenu() {
    if (IsKeyPressed(KEY_DOWN)) {
        selectedOption = (selectedOption + 1) % totalOptions;
    }

    if (IsKeyPressed(KEY_UP)) {
        selectedOption--;
        if (selectedOption < 0) selectedOption = totalOptions - 1;
    }

    if (IsKeyPressed(KEY_ENTER)) {
        switch (selectedOption) {
            case 0:
                // Jogar
                currentGameState = STATE_EXPLORATION;
                break;

            case 1:
                // Sair
                CloseWindow();
                break;
        }
    }
}

void drawMenu() {
    DrawTexture(background, 0, 0, WHITE);

    DrawTexture(arrow, arrowX, optionY[selectedOption], WHITE);
}

void unloadMenu() {
    UnloadTexture(background);
    UnloadTexture(arrow);
}