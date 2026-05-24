#include "world_pc.h"

#include "worlds.h"
#include "../core/game.h"
#include "../core/collision.h"
#include "raylib.h"
#include <stddef.h>

static const Vector2 WORLD_PC_PARTY_SPAWN = {900.0f, 560.0f};
static const Vector2 WORLD_CS_RETURN_SPAWN = {860.0f, 850.0f};
static const char* WORLD_PC_MAP_PATH = "assets/cenarios/PC_ARSENAL_ATUALIZADO.png";
static const float WORLD_PC_BORDER_THICKNESS = 128.0f;

/* Faixa horizontal interpretada como um retangulo fino em y=240. */
static const Rectangle WORLD_PC_PREVIOUS_WORLD_TRIGGER = {780.0f, 240.0f, 250.0f, 12.0f};
static const WorldTransitionZone WORLD_PC_TRANSITIONS[] = {
    {WORLD_PC_PREVIOUS_WORLD_TRIGGER, WORLD_TRANSITION_PREVIOUS, WORLD_CS_RETURN_SPAWN}
};

static Rectangle WORLD_PC_EDGE_BLOCKERS[4];
static int WORLD_PC_EDGE_BLOCKER_COUNT = 0;

static void drawTriggerRect(Rectangle rect, Color color) {
    DrawRectangleLinesEx(rect, 2.0f, color);
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
        *outCount = WORLD_PC_EDGE_BLOCKER_COUNT;
    }

    if (outBlockers == NULL || WORLD_PC_EDGE_BLOCKER_COUNT <= 0) {
        return;
    }

    for (int i = 0; i < WORLD_PC_EDGE_BLOCKER_COUNT; i++) {
        outBlockers[i] = WORLD_PC_EDGE_BLOCKERS[i];
    }
}

static void drawWorldPCOverlay(void) {
    drawTriggerRect(WORLD_PC_PREVIOUS_WORLD_TRIGGER, ORANGE);

    for (int i = 0; i < WORLD_PC_EDGE_BLOCKER_COUNT; i++) {
        DrawRectangleLinesEx(WORLD_PC_EDGE_BLOCKERS[i], 2.0f, GREEN);
    }
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
    rebuildWorldPCBorderBlockers();
    configureCameraForCurrentWorld();
}

static void teardownWorldPCNode(void) {
    if (mapTexture.id != 0) {
        UnloadTexture(mapTexture);
        mapTexture.id = 0;
    }

    WORLD_PC_EDGE_BLOCKER_COUNT = 0;

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