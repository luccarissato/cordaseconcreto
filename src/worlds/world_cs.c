#include "world_cs.h"

#include "worlds.h"
#include "../core/game.h"
#include "../core/collision.h"
#include "raylib.h"
#include <stddef.h>

static const Vector2 WORLD_CS_PARTY_SPAWN = {120.0f, 820.0f};
static const Vector2 WORLD_MC_LEFT_EDGE_SPAWN = {120.0f, 680.0f};
static const char* WORLD_CS_MAP_PATH = "assets/cenarios/CS_FRENTE.png";
static const float WORLD_CS_BORDER_THICKNESS = 128.0f;

static const Rectangle WORLD_CS_PREVIOUS_WORLD_TRIGGER = {1835.0f, 800.0f, 160.0f, 270.0f};
static const WorldTransitionZone WORLD_CS_TRANSITIONS[] = {
    {WORLD_CS_PREVIOUS_WORLD_TRIGGER, WORLD_NAME_MC, WORLD_MC_LEFT_EDGE_SPAWN}
};

static Rectangle WORLD_CS_EDGE_BLOCKERS[4];
static int WORLD_CS_EDGE_BLOCKER_COUNT = 0;

static void rebuildWorldCSBorderBlockers(void) {
    float mapWidth = (float)mapTexture.width;
    float mapHeight = (float)mapTexture.height;

    if (mapWidth <= 0.0f || mapHeight <= 0.0f) {
        WORLD_CS_EDGE_BLOCKER_COUNT = 0;
        return;
    }

    WORLD_CS_EDGE_BLOCKERS[0] = (Rectangle){-WORLD_CS_BORDER_THICKNESS, 0.0f, WORLD_CS_BORDER_THICKNESS, mapHeight};
    WORLD_CS_EDGE_BLOCKERS[1] = (Rectangle){mapWidth, 0.0f, WORLD_CS_BORDER_THICKNESS, mapHeight};
    WORLD_CS_EDGE_BLOCKERS[2] = (Rectangle){0.0f, -WORLD_CS_BORDER_THICKNESS, mapWidth, WORLD_CS_BORDER_THICKNESS};
    WORLD_CS_EDGE_BLOCKERS[3] = (Rectangle){0.0f, mapHeight, mapWidth, WORLD_CS_BORDER_THICKNESS};
    WORLD_CS_EDGE_BLOCKER_COUNT = 4;
}

static void collectWorldCSBlockers(Rectangle* outBlockers, int* outCount) {
    if (outCount != NULL) {
        *outCount = WORLD_CS_EDGE_BLOCKER_COUNT;
    }

    if (outBlockers == NULL || WORLD_CS_EDGE_BLOCKER_COUNT <= 0) {
        return;
    }

    for (int i = 0; i < WORLD_CS_EDGE_BLOCKER_COUNT; i++) {
        outBlockers[i] = WORLD_CS_EDGE_BLOCKERS[i];
    }
}

static void drawWorldCSOverlay(void) {
    DrawRectangleLinesEx(WORLD_CS_PREVIOUS_WORLD_TRIGGER, 2.0f, ORANGE);

    for (int i = 0; i < WORLD_CS_EDGE_BLOCKER_COUNT; i++) {
        DrawRectangleLinesEx(WORLD_CS_EDGE_BLOCKERS[i], 2.0f, GREEN);
    }
}

static void getWorldCSBounds(Rectangle* outBounds) {
    if (outBounds == NULL) {
        return;
    }

    *outBounds = (Rectangle){0.0f, 0.0f, (float)mapTexture.width, (float)mapTexture.height};
}

static int processWorldCSTriggers(Vector2 playerPos) {
    Collider playerCollider = {
        .offset = {75.0f, 0.0f},
        .size = {150.0f, 200.0f}
    };

    return processWorldTransitionZones(
        getColliderRect(playerPos, playerCollider),
        WORLD_CS_TRANSITIONS,
        (int)(sizeof(WORLD_CS_TRANSITIONS) / sizeof(WORLD_CS_TRANSITIONS[0]))
    );
}

static void setupWorldCSNode(void* userData) {
    (void)userData;

    initParty(1, getWorldSpawnPosition(WORLD_CS_PARTY_SPAWN));

    if (mapTexture.id != 0) {
        UnloadTexture(mapTexture);
    }

    mapTexture = LoadTexture(WORLD_CS_MAP_PATH);
    rebuildWorldCSBorderBlockers();
    configureCameraForCurrentWorld();
}

static void teardownWorldCSNode(void) {
    if (mapTexture.id != 0) {
        UnloadTexture(mapTexture);
        mapTexture.id = 0;
    }

    WORLD_CS_EDGE_BLOCKER_COUNT = 0;
    unloadParty();
}

void RegisterWorldCS(void) {
    static WorldNode worldCSNode = {
        .name = WORLD_NAME_CS,
        .setup = setupWorldCSNode,
        .teardown = teardownWorldCSNode,
        .collectBlockers = collectWorldCSBlockers,
        .drawOverlay = drawWorldCSOverlay,
        .processTriggers = processWorldCSTriggers,
        .getCameraBounds = getWorldCSBounds,
        .userData = NULL,
        .prev = NULL,
        .next = NULL
    };

    registerWorldNode(&worldCSNode);
}
