#include "game_menu.h"

#include "../core/state.h"
#include "../items/inventory.h"
#include "../entities/player.h"

#include "raylib.h"
#include <stdio.h>

static Texture2D gameMenuBg;
static Texture2D gameMenuOptions;
static Texture2D itemMenuOptions;
static Texture2D cursorArrow;
static int menuX;
static int menuY;

static int menuCursorY[4] = {170, 240, 310, 380};
static int categoryCursorX[2] = {400, 880};

static ListNode* selectedItemNode = NULL;
static int targetSelection = 0;
static int mainMenuSelection = 0;
static int categorySelection = 0;
static int itemSelection = 0;

extern Inventory playerInventory;
extern Player party[4];

static void updateItemTarget();
static void updateMainMenu();
static void updateItemCategory();
static void updateItemList();

static void drawItemTarget();
static void drawMainMenu();
static void drawItemCategory();
static void drawItemList();

void initGameMenu() {
    gameMenuBg = LoadTexture("assets/interface/game_menu_bg_placeholder.png");
    gameMenuOptions = LoadTexture("assets/interface/game_menu_opcoes_placeholder.png");
    itemMenuOptions = LoadTexture("assets/interface/item_menu_opcoes_placeholder.png");
    cursorArrow = LoadTexture("assets/interface/seta_placeholder.png");
    menuX = (1920 - 1650) / 2;
    menuY = (1080 - 900) / 2;
}

void openGameMenu() {
    currentGameState = STATE_GAME_MENU;
    currentMenuState = MENU_GAME_MAIN;
}

void closeGameMenu() {
    currentGameState = STATE_EXPLORATION;
    currentMenuState = MENU_MAIN;
}

int isGameMenuOpen() {
    return currentMenuState != MENU_MAIN;
}

void updateGameMenu() {
    switch(currentMenuState) {
        case MENU_GAME_MAIN:
            updateMainMenu();
            break;

        case MENU_ITEMS_CATEGORY:
            updateItemCategory();
            break;

        case MENU_ITEMS_LIST:
            updateItemList();
            break;

        case MENU_ITEM_TARGET:
            updateItemTarget();
            break;
    }
}

static void updateMainMenu() {
    if (IsKeyPressed(KEY_DOWN)) {
        mainMenuSelection++;
        if (mainMenuSelection > 3) mainMenuSelection = 0;
    }

    if (IsKeyPressed(KEY_UP)) {
        mainMenuSelection--;
        if (mainMenuSelection < 0) mainMenuSelection = 3;
    }

    if (IsKeyPressed(KEY_Z)) {
        switch(mainMenuSelection) {
            case 0:
                currentMenuState = MENU_ITEMS_CATEGORY;
                break;

            case 3:
                closeGameMenu();
                break;
        }
    }

    if (IsKeyPressed(KEY_X)) {
        closeGameMenu();
    }
}

static void updateItemList() {
    int totalItems = playerInventory.items.size;
    if (totalItems <= 0) {
        if (IsKeyPressed(KEY_X)) {
            currentMenuState = MENU_ITEMS_CATEGORY;
        }

        return;
    }

    if (IsKeyPressed(KEY_DOWN)) {
        itemSelection++;
        if (itemSelection >= totalItems) {
            itemSelection = 0;
        }
    }

    if (IsKeyPressed(KEY_UP)) {
        itemSelection--;
        if (itemSelection < 0) {
            itemSelection = totalItems - 1;
        }
    }

    if (IsKeyPressed(KEY_Z)) {
        ListNode* current = playerInventory.items.head;

        for (int i = 0; i < itemSelection; i++) {
            current = current->next;
        }
        
        //aqui
        selectedItemNode = current;
        currentMenuState = MENU_ITEM_TARGET;

        if (itemSelection >= playerInventory.items.size) {
            itemSelection = playerInventory.items.size - 1;
        }
    }

    if (IsKeyPressed(KEY_X)) {
        currentMenuState = MENU_ITEMS_CATEGORY;
    }
}

static void updateItemCategory() {

    if (IsKeyPressed(KEY_RIGHT)) {
        categorySelection = 1;
    }

    if (IsKeyPressed(KEY_LEFT)) {
        categorySelection = 0;
    }

    if (IsKeyPressed(KEY_Z)) {

        if (categorySelection == 0) {
            currentMenuState = MENU_ITEMS_LIST;
        }
    }

    if (IsKeyPressed(KEY_X)) {
        currentMenuState = MENU_GAME_MAIN;
    }
}

