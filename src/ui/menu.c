#include "menu.h"
#include "raylib.h"
#include "../core/state.h"

static Texture2D background;
static Texture2D arrow;

static int selectedOption = 0;
static const int totalOptions = 2;

// caixas invisíveis sobre os botões desenhados no asset
static const Rectangle optionBoxes[2] = {
    { 800.0f, 780.0f, 260.0f, 64.0f }, // jogar
    { 800.0f, 866.0f, 260.0f, 64.0f }  // sair
};

static const float arrowX = 748.0f;
static const float arrowOffsetY = 6.0f;

static void activateSelectedOption(void) {
    switch (selectedOption) {
        case 0:
            currentGameState = STATE_EXPLORATION;
            break;

        case 1:
            CloseWindow();
            break;
    }
}

void initMenu() {
    background = LoadTexture("assets/interface/TELA_INICIO_ATUALIZADA.png");
    arrow = LoadTexture("assets/interface/SETA_MENU.png");
}

void updateMenu() {
    Vector2 mousePosition = GetMousePosition();

    if (IsKeyPressed(KEY_DOWN)) {
        selectedOption = (selectedOption + 1) % totalOptions;
    }

    if (IsKeyPressed(KEY_UP)) {
        selectedOption--;
        if (selectedOption < 0) selectedOption = totalOptions - 1;
    }

    for (int i = 0; i < totalOptions; i++) {
        if (CheckCollisionPointRec(mousePosition, optionBoxes[i])) {
            selectedOption = i;

            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                activateSelectedOption();
            }
        }
    }

    if (IsKeyPressed(KEY_ENTER)) {
        activateSelectedOption();
    }
}

void drawMenu() {
    DrawTexture(background, 0, 0, WHITE);

    DrawTexture(arrow, (int)arrowX, (int)(optionBoxes[selectedOption].y + arrowOffsetY), WHITE);
}

void unloadMenu() {
    UnloadTexture(background);
    UnloadTexture(arrow);
}