#include "game_menu.h"

#include "../core/state.h"
#include "../items/inventory.h"
#include "../entities/player.h"

#include "raylib.h"
#include <stdio.h>

static Texture2D gameMenuBg;
static Texture2D gameMenuPlaceholderBg;
static Texture2D cursorArrow;
static int menuX;
static int menuY;

static const int playerMenuFrameSize = 184;
static int mainMenuCursorY[2] = {310, 620};
static int categoryCursorX[2] = {400, 880};

static ListNode* selectedItemNode = NULL;
static int targetSelection = 0;
static int mainMenuSelection = 0;
static int categorySelection = 0;
static int itemSelection = 0;

extern Inventory playerInventory;
extern Player party[4];

static int countItemsInCategory(int category);
static int isKeyItem(InventoryItem* item);
static int isConsumableItem(InventoryItem* item);
static InventoryItem* getItemInCategory(int category, int index);
static ListNode* getItemNodeInCategory(int category, int index);

static void updateItemTarget();
static void updateMainMenu();
static void updateItemCategory();
static void updateStatsMenu();

static void drawItemTarget();
static void drawMainMenu();
static void drawItemCategory();
static void drawStatsMenu();

void initGameMenu() {
    gameMenuBg = LoadTexture("assets/interface/game_menu_background.png");
    gameMenuPlaceholderBg = LoadTexture("assets/interface/game_menu_bg_placeholder.png");
    cursorArrow = LoadTexture("assets/interface/SETA_MENU.png");
    menuX = (1920 - gameMenuBg.width) / 2;
    menuY = (1080 - gameMenuBg.height) / 2;
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
            updateStatsMenu();
            break;

        case MENU_ITEM_TARGET:
            updateItemTarget();
            break;
    }
}

static void updateMainMenu() {
    if (IsKeyPressed(KEY_DOWN)) {
        mainMenuSelection++;
        if (mainMenuSelection > 1) mainMenuSelection = 0;
    }

    if (IsKeyPressed(KEY_UP)) {
        mainMenuSelection--;
        if (mainMenuSelection < 0) mainMenuSelection = 1;
    }

    if (IsKeyPressed(KEY_Z)) {
        switch(mainMenuSelection) {
            case 0:
                currentMenuState = MENU_ITEMS_CATEGORY;
                categorySelection = 0;  // Reseta categoria
                itemSelection = 0;       // Reseta item
                break;

            case 1:
                currentMenuState = MENU_ITEMS_LIST;
                break;
        }
    }

    if (IsKeyPressed(KEY_X)) {
        closeGameMenu();
    }
}


static void updateItemCategory() {

    if (IsKeyPressed(KEY_RIGHT)) {
        categorySelection = 1;
        itemSelection = 0;  // Reseta seleção ao mudar categoria
    }

    if (IsKeyPressed(KEY_LEFT)) {
        categorySelection = 0;
        itemSelection = 0;  // Reseta seleção ao mudar categoria
    }

    // Navegar entre itens da categoria
    if (IsKeyPressed(KEY_DOWN)) {
        int itemsInCategory = countItemsInCategory(categorySelection);
        itemSelection++;
        if (itemSelection >= itemsInCategory) {
            itemSelection = 0;
        }
    }

    if (IsKeyPressed(KEY_UP)) {
        int itemsInCategory = countItemsInCategory(categorySelection);
        itemSelection--;
        if (itemSelection < 0) {
            itemSelection = itemsInCategory - 1;
        }
    }

    if (IsKeyPressed(KEY_Z)) {
        // Seleciona item e vai para tela de alvo
        int itemsInCategory = countItemsInCategory(categorySelection);
        if (itemsInCategory > 0) {
            selectedItemNode = getItemNodeInCategory(categorySelection, itemSelection);
            currentMenuState = MENU_ITEM_TARGET;
            targetSelection = 0;
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
            drawStatsMenu();
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

    // seta do menu
    DrawTexture(cursorArrow, leftSectionX + 65, menuY + mainMenuCursorY[mainMenuSelection] - 30, WHITE);

    // personagens
    int slotWidth = rightSectionWidth / 4 - 13;

    for (int i = 0; i < 4; i++) {
        int slotX = rightSectionX + (slotWidth * i);
        int centerX = slotX + (slotWidth / 2);

        // HP
        DrawText(TextFormat("%d/%d", party[i].stats.currentHP, party[i].stats.maxHP), centerX - 15, menuY + 195, 20, WHITE);
        // MP
        DrawText(TextFormat("%d/%d", party[i].stats.currentMana, party[i].stats.maxMana), centerX - 15, menuY + 250, 20, WHITE);

        // sprite
        Texture2D spriteTexture = party[i].walkDown.texture.id != 0 ? party[i].walkDown.texture : party[i].front;
        int sourceX = spriteTexture.width - playerMenuFrameSize;
        if (sourceX < 0) sourceX = 0;
        Rectangle spriteSource = {(float)sourceX, 0.0f, (float)playerMenuFrameSize, (float)playerMenuFrameSize};
        if (spriteTexture.width < playerMenuFrameSize || spriteTexture.height < playerMenuFrameSize) {
            spriteSource = (Rectangle){0.0f, 0.0f, (float)spriteTexture.width, (float)spriteTexture.height};
        }

        int spriteX = centerX - ((int)spriteSource.width / 2);
        int spriteY = menuY + 460;

        DrawTextureRec(spriteTexture, spriteSource, (Vector2){(float)spriteX, (float)spriteY}, WHITE);

    }
}

static void drawItemCategory() {
    DrawTexture(gameMenuPlaceholderBg, menuX, menuY, WHITE);

    DrawText("Consumiveis", menuX + 480, menuY + 95, 36, categorySelection == 0 ? YELLOW : WHITE);
    DrawText("Itens chave", menuX + 960, menuY + 95, 36, categorySelection == 1 ? YELLOW : WHITE);
    DrawTexture(cursorArrow, menuX + categoryCursorX[categorySelection], menuY + 95, WHITE);

    int previewX = menuX + 120;
    int previewY = menuY + 200;
    int previewWidth = 350;
    int previewHeight = 100;

    InventoryItem* selectedItem = getItemInCategory(categorySelection, itemSelection);
    
    if (selectedItem != NULL) {
        int textX = previewX + 20;
        int textY = previewY + 30;

        DrawText(selectedItem->baseItem->description, textX + 450, textY, 20, LIGHTGRAY);
    } else {
        DrawText("Nenhum item", previewX + 20, previewY + 50, 25, GRAY);
    }

    int itemsInCategory = countItemsInCategory(categorySelection);
    int listStartY = previewY + previewHeight;
    
    for (int i = 0; i < itemsInCategory; i++) {
        InventoryItem* item = getItemInCategory(categorySelection, i);
        
        if (item != NULL) {
            Color color = (i == itemSelection) ? YELLOW : WHITE;
            int y = listStartY + (i * 50);

            if (i == itemSelection) {
                DrawTexture(cursorArrow, menuX + 120, y - 5, WHITE);
            }

            DrawText(TextFormat("%s x%d", item->baseItem->name, item->quantity), menuX + 220, y, 35, color);
        }
    }
}

static void updateStatsMenu() {
    if (IsKeyPressed(KEY_X)) {
        currentMenuState = MENU_GAME_MAIN;
    }
}

static void drawPlayerStatsColumn(int playerIndex, int x, int y) {
    Player* player = &party[playerIndex];
    Stats* stats = &player->stats;
    int lineHeight = 32;
    int fontSize = 18;

    DrawText(TextFormat("%s", player->name), x, y, 22, WHITE);
    y += 44;

    DrawText(TextFormat("Nivel: %d", player->level), x, y, fontSize, WHITE);
    y += lineHeight;
    DrawText(TextFormat("HP: %d/%d", stats->currentHP, stats->maxHP), x, y, fontSize, WHITE);
    y += lineHeight;
    DrawText(TextFormat("MP: %d/%d", stats->currentMana, stats->maxMana), x, y, fontSize, WHITE);
    y += lineHeight;
    DrawText(TextFormat("HP Base: %d", stats->baseHP), x, y, fontSize, WHITE);
    y += lineHeight;
    DrawText(TextFormat("Mana Base: %d", stats->baseMana), x, y, fontSize, WHITE);
    y += lineHeight;
    DrawText(TextFormat("Fortitude: %d", stats->fortitude), x, y, fontSize, WHITE);
    y += lineHeight;
    DrawText(TextFormat("Mente: %d", stats->mente), x, y, fontSize, WHITE);
    y += lineHeight;
    DrawText(TextFormat("Forca: %d", stats->forca), x, y, fontSize, WHITE);
    y += lineHeight;
    DrawText(TextFormat("Defesa: %d", stats->defesa), x, y, fontSize, WHITE);
    y += lineHeight;
    DrawText(TextFormat("Velocidade: %d", stats->velocidade), x, y, fontSize, WHITE);
    y += lineHeight;
    DrawText(TextFormat("Def. Calor: %d", stats->defCalor), x, y, fontSize, WHITE);
    y += lineHeight;
    DrawText(TextFormat("Def. Vento: %d", stats->defVento), x, y, fontSize, WHITE);
    y += lineHeight;
    DrawText(TextFormat("Def. Mare: %d", stats->defMare), x, y, fontSize, WHITE);
    y += lineHeight;
    DrawText(TextFormat("Def. Terra: %d", stats->defTerra), x, y, fontSize, WHITE);
}

static void drawStatsMenu() {
    DrawTexture(gameMenuPlaceholderBg, menuX, menuY, WHITE);

    int rightSectionX = menuX + 550;
    int rightSectionWidth = 1100;
    int slotWidth = rightSectionWidth / 4 - 13;

    for (int i = 0; i < 4; i++) {
        int slotX = rightSectionX + (slotWidth * i);
        drawPlayerStatsColumn(i, slotX - 200, menuY + 165);
    }
}

void unloadGameMenu() {
    UnloadTexture(gameMenuBg);
    UnloadTexture(gameMenuPlaceholderBg);
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
        currentMenuState = MENU_ITEMS_CATEGORY;

        int itemsInCategory = countItemsInCategory(categorySelection);
        if ( itemSelection >= itemsInCategory) {
            itemSelection = itemsInCategory - 1;
        }
    }

    if (IsKeyPressed(KEY_X)) {
        currentMenuState = MENU_ITEMS_CATEGORY;
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

static int isConsumableItem(InventoryItem* item) {
    ItemType type = item->baseItem->type;
    return (type == ITEM_HEAL || type == ITEM_MANA || 
            type == ITEM_BUFF || type == ITEM_CURE || 
            type == ITEM_REVIVE || type == ITEM_STAT_BOOST);
}

static int isKeyItem(InventoryItem* item) {
    return item->baseItem->type == ITEM_KEY;
}

static int countItemsInCategory(int category) {
    int count = 0;
    if (playerInventory.items.head == NULL) return 0;
    
    ListNode* current = playerInventory.items.head;
    do {
        InventoryItem* item = (InventoryItem*) current->data;
        int isInCategory = (category == 0) ? isConsumableItem(item) : isKeyItem(item);
        
        if (isInCategory) count++;
        current = current->next;
    } while (current != playerInventory.items.head);
    
    return count;
}

static InventoryItem* getItemInCategory(int category, int index) {
    if (playerInventory.items.head == NULL) return NULL;
    
    int count = 0;
    ListNode* current = playerInventory.items.head;
    
    do {
        InventoryItem* item = (InventoryItem*) current->data;
        int isInCategory = (category == 0) ? isConsumableItem(item) : isKeyItem(item);
        
        if (isInCategory) {
            if (count == index) {
                return item;
            }
            count++;
        }
        current = current->next;
    } while (current != playerInventory.items.head);
    
    return NULL;
}

static ListNode* getItemNodeInCategory(int category, int index) {
    if (playerInventory.items.head == NULL) return NULL;
    
    int count = 0;
    ListNode* current = playerInventory.items.head;
    
    do {
        InventoryItem* item = (InventoryItem*) current->data;
        int isInCategory = (category == 0) ? isConsumableItem(item) : isKeyItem(item);
        
        if (isInCategory) {
            if (count == index) {
                return current;
            }
            count++;
        }
        current = current->next;
    } while (current != playerInventory.items.head);
    
    return NULL;
}
