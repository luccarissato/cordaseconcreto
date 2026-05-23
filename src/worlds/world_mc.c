#include "world_mc.h"

#include "worlds.h"
#include "../core/game.h"
#include "raylib.h"
#include <stddef.h>

static const Vector2 WORLD_MC_PARTY_SPAWN = {1680.0f, 680.0f};
static const char* WORLD_MC_MAP_PATH = "assets/cenarios/MZERO_ATUALIZADO.png";
static const float WORLD_MC_BORDER_THICKNESS = 128.0f;

static const Rectangle WORLD_MC_HORIZONTAL_BLOCKER = {0.0f, 240.0f, 1920.0f, 12.0f};

static Rectangle WORLD_MC_EDGE_BLOCKERS[5];
static int WORLD_MC_EDGE_BLOCKER_COUNT = 0;

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
        *outCount = WORLD_MC_EDGE_BLOCKER_COUNT;
    }

    if (outBlockers == NULL || WORLD_MC_EDGE_BLOCKER_COUNT <= 0) {
        return;
    }

    for (int i = 0; i < WORLD_MC_EDGE_BLOCKER_COUNT; i++) {
        outBlockers[i] = WORLD_MC_EDGE_BLOCKERS[i];
    }
}

static void drawWorldMCOverlay(void) {
    for (int i = 0; i < WORLD_MC_EDGE_BLOCKER_COUNT; i++) {
        DrawRectangleLinesEx(WORLD_MC_EDGE_BLOCKERS[i], 2.0f, GREEN);
    }
}

static void setupWorldMCNode(void* userData) {
    (void)userData;

    initParty(1, WORLD_MC_PARTY_SPAWN);

    if (mapTexture.id != 0) {
        UnloadTexture(mapTexture);
    }

    mapTexture = LoadTexture(WORLD_MC_MAP_PATH);
    rebuildWorldMCBorderBlockers();
    camera.target = party[0].position;
    camera.offset = (Vector2){ (float)GetScreenWidth() * 0.5f, (float)GetScreenHeight() * 0.5f };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;
}

static void teardownWorldMCNode(void) {
    if (mapTexture.id != 0) {
        UnloadTexture(mapTexture);
        mapTexture.id = 0;
    }

    WORLD_MC_EDGE_BLOCKER_COUNT = 0;

    unloadParty();
}

void RegisterWorldMC(void) {
    static WorldNode worldMCNode = {
        .name = "MZERO_ATUALIZADO",
        .setup = setupWorldMCNode,
        .teardown = teardownWorldMCNode,
        .collectBlockers = collectWorldMCBlockers,
        .drawOverlay = drawWorldMCOverlay,
        .userData = NULL,
        .prev = NULL,
        .next = NULL
    };

    registerWorldNode(&worldMCNode);
}