#include "world_pc.h"

#include "worlds.h"
#include "../core/game.h"
#include "../core/collision.h"
#include "../entities/npc.h"
#include "../data/dialogues/npc_dialogues.h"
#include "raylib.h"
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include "../items/inventory.h"
#include "../items/game_items.h"
#include "../combat/boss_ai.h"

static const Vector2 WORLD_PC_PARTY_SPAWN = {900.0f, 560.0f};
static const Vector2 WORLD_CS_RETURN_SPAWN = {860.0f, 850.0f};
static const char* WORLD_PC_MAP_PATH = "assets/cenarios/PC_ARSENAL_ATUALIZADO.png";
static const char* WORLD_PC_CAR_PATH = "assets/cenarios/CARRO_PCARSENAL.png";
static const float WORLD_PC_BORDER_THICKNESS = 128.0f;
static const Vector2 WORLD_PC_CAR_BASE_CENTER = {480.0f, 800.0f};

NPC worldPCNpc1;
NPC worldPCNpc2;

static const Rectangle WORLD_PC_PREVIOUS_WORLD_TRIGGER = {780.0f, 200.0f, 250.0f, 12.0f};
static const Rectangle WORLD_PC_NEXT_WORLD_TRIGGER = {1370.0f, 200.0f, 180.0f, 12.0f};
static const WorldTransitionZone WORLD_PC_TRANSITIONS[] = {
    {WORLD_PC_PREVIOUS_WORLD_TRIGGER, WORLD_TRANSITION_PREVIOUS, WORLD_CS_RETURN_SPAWN},
    {WORLD_PC_NEXT_WORLD_TRIGGER, WORLD_TRANSITION_NEXT, {30.0f, 380.0f}}
};

static Rectangle WORLD_PC_EDGE_BLOCKERS[4];
static int WORLD_PC_EDGE_BLOCKER_COUNT = 0;
static Texture2D WORLD_PC_CAR_TEXTURE = {0};
static Rectangle WORLD_PC_CAR_BLOCKER = {0.0f, 0.0f, 0.0f, 0.0f};
static Rectangle WORLD_PC_CUSTOM_BLOCKERS[3];
static int WORLD_PC_CUSTOM_BLOCKER_COUNT = 0;

static void drawTriggerRect(Rectangle rect, Color color) {
    DrawRectangleLinesEx(rect, 2.0f, color);
}

static Rectangle getWorldPCCoinPickupArea(void) {
    if (WORLD_PC_CAR_BLOCKER.width > 0.0f && WORLD_PC_CAR_BLOCKER.height > 0.0f) {
        return (Rectangle){
            WORLD_PC_CAR_BLOCKER.x + (WORLD_PC_CAR_BLOCKER.width * 0.25f),
            WORLD_PC_CAR_BLOCKER.y + WORLD_PC_CAR_BLOCKER.height + 6.0f,
            WORLD_PC_CAR_BLOCKER.width * 0.5f,
            48.0f
        };
    }

    return (Rectangle){
        WORLD_PC_CAR_BASE_CENTER.x - 140.0f,
        WORLD_PC_CAR_BASE_CENTER.y + 6.0f,
        280.0f,
        48.0f
    };
}

static void drawWorldPCCar(void) {
    if (WORLD_PC_CAR_TEXTURE.id == 0) {
        return;
    }

    Vector2 drawPosition = {
        WORLD_PC_CAR_BASE_CENTER.x - ((float)WORLD_PC_CAR_TEXTURE.width * 0.5f),
        WORLD_PC_CAR_BASE_CENTER.y - (float)WORLD_PC_CAR_TEXTURE.height
    };

    DrawTextureV(WORLD_PC_CAR_TEXTURE, drawPosition, WHITE);
}

static void rebuildWorldPCBorderBlockers(void) {
    float mapWidth = (float)mapTexture.width;
    float mapHeight = (float)mapTexture.height;

    if (mapWidth <= 0.0f || mapHeight <= 0.0f) {
        WORLD_PC_EDGE_BLOCKER_COUNT = 0;
        return;
    }

    WORLD_PC_EDGE_BLOCKERS[0] = (Rectangle){-WORLD_PC_BORDER_THICKNESS, 0.0f, WORLD_PC_BORDER_THICKNESS, mapHeight};
    WORLD_PC_EDGE_BLOCKERS[1] = (Rectangle){mapWidth, 0.0f, WORLD_PC_BORDER_THICKNESS, mapHeight};
    WORLD_PC_EDGE_BLOCKERS[2] = (Rectangle){0.0f, -WORLD_PC_BORDER_THICKNESS, mapWidth, WORLD_PC_BORDER_THICKNESS};
    WORLD_PC_EDGE_BLOCKERS[3] = (Rectangle){0.0f, mapHeight, mapWidth, WORLD_PC_BORDER_THICKNESS};
    WORLD_PC_EDGE_BLOCKER_COUNT = 4;
}

static void collectWorldPCBlockers(Rectangle* outBlockers, int* outCount) {
    if (outCount != NULL) {
        *outCount = WORLD_PC_EDGE_BLOCKER_COUNT + WORLD_PC_CUSTOM_BLOCKER_COUNT + ((WORLD_PC_CAR_BLOCKER.width > 0.0f && WORLD_PC_CAR_BLOCKER.height > 0.0f) ? 1 : 0);
    }

    if (outBlockers == NULL) {
        return;
    }

    int idx = 0;
    for (int i = 0; i < WORLD_PC_EDGE_BLOCKER_COUNT; i++) {
        outBlockers[idx++] = WORLD_PC_EDGE_BLOCKERS[i];
    }

    for (int i = 0; i < WORLD_PC_CUSTOM_BLOCKER_COUNT; i++) {
        outBlockers[idx++] = WORLD_PC_CUSTOM_BLOCKERS[i];
    }

    if (WORLD_PC_CAR_BLOCKER.width > 0.0f && WORLD_PC_CAR_BLOCKER.height > 0.0f) {
        outBlockers[idx++] = WORLD_PC_CAR_BLOCKER;
    }
}

static void drawWorldPCOverlay(void) {
    drawWorldPCCar();

    drawTriggerRect(WORLD_PC_PREVIOUS_WORLD_TRIGGER, ORANGE);
    drawTriggerRect(WORLD_PC_NEXT_WORLD_TRIGGER, ORANGE);

    for (int i = 0; i < WORLD_PC_EDGE_BLOCKER_COUNT; i++) {
        DrawRectangleLinesEx(WORLD_PC_EDGE_BLOCKERS[i], 2.0f, GREEN);
    }

    for (int i = 0; i < WORLD_PC_CUSTOM_BLOCKER_COUNT; i++) {
        DrawRectangleLinesEx(WORLD_PC_CUSTOM_BLOCKERS[i], 2.0f, RED);
    }

    if (WORLD_PC_CAR_BLOCKER.width > 0.0f && WORLD_PC_CAR_BLOCKER.height > 0.0f) {
        DrawRectangleLinesEx(WORLD_PC_CAR_BLOCKER, 2.0f, RED);
    }

    Rectangle coinArea = getWorldPCCoinPickupArea();
    DrawRectangleLinesEx(coinArea, 2.0f, YELLOW);
    DrawText("Moeda do Cais", (int)coinArea.x, (int)(coinArea.y - 14.0f), 10, WHITE);
}

static void getWorldPCBounds(Rectangle* outBounds) {
    if (outBounds == NULL) {
        return;
    }

    *outBounds = (Rectangle){0.0f, 0.0f, (float)mapTexture.width, (float)mapTexture.height};
}

static int processWorldPCTriggers(Vector2 playerPos) {
    Collider playerCollider = {
        .offset = {75.0f, 0.0f},
        .size = {150.0f, 200.0f}
    };

    extern Inventory playerInventory;

    Rectangle playerRect = getColliderRect(playerPos, playerCollider);
    if (IsKeyPressed(KEY_Z)) {
        Rectangle coinArea = getWorldPCCoinPickupArea();
        if (CheckCollisionRecs(playerRect, coinArea)) {
            ListNode* cur = playerInventory.items.head;
            int found = 0;
            for (int i = 0; i < playerInventory.items.size && cur != NULL; i++) {
                InventoryItem* it = (InventoryItem*)cur->data;
                if (it != NULL && strcmp(it->baseItem->name, "Moeda do Cais") == 0) { found = 1; break; }
                cur = cur->next;
            }
            if (!found) {
                InventoryItem* newItem = malloc(sizeof(InventoryItem));
                if (newItem != NULL) {
                    newItem->baseItem = &moedaDoCais;
                    newItem->quantity = 1;
                    addItemInventory(&playerInventory, newItem);
                    bossAiQueueMessage("Voce encontrou: Moeda do Cais");
                }
            }
        }
    }

    playerRect = getColliderRect(playerPos, playerCollider);
    if (CheckCollisionRecs(playerRect, WORLD_PC_PREVIOUS_WORLD_TRIGGER)) {
        requestWorldTransitionToName(WORLD_NAME_CS, WORLD_CS_RETURN_SPAWN);
        return 1;
    }

    if (CheckCollisionRecs(playerRect, WORLD_PC_NEXT_WORLD_TRIGGER)) {
        requestWorldTransitionNext((Vector2){30.0f, 380.0f});
        return 1;
    }

    return 0;
}

static void setupWorldPCNode(void* userData) {
    (void)userData;

    initParty(0, getWorldSpawnPosition(WORLD_PC_PARTY_SPAWN));
    initNPC(&worldPCNpc1, (Vector2){500.0f, 190.0f}, "assets/NPCs/NPC1.png", &npc2Dialogue, "Giovanna");
    initNPC(&worldPCNpc2, (Vector2){1750.0f, 190.0f}, "assets/NPCs/NPC3.png", &npc3Dialogue, "Igor");

    if (mapTexture.id != 0) {
        UnloadTexture(mapTexture);
    }

    mapTexture = LoadTexture(WORLD_PC_MAP_PATH);
    if (WORLD_PC_CAR_TEXTURE.id != 0) {
        UnloadTexture(WORLD_PC_CAR_TEXTURE);
    }

    WORLD_PC_CAR_TEXTURE = LoadTexture(WORLD_PC_CAR_PATH);
    if (WORLD_PC_CAR_TEXTURE.id != 0 && WORLD_PC_CAR_TEXTURE.width > 0 && WORLD_PC_CAR_TEXTURE.height > 0) {
        WORLD_PC_CAR_BLOCKER = (Rectangle){
            WORLD_PC_CAR_BASE_CENTER.x - ((float)WORLD_PC_CAR_TEXTURE.width * 0.5f),
            WORLD_PC_CAR_BASE_CENTER.y - (float)WORLD_PC_CAR_TEXTURE.height,
            (float)WORLD_PC_CAR_TEXTURE.width,
            (float)WORLD_PC_CAR_TEXTURE.height * 0.92f
        };
    } else {
        WORLD_PC_CAR_BLOCKER = (Rectangle){0.0f, 0.0f, 0.0f, 0.0f};
    }

    WORLD_PC_CUSTOM_BLOCKERS[0] = (Rectangle){0.0f, 220.0f, 776.0f - 0.0f, 12.0f};
    WORLD_PC_CUSTOM_BLOCKERS[1] = (Rectangle){1035.0f, 220.0f, 1360.0f - 1035.0f, 12.0f};
    WORLD_PC_CUSTOM_BLOCKERS[2] = (Rectangle){1560.0f, 220.0f, 1870.0f - 1560.0f, 12.0f};
    WORLD_PC_CUSTOM_BLOCKER_COUNT = 3;

    rebuildWorldPCBorderBlockers();
    configureCameraForCurrentWorld();
}

static void teardownWorldPCNode(void) {
    if (mapTexture.id != 0) {
        UnloadTexture(mapTexture);
        mapTexture.id = 0;
    }

    if (WORLD_PC_CAR_TEXTURE.id != 0) {
        UnloadTexture(WORLD_PC_CAR_TEXTURE);
        WORLD_PC_CAR_TEXTURE = (Texture2D){0};
    }

    unloadNPC(&worldPCNpc1);
    unloadNPC(&worldPCNpc2);

    WORLD_PC_EDGE_BLOCKER_COUNT = 0;
    WORLD_PC_CAR_BLOCKER = (Rectangle){0.0f, 0.0f, 0.0f, 0.0f};
    WORLD_PC_CUSTOM_BLOCKER_COUNT = 0;

    unloadParty();
}

void RegisterWorldPC(void) {
    static WorldNode worldPCNode = {
        .name = WORLD_NAME_PC,
        .setup = setupWorldPCNode,
        .teardown = teardownWorldPCNode,
        .collectBlockers = collectWorldPCBlockers,
        .drawOverlay = drawWorldPCOverlay,
        .processTriggers = processWorldPCTriggers,
        .getCameraBounds = getWorldPCBounds,
        .userData = NULL,
        .prev = NULL,
        .next = NULL
    };

    registerWorldNode(&worldPCNode);
}
