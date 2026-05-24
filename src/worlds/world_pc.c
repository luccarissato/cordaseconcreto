#include "world_pc.h"

#include "worlds.h"
#include "../core/game.h"
#include "../core/collision.h"
#include "raylib.h"
#include <stddef.h>

static const Vector2 WORLD_PC_PARTY_SPAWN = {900.0f, 560.0f};
static const Vector2 WORLD_CS_RETURN_SPAWN = {860.0f, 850.0f};
static const char* WORLD_PC_MAP_PATH = "assets/cenarios/PC_ARSENAL_ATUALIZADO.png";
static const char* WORLD_PC_CAR_PATH = "assets/cenarios/CARRO_PCARSENAL.png";
static const float WORLD_PC_BORDER_THICKNESS = 128.0f;
static const Vector2 WORLD_PC_CAR_BASE_CENTER = {480.0f, 850.0f};

/* Faixa horizontal interpretada como um retangulo fino em y=240. */
static const Rectangle WORLD_PC_PREVIOUS_WORLD_TRIGGER = {780.0f, 190.0f, 250.0f, 12.0f};
static const WorldTransitionZone WORLD_PC_TRANSITIONS[] = {
    {WORLD_PC_PREVIOUS_WORLD_TRIGGER, WORLD_TRANSITION_PREVIOUS, WORLD_CS_RETURN_SPAWN}
};

static Rectangle WORLD_PC_EDGE_BLOCKERS[4];
static int WORLD_PC_EDGE_BLOCKER_COUNT = 0;
static Texture2D WORLD_PC_CAR_TEXTURE = {0};
static Rectangle WORLD_PC_CAR_BLOCKER = {0.0f, 0.0f, 0.0f, 0.0f};

static void drawTriggerRect(Rectangle rect, Color color) {
    DrawRectangleLinesEx(rect, 2.0f, color);
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
        *outCount = WORLD_PC_EDGE_BLOCKER_COUNT + ((WORLD_PC_CAR_BLOCKER.width > 0.0f && WORLD_PC_CAR_BLOCKER.height > 0.0f) ? 1 : 0);
    }

    if (outBlockers == NULL) {
        return;
    }

    int idx = 0;
    for (int i = 0; i < WORLD_PC_EDGE_BLOCKER_COUNT; i++) {
        outBlockers[idx++] = WORLD_PC_EDGE_BLOCKERS[i];
    }

    if (WORLD_PC_CAR_BLOCKER.width > 0.0f && WORLD_PC_CAR_BLOCKER.height > 0.0f) {
        outBlockers[idx++] = WORLD_PC_CAR_BLOCKER;
    }
}

static void drawWorldPCOverlay(void) {
    drawTriggerRect(WORLD_PC_PREVIOUS_WORLD_TRIGGER, ORANGE);

    for (int i = 0; i < WORLD_PC_EDGE_BLOCKER_COUNT; i++) {
        DrawRectangleLinesEx(WORLD_PC_EDGE_BLOCKERS[i], 2.0f, GREEN);
    }

    if (WORLD_PC_CAR_BLOCKER.width > 0.0f && WORLD_PC_CAR_BLOCKER.height > 0.0f) {
        DrawRectangleLinesEx(WORLD_PC_CAR_BLOCKER, 2.0f, RED);
    }

    drawWorldPCCar();
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

    return processWorldTransitionZones(
        getColliderRect(playerPos, playerCollider),
        WORLD_PC_TRANSITIONS,
        (int)(sizeof(WORLD_PC_TRANSITIONS) / sizeof(WORLD_PC_TRANSITIONS[0]))
    );
}

static void setupWorldPCNode(void* userData) {
    (void)userData;

    initParty(1, getWorldSpawnPosition(WORLD_PC_PARTY_SPAWN));

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

    WORLD_PC_EDGE_BLOCKER_COUNT = 0;
    WORLD_PC_CAR_BLOCKER = (Rectangle){0.0f, 0.0f, 0.0f, 0.0f};

    unloadParty();
}

void RegisterWorldPC(void) {
    static WorldNode worldPCNode = {
        .name = "PC_ARSENAL_ATUALIZADO",
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