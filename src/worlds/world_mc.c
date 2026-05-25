#include "world_mc.h"

#include "worlds.h"
#include "../core/game.h"
#include "../core/collision.h"
#include "../items/inventory.h"
#include "../items/game_items.h"
#include "../combat/boss_ai.h"
#include "../data/dialogues/quest_npc.h"
#include "../entities/npc.h"
#include "raylib.h"
#include <string.h>
#include <stdlib.h>
#include <stddef.h>

static const Vector2 WORLD_MC_PARTY_SPAWN = {1680.0f, 680.0f};
static const Vector2 WORLD_CS_RIGHT_EDGE_SPAWN = {1600.0f, 820.0f};
static const char* WORLD_MC_MAP_PATH = "assets/cenarios/MZERO_ATUALIZADO.png";
static const char* WORLD_MC_TREE_PATH = "assets/cenarios/ARVORE2_MZERO.png";
static const float WORLD_MC_BORDER_THICKNESS = 128.0f;

static const Rectangle WORLD_MC_HORIZONTAL_BLOCKER = {0.0f, 240.0f, 1920.0f, 12.0f};

static const Vector2 WORLD_MC_TREE_POSITIONS_NORMAL[] = {
    {1870.0f, 565.0f},
    {140.0f, 580.0f}
};

static const Vector2 WORLD_MC_TREE_POSITIONS_RD[] = {
    {220.0f, 705.0f},
    {1735.0f, 690.0f}
};

static const Vector2 WORLD_MC_STATUE_POSITION = {1860.0f, 650.0f};
static const Rectangle WORLD_MC_NEXT_WORLD_TRIGGER = {-75.0f, 740.0f, 160.0f, 280.0f};
static const WorldTransitionZone WORLD_MC_TRANSITIONS[] = {
    {WORLD_MC_NEXT_WORLD_TRIGGER, WORLD_TRANSITION_NEXT, WORLD_CS_RIGHT_EDGE_SPAWN}
};

static Rectangle WORLD_MC_EDGE_BLOCKERS[5];
static int WORLD_MC_EDGE_BLOCKER_COUNT = 0;
static Rectangle WORLD_MC_TREE_BLOCKERS[4];
static int WORLD_MC_TREE_BLOCKER_COUNT = 0;
static Texture2D WORLD_MC_TREE_TEXTURE = {0};
static Texture2D WORLD_MC_STATUE_TEXTURE = {0};

static void drawTreeAtBaseCenter(Vector2 baseCenter) {
    if (WORLD_MC_TREE_TEXTURE.id == 0) {
        return;
    }

    Vector2 drawPosition = {
        baseCenter.x - ((float)WORLD_MC_TREE_TEXTURE.width * 0.5f),
        baseCenter.y - (float)WORLD_MC_TREE_TEXTURE.height
    };

    DrawTextureV(WORLD_MC_TREE_TEXTURE, drawPosition, WHITE);
}

static void drawStatueAtBaseCenter(Vector2 baseCenter) {
    if (WORLD_MC_STATUE_TEXTURE.id == 0) {
        return;
    }

    Vector2 drawPosition = {
        baseCenter.x - ((float)WORLD_MC_STATUE_TEXTURE.width * 0.5f),
        baseCenter.y - (float)WORLD_MC_STATUE_TEXTURE.height
    };

    DrawTextureV(WORLD_MC_STATUE_TEXTURE, drawPosition, WHITE);
}

static void drawTriggerRect(Rectangle rect, Color color) {
    DrawRectangleLinesEx(rect, 2.0f, color);
}

static Rectangle getWorldMCTreeRect(Vector2 baseCenter) {
    if (WORLD_MC_TREE_TEXTURE.id == 0) {
        return (Rectangle){0.0f, 0.0f, 0.0f, 0.0f};
    }

    return (Rectangle){
        baseCenter.x - ((float)WORLD_MC_TREE_TEXTURE.width * 0.325f),
        baseCenter.y - (float)WORLD_MC_TREE_TEXTURE.height,
        (float)WORLD_MC_TREE_TEXTURE.width * 0.65f,
        (float)WORLD_MC_TREE_TEXTURE.height * 0.9f
    };
}