void drawGameMenu() {
    switch(currentMenuState) {
        case MENU_GAME_MAIN:
            drawMainMenu();
            break;

        case MENU_ITEMS_CATEGORY:
            drawItemCategory();
            break;

        case MENU_ITEMS_LIST:
            drawItemList();
            break;
        case MENU_ITEM_TARGET:
            drawItemTarget();
            break;
    }
}

static void drawMainMenu() {

    DrawTexture(gameMenuBg, menuX, menuY, WHITE);

    // divisão do menu
    int leftSectionX = menuX;
    int leftSectionWidth = 550;

    int rightSectionX = menuX + 550;
    int rightSectionWidth = 1100;

    // opções da esquerda
    int optionsX = leftSectionX + (leftSectionWidth - gameMenuOptions.width) / 2;
    int optionsY = menuY + 220;

    DrawTexture(gameMenuOptions, optionsX, optionsY, WHITE);

    // seta do menu
    int arrowX = optionsX - 70;

    int arrowY[4] = {optionsY + 35, optionsY + 125, optionsY + 235, optionsY + 335};
    DrawTexture(cursorArrow, arrowX, arrowY[mainMenuSelection], WHITE);

    // personagens
    int slotWidth = rightSectionWidth / 4;

    for (int i = 0; i < 4; i++) {
        int slotX = rightSectionX + (slotWidth * i);
        int centerX = slotX + (slotWidth / 2);

        // HP
        DrawText(TextFormat("HP: %d/%d", party[i].stats.currentHP, party[i].stats.maxHP), centerX - 170, menuY + 220, 25, WHITE);
        // MP
        DrawText(TextFormat("MP: %d/%d", party[i].stats.currentMana, party[i].stats.maxMana), centerX - 170, menuY + 260, 25, WHITE);

        // sprite
        int spriteX = centerX - (party[i].front.width / 2) - 90;
        int spriteY = menuY + 360;

        DrawTexture(party[i].front, spriteX, spriteY, WHITE);

        // nome
        DrawText(party[i].name, centerX - 100, menuY + 700, 25, WHITE);
    }
}

static void drawItemCategory() {
    DrawTexture(gameMenuBg, menuX, menuY, WHITE);
    DrawTexture(itemMenuOptions, menuX + 450, menuY + 75, WHITE);
    DrawTexture(cursorArrow, menuX + categoryCursorX[categorySelection], menuY + 95, WHITE);
}

static void drawItemList() {
    DrawTexture(gameMenuBg, menuX, menuY, WHITE);
    if (playerInventory.items.head == NULL) {
        DrawText("Inventario vazio", 300, 200, 40, WHITE);
        return;
    }

    ListNode* current = playerInventory.items.head;

    int y = 200;
    int index = 0;

    do {

        InventoryItem* item = (InventoryItem*) current->data;
        Color color = (index == itemSelection) ? YELLOW: WHITE;

        if (index == itemSelection) {
            DrawTexture(cursorArrow, menuX + 120, y - 5, WHITE);
        }

        DrawText(TextFormat("%s x%d", item->baseItem->name, item->quantity), menuX + 220, y, 35, color);

        y += 50;
        current = current->next;
        index++;

    } while (
        current != playerInventory.items.head
    );
}

void unloadGameMenu() {
    UnloadTexture(gameMenuBg);
    UnloadTexture(gameMenuOptions);
    UnloadTexture(itemMenuOptions);
    UnloadTexture(cursorArrow);
}

static void updateItemTarget() {
    if (IsKeyPressed(KEY_RIGHT)) {
        targetSelection++;

        if (targetSelection > 3) {
            targetSelection = 0;
        }
    }

    if (IsKeyPressed(KEY_LEFT)) {
        targetSelection--;

        if (targetSelection < 0) {
            targetSelection = 3;
        }
    }

    if (IsKeyPressed(KEY_Z)) {
        consumeItem(&playerInventory, selectedItemNode, &party[targetSelection]);
        currentMenuState = MENU_GAME_MAIN;

        if ( itemSelection >= playerInventory.items.size) {
            itemSelection = playerInventory.items.size - 1;
        }
    }

    if (IsKeyPressed(KEY_X)) {
        currentMenuState = MENU_ITEMS_LIST;
    }
}

static void drawItemTarget() {
    drawMainMenu();

    int rightSectionX = menuX + 550;
    int rightSectionWidth = 1100;

    int spacing = rightSectionWidth / 4;

    for (int i = 0; i < 4; i++) {
        int centerX = rightSectionX + (spacing * i) + (spacing / 2);

        if (i == targetSelection) {
            DrawTexture(cursorArrow, centerX - 200, menuY + 520, WHITE);
        }
    }

    DrawText("Escolha um personagem", menuX + 650, menuY + 800, 30, WHITE);
}