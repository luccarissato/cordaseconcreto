#include "game_menu.h"

#include "../core/state.h"
#include "../items/inventory.h"
#include "../entities/player.h"

#include "raylib.h"
#include <stdio.h>
#include <string.h>

static Texture2D gameMenuBg;
static Texture2D gameMenuItemsBg;
static Texture2D gameMenuStatsBg;
static Texture2D cursorArrow;
static int menuX;
static int menuY;

static const int playerMenuFrameSize = 184;
static int mainMenuCursorY[2] = {310, 620};

static ListNode* selectedItemNode = NULL;
static int targetSelection = 0;
static int mainMenuSelection = 0;
static int categorySelection = 0;
static int itemSelection = 0;

static const int itemRowsPerColumn = 9;
static const int itemPageSize = 18;

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
    gameMenuItemsBg = LoadTexture("assets/interface/game_menu_itens_bg.png");
    gameMenuStatsBg = LoadTexture("assets/interface/game_menu_stats_bg.png");
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
        if (itemsInCategory <= 0) {
            itemSelection = 0;
        } else {
            itemSelection++;
            if (itemSelection >= itemsInCategory) {
                itemSelection = 0;
            }
        }
    }

    if (IsKeyPressed(KEY_UP)) {
        int itemsInCategory = countItemsInCategory(categorySelection);
        if (itemsInCategory <= 0) {
            itemSelection = 0;
        } else {
            itemSelection--;
            if (itemSelection < 0) {
                itemSelection = itemsInCategory - 1;
            }
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

static int getCenteredTextureX(Texture2D texture) {
    return (GetScreenWidth() - texture.width) / 2;
}

static int getCenteredTextureY(Texture2D texture) {
    return (GetScreenHeight() - texture.height) / 2;
}

static void drawCenteredTextInBox(const char* text, int x, int y, int width, int fontSize, Color color) {
    if (text == NULL) {
        return;
    }

    DrawText(text, x + ((width - MeasureText(text, fontSize)) / 2), y, fontSize, color);
}

static void drawTextClippedWidth(const char* text, int x, int y, int fontSize, int maxWidth, Color color) {
    if (text == NULL) {
        return;
    }

    char buffer[96];
    snprintf(buffer, sizeof(buffer), "%s", text);

    while (buffer[0] != '\0' && MeasureText(buffer, fontSize) > maxWidth) {
        size_t len = strlen(buffer);
        if (len <= 3) {
            break;
        }

        buffer[len - 1] = '\0';
        buffer[len - 2] = '.';
        buffer[len - 3] = '.';
        buffer[len - 4] = '.';
    }

    DrawText(buffer, x, y, fontSize, color);
}

static void drawWrappedText(const char* text, int x, int y, int fontSize, int maxWidth, int lineHeight, int maxLines, Color color) {
    if (text == NULL || maxLines <= 0) {
        return;
    }

    char source[512];
    snprintf(source, sizeof(source), "%s", text);

    char line[256] = "";
    int drawnLines = 0;
    char* token = strtok(source, " ");

    while (token != NULL && drawnLines < maxLines) {
        char candidate[256];
        if (line[0] == '\0') {
            snprintf(candidate, sizeof(candidate), "%s", token);
        } else {
            snprintf(candidate, sizeof(candidate), "%s %s", line, token);
        }

        if (MeasureText(candidate, fontSize) <= maxWidth) {
            snprintf(line, sizeof(line), "%s", candidate);
        } else {
            DrawText(line, x, y + (drawnLines * lineHeight), fontSize, color);
            drawnLines++;
            snprintf(line, sizeof(line), "%s", token);
        }

        token = strtok(NULL, " ");
    }

    if (line[0] != '\0' && drawnLines < maxLines) {
        DrawText(line, x, y + (drawnLines * lineHeight), fontSize, color);
    }
}

static int getVisibleItemPageStart(void) {
    if (itemSelection < 0) {
        return 0;
    }

    return (itemSelection / itemPageSize) * itemPageSize;
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
    int bgX = getCenteredTextureX(gameMenuItemsBg);
    int bgY = getCenteredTextureY(gameMenuItemsBg);
    DrawTexture(gameMenuItemsBg, bgX, bgY, WHITE);

    int tabY = bgY + 127;
    int leftTabX = bgX + 285;
    int rightTabX = bgX + 840;
    int tabWidth = 520;
    drawCenteredTextInBox("Consumiveis", leftTabX, tabY, tabWidth, 32, categorySelection == 0 ? YELLOW : BROWN);
    drawCenteredTextInBox("Itens chave", rightTabX, tabY, tabWidth, 32, categorySelection == 1 ? YELLOW : BROWN);

    int selectedTabX = categorySelection == 0 ? leftTabX : rightTabX;
    DrawTexture(cursorArrow, selectedTabX - 55, tabY + 1, WHITE);

    InventoryItem* selectedItem = getItemInCategory(categorySelection, itemSelection);
    
    if (selectedItem != NULL) {
        DrawText(selectedItem->baseItem->name, bgX + 1040, bgY + 290, 28, BROWN);
        drawWrappedText(selectedItem->baseItem->description, bgX + 1040, bgY + 340, 22, 420, 30, 7, DARKBROWN);
    } else {
        DrawText("Nenhum item", bgX + 150, bgY + 290, 28, GRAY);
    }

    int itemsInCategory = countItemsInCategory(categorySelection);
    int firstVisibleItem = getVisibleItemPageStart();
    int lastVisibleItem = firstVisibleItem + itemPageSize;
    if (lastVisibleItem > itemsInCategory) {
        lastVisibleItem = itemsInCategory;
    }
    
    for (int i = firstVisibleItem; i < lastVisibleItem; i++) {
        InventoryItem* item = getItemInCategory(categorySelection, i);
        
        if (item != NULL) {
            int pageIndex = i - firstVisibleItem;
            int column = pageIndex / itemRowsPerColumn;
            int row = pageIndex % itemRowsPerColumn;
            int x = bgX + 150 + (column * 430);
            int y = bgY + 285 + (row * 58);
            Color color = (i == itemSelection) ? YELLOW : DARKBROWN;

            if (i == itemSelection) {
                DrawTexture(cursorArrow, x - 72, y - 7, WHITE);
            }

            drawTextClippedWidth(TextFormat("%s x%d", item->baseItem->name, item->quantity), x, y, 26, 350, color);
        }
    }

    if (itemsInCategory > itemPageSize) {
        DrawText(TextFormat("%d-%d/%d", firstVisibleItem + 1, lastVisibleItem, itemsInCategory), bgX + 150, bgY + 830, 20, BROWN);
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
    int fontSize = 17;
    int width = 275;
    int textX = x + 18;

    drawCenteredTextInBox(player->name, x, y, width, 22, DARKBROWN);

    int rowY[14] = {
        y + 82,
        y + 127,
        y + 172,
        y + 217,
        y + 262,
        y + 307,
        y + 352,
        y + 397,
        y + 442,
        y + 487,
        y + 532,
        y + 577,
        y + 622,
        y + 667
    };

    DrawText(TextFormat("Nivel: %d", player->level), textX, rowY[0], fontSize, DARKBROWN);
    DrawText(TextFormat("HP: %d/%d", stats->currentHP, stats->maxHP), textX, rowY[1], fontSize, DARKBROWN);
    DrawText(TextFormat("MP: %d/%d", stats->currentMana, stats->maxMana), textX, rowY[2], fontSize, DARKBROWN);
    DrawText(TextFormat("HP Base: %d", stats->baseHP), textX, rowY[3], fontSize, DARKBROWN);
    DrawText(TextFormat("Mana Base: %d", stats->baseMana), textX, rowY[4], fontSize, DARKBROWN);
    DrawText(TextFormat("Fortitude: %d", stats->fortitude), textX, rowY[5], fontSize, DARKBROWN);
    DrawText(TextFormat("Mente: %d", stats->mente), textX, rowY[6], fontSize, DARKBROWN);
    DrawText(TextFormat("Forca: %d", stats->forca), textX, rowY[7], fontSize, DARKBROWN);
    DrawText(TextFormat("Defesa: %d", stats->defesa), textX, rowY[8], fontSize, DARKBROWN);
    DrawText(TextFormat("Velocidade: %d", stats->velocidade), textX, rowY[9], fontSize, DARKBROWN);
    DrawText(TextFormat("Def. Calor: %d", stats->defCalor), textX, rowY[10], fontSize, DARKBROWN);
    DrawText(TextFormat("Def. Vento: %d", stats->defVento), textX, rowY[11], fontSize, DARKBROWN);
    DrawText(TextFormat("Def. Mare: %d", stats->defMare), textX, rowY[12], fontSize, DARKBROWN);
    DrawText(TextFormat("Def. Terra: %d", stats->defTerra), textX, rowY[13], fontSize, DARKBROWN);
}

static void drawStatsMenu() {
    int bgX = getCenteredTextureX(gameMenuStatsBg);
    int bgY = getCenteredTextureY(gameMenuStatsBg);
    DrawTexture(gameMenuStatsBg, bgX, bgY, WHITE);

    int columnX[4] = {
        bgX + 152,
        bgX + 508,
        bgX + 864,
        bgX + 1220
    };

    for (int i = 0; i < 4; i++) {
        drawPlayerStatsColumn(i, columnX[i], bgY + 165);
    }
}

void unloadGameMenu() {
    UnloadTexture(gameMenuBg);
    UnloadTexture(gameMenuItemsBg);
    UnloadTexture(gameMenuStatsBg);
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