extern Inventory playerInventory;
extern Player party[PARTY_SIZE];

NPC worldMCQuestNpc;

static int hasItemInInventory(const char* name) {
    ListNode* cur = playerInventory.items.head;
    int iterations = playerInventory.items.size;
    for (int i = 0; i < iterations && cur != NULL; i++) {
        InventoryItem* it = (InventoryItem*)cur->data;
        if (it != NULL && strcmp(it->baseItem->name, name) == 0) return 1;
        cur = cur->next;
    }
    return 0;
}

static void removeItemFromInventoryByName(const char* name) {
    ListNode* cur = playerInventory.items.head;
    while (cur != NULL) {
        InventoryItem* it = (InventoryItem*)cur->data;
        if (it != NULL && strcmp(it->baseItem->name, name) == 0) {
            free(it);
            ListNode* toRemove = cur;
            cur = cur->next;
            removeNode(&playerInventory.items, toRemove);
            return;
        }
        cur = cur->next;
    }
}

static int worldMCQuestPreInteract(NPC* npc) {
    if (hasItemInInventory("Moeda do Cais") && hasItemInInventory("Coracao de Barro") && hasItemInInventory("Casca do Mangue")) {
        removeItemFromInventoryByName("Moeda do Cais");
        removeItemFromInventoryByName("Coracao de Barro");
        removeItemFromInventoryByName("Casca do Mangue");

        npc->dialogueTree = &questNpcCompleteDialogue;
        startDialogue(npc->dialogueTree);

        for (int i = 0; i < PARTY_SIZE; i++) {
            playerLevelUp(&party[i]);
        }

        return 1;
    }

    return 0;
}

static void rebuildWorldMCBorderBlockers(void) {
    float mapWidth = (float)mapTexture.width;
    float mapHeight = (float)mapTexture.height;

    if (mapWidth <= 0.0f || mapHeight <= 0.0f) {

        WORLD_MC_EDGE_BLOCKER_COUNT = 0;
        return;
    }

    WORLD_MC_EDGE_BLOCKERS[0] = (Rectangle){ -WORLD_MC_BORDER_THICKNESS, 0.0f, WORLD_MC_BORDER_THICKNESS, mapHeight };
    WORLD_MC_EDGE_BLOCKERS[1] = (Rectangle){ mapWidth, 0.0f, WORLD_MC_BORDER_THICKNESS, mapHeight };
    WORLD_MC_EDGE_BLOCKERS[2] = (Rectangle){ 0.0f, -WORLD_MC_BORDER_THICKNESS, mapWidth, WORLD_MC_BORDER_THICKNESS };
    WORLD_MC_EDGE_BLOCKERS[3] = (Rectangle){ 0.0f, mapHeight, mapWidth, WORLD_MC_BORDER_THICKNESS };
    WORLD_MC_EDGE_BLOCKERS[4] = WORLD_MC_HORIZONTAL_BLOCKER;
    WORLD_MC_EDGE_BLOCKER_COUNT = 5;
}

static void collectWorldMCBlockers(Rectangle* outBlockers, int* outCount) {
    if (outCount != NULL) {
        *outCount = WORLD_MC_EDGE_BLOCKER_COUNT + WORLD_MC_TREE_BLOCKER_COUNT;
    }

    if (outBlockers == NULL || (WORLD_MC_EDGE_BLOCKER_COUNT + WORLD_MC_TREE_BLOCKER_COUNT) <= 0) {
        return;
    }

    for (int i = 0; i < WORLD_MC_EDGE_BLOCKER_COUNT; i++) {
        outBlockers[i] = WORLD_MC_EDGE_BLOCKERS[i];
    }

    for (int i = 0; i < WORLD_MC_TREE_BLOCKER_COUNT; i++) {
        outBlockers[WORLD_MC_EDGE_BLOCKER_COUNT + i] = WORLD_MC_TREE_BLOCKERS[i];
    }
}

static void rebuildWorldMCTreeBlockers(void) {
    if (WORLD_MC_TREE_TEXTURE.id == 0 || WORLD_MC_TREE_TEXTURE.width <= 0 || WORLD_MC_TREE_TEXTURE.height <= 0) {
        WORLD_MC_TREE_BLOCKER_COUNT = 0;
        return;
    }

    WORLD_MC_TREE_BLOCKERS[0] = getWorldMCTreeRect(WORLD_MC_TREE_POSITIONS_NORMAL[0]);
    WORLD_MC_TREE_BLOCKERS[1] = getWorldMCTreeRect(WORLD_MC_TREE_POSITIONS_NORMAL[1]);
    WORLD_MC_TREE_BLOCKERS[2] = getWorldMCTreeRect(WORLD_MC_TREE_POSITIONS_RD[0]);
    WORLD_MC_TREE_BLOCKERS[3] = getWorldMCTreeRect(WORLD_MC_TREE_POSITIONS_RD[1]);
    WORLD_MC_TREE_BLOCKER_COUNT = 4;
}

static void drawWorldMCOverlay(void) {
    for (int i = 0; i < (int)(sizeof(WORLD_MC_TREE_POSITIONS_NORMAL) / sizeof(WORLD_MC_TREE_POSITIONS_NORMAL[0])); i++) {
        drawTreeAtBaseCenter(WORLD_MC_TREE_POSITIONS_NORMAL[i]);
    }

    for (int i = 0; i < (int)(sizeof(WORLD_MC_TREE_POSITIONS_RD) / sizeof(WORLD_MC_TREE_POSITIONS_RD[0])); i++) {
        drawTreeAtBaseCenter(WORLD_MC_TREE_POSITIONS_RD[i]);
    }

    drawStatueAtBaseCenter(WORLD_MC_STATUE_POSITION);
    drawTriggerRect(WORLD_MC_NEXT_WORLD_TRIGGER, ORANGE);

    for (int i = 0; i < WORLD_MC_EDGE_BLOCKER_COUNT; i++) {
        DrawRectangleLinesEx(WORLD_MC_EDGE_BLOCKERS[i], 2.0f, GREEN);
    }

    for (int i = 0; i < WORLD_MC_TREE_BLOCKER_COUNT; i++) {
        DrawRectangleLinesEx(WORLD_MC_TREE_BLOCKERS[i], 2.0f, GREEN);
    }

    Rectangle pickArea;
    Rectangle treeRect = getWorldMCTreeRect(WORLD_MC_TREE_POSITIONS_RD[1]);
    if (treeRect.width > 0 && treeRect.height > 0) {
        pickArea = (Rectangle){ treeRect.x - 24.0f, treeRect.y, treeRect.width + 48.0f, treeRect.height };
    } else {
        Vector2 base = WORLD_MC_TREE_POSITIONS_RD[1];
        pickArea = (Rectangle){ base.x - 36.0f, base.y - 120.0f, 72.0f, 120.0f };
    }
    DrawRectangleLinesEx(pickArea, 2.0f, YELLOW);
    DrawText("Casca do Mangue", (int)pickArea.x, (int)(pickArea.y - 14.0f), 10, WHITE);
}

static void getWorldMCBounds(Rectangle* outBounds) {
    if (outBounds == NULL) {
        return;
    }

    *outBounds = (Rectangle){0.0f, 0.0f, (float)mapTexture.width, (float)mapTexture.height};
}

static int processWorldMCTriggers(Vector2 playerPos) {
    Collider playerCollider = {
        .offset = {75.0f, 0.0f},
        .size = {150.0f, 200.0f}
    };

    Rectangle playerRect = getColliderRect(playerPos, playerCollider);

    if (IsKeyPressed(KEY_Z)) {
        Rectangle treeRect = getWorldMCTreeRect(WORLD_MC_TREE_POSITIONS_RD[1]);
        Rectangle pickArea;
        if (treeRect.width > 0 && treeRect.height > 0) {
            pickArea = (Rectangle){ treeRect.x, treeRect.y + treeRect.height + 6.0f, treeRect.width, 40.0f };
        } else {
            Vector2 base = WORLD_MC_TREE_POSITIONS_RD[1];
            pickArea = (Rectangle){ base.x - 36.0f, base.y + 6.0f, 72.0f, 40.0f };
        }
        if (CheckCollisionRecs(playerRect, pickArea)) {
            ListNode* cur = playerInventory.items.head;
            int found = 0;
            for (int i = 0; i < playerInventory.items.size && cur != NULL; i++) {
                InventoryItem* it = (InventoryItem*)cur->data;
                if (it != NULL && strcmp(it->baseItem->name, "Casca do Mangue") == 0) { found = 1; break; }
                cur = cur->next;
            }
            if (!found) {
                InventoryItem* newItem = malloc(sizeof(InventoryItem));
                if (newItem != NULL) {
                    newItem->baseItem = &cascaDoMangue;
                    newItem->quantity = 1;
                    addItemInventory(&playerInventory, newItem);
                    bossAiQueueMessage("Voce encontrou: Casca do Mangue");
                }
            }
        }
    }

    return processWorldTransitionZones(
        playerRect,
        WORLD_MC_TRANSITIONS,
        (int)(sizeof(WORLD_MC_TRANSITIONS) / sizeof(WORLD_MC_TRANSITIONS[0]))
    );
}

static void setupWorldMCNode(void* userData) {
    (void)userData;

    initParty(0, getWorldSpawnPosition(WORLD_MC_PARTY_SPAWN));

    initNPC(&worldMCQuestNpc, (Vector2){1285.0f, 445.0f}, "assets/NPCs/npc_placeholder.png", &questNpcIntroDialogue);
    worldMCQuestNpc.preInteract = worldMCQuestPreInteract;

    if (mapTexture.id != 0) {
        UnloadTexture(mapTexture);
    }

    mapTexture = LoadTexture(WORLD_MC_MAP_PATH);

    if (WORLD_MC_TREE_TEXTURE.id != 0) {
        UnloadTexture(WORLD_MC_TREE_TEXTURE);
    }

    WORLD_MC_TREE_TEXTURE = LoadTexture(WORLD_MC_TREE_PATH);

    if (WORLD_MC_STATUE_TEXTURE.id != 0) {
        UnloadTexture(WORLD_MC_STATUE_TEXTURE);
    }

    WORLD_MC_STATUE_TEXTURE = LoadTexture("assets/cenarios/ESTATUA_MZERO.png");
    rebuildWorldMCBorderBlockers();
    rebuildWorldMCTreeBlockers();
    configureCameraForCurrentWorld();
}

static void teardownWorldMCNode(void) {
    if (mapTexture.id != 0) {
        UnloadTexture(mapTexture);
        mapTexture.id = 0;
    }

    if (WORLD_MC_TREE_TEXTURE.id != 0) {
        UnloadTexture(WORLD_MC_TREE_TEXTURE);
        WORLD_MC_TREE_TEXTURE = (Texture2D){0};
    }

    if (WORLD_MC_STATUE_TEXTURE.id != 0) {
        UnloadTexture(WORLD_MC_STATUE_TEXTURE);
        WORLD_MC_STATUE_TEXTURE = (Texture2D){0};
    }

    WORLD_MC_EDGE_BLOCKER_COUNT = 0;
    WORLD_MC_TREE_BLOCKER_COUNT = 0;

    unloadNPC(&worldMCQuestNpc);
    unloadParty();
}

void RegisterWorldMC(void) {
    static WorldNode worldMCNode = {
        .name = WORLD_NAME_MC,
        .setup = setupWorldMCNode,
        .teardown = teardownWorldMCNode,
        .collectBlockers = collectWorldMCBlockers,
        .drawOverlay = drawWorldMCOverlay,
        .processTriggers = processWorldMCTriggers,
        .getCameraBounds = getWorldMCBounds,
        .userData = NULL,
        .prev = NULL,
        .next = NULL
    };

    registerWorldNode(&worldMCNode);
}
