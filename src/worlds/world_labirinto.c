#include "world_labirinto.h"

#include "worlds.h"
#include "../core/game.h"
#include "../core/collision.h"
#include "../entities/player.h"
#include "raylib.h"
#include <stddef.h>

static const Vector2 WORLD_CS_LABIRINTO_RETURN_SPAWN = {880.0f, 410.0f};
static const char* WORLD_LABIRINTO_MAP_PATH = "assets/cenarios/PUZZLE_CS.png";
static const float WORLD_LABIRINTO_PLAYER_SCALE = 35.0f / 184.0f;
static const float WORLD_LABIRINTO_BORDER_THICKNESS = 64.0f;

static const Rectangle WORLD_LABIRINTO_RETURN_TRIGGER = {915.0f, 60.0f, 80.0f, 12.0f};

static Rectangle WORLD_LABIRINTO_EDGE_BLOCKERS[4];
static int WORLD_LABIRINTO_EDGE_BLOCKER_COUNT = 0;

static void rebuildWorldLabirintoBorderBlockers(void) {
    float mapWidth = (float)mapTexture.width;
    float mapHeight = (float)mapTexture.height;

    if (mapWidth <= 0.0f || mapHeight <= 0.0f) {
        WORLD_LABIRINTO_EDGE_BLOCKER_COUNT = 0;
        return;
    }

    WORLD_LABIRINTO_EDGE_BLOCKERS[0] = (Rectangle){-WORLD_LABIRINTO_BORDER_THICKNESS, 0.0f, WORLD_LABIRINTO_BORDER_THICKNESS, mapHeight};
    WORLD_LABIRINTO_EDGE_BLOCKERS[1] = (Rectangle){mapWidth, 0.0f, WORLD_LABIRINTO_BORDER_THICKNESS, mapHeight};
    WORLD_LABIRINTO_EDGE_BLOCKERS[2] = (Rectangle){0.0f, -WORLD_LABIRINTO_BORDER_THICKNESS, mapWidth, WORLD_LABIRINTO_BORDER_THICKNESS};
    WORLD_LABIRINTO_EDGE_BLOCKERS[3] = (Rectangle){0.0f, mapHeight, mapWidth, WORLD_LABIRINTO_BORDER_THICKNESS};
    WORLD_LABIRINTO_EDGE_BLOCKER_COUNT = 4;
}

static void collectWorldLabirintoBlockers(Rectangle* outBlockers, int* outCount) {
    if (outCount != NULL) {
        *outCount = WORLD_LABIRINTO_EDGE_BLOCKER_COUNT;
    }

    if (outBlockers == NULL) {
        return;
    }

    for (int i = 0; i < WORLD_LABIRINTO_EDGE_BLOCKER_COUNT; i++) {
        outBlockers[i] = WORLD_LABIRINTO_EDGE_BLOCKERS[i];
    }
}

static void drawWorldLabirintoOverlay(void) {
    DrawRectangleLinesEx(WORLD_LABIRINTO_RETURN_TRIGGER, 2.0f, ORANGE);

    for (int i = 0; i < WORLD_LABIRINTO_EDGE_BLOCKER_COUNT; i++) {
        DrawRectangleLinesEx(WORLD_LABIRINTO_EDGE_BLOCKERS[i], 2.0f, GREEN);
    }
}

static void getWorldLabirintoBounds(Rectangle* outBounds) {
    if (outBounds == NULL) {
        return;
    }

    *outBounds = (Rectangle){0.0f, 0.0f, (float)mapTexture.width, (float)mapTexture.height};
}

static int processWorldLabirintoTriggers(Vector2 playerPos) {
    Collider playerCollider = {
        .offset = {50.0f * WORLD_LABIRINTO_PLAYER_SCALE, 0.0f},
        .size = {184.0f * WORLD_LABIRINTO_PLAYER_SCALE, 184.0f * WORLD_LABIRINTO_PLAYER_SCALE}
    };

    Rectangle playerRect = getColliderRect(playerPos, playerCollider);
    if (CheckCollisionRecs(playerRect, WORLD_LABIRINTO_RETURN_TRIGGER)) {
        requestWorldTransitionToName(WORLD_NAME_CS, WORLD_CS_LABIRINTO_RETURN_SPAWN);
        return 1;
    }

    return 0;
}

static void setupWorldLabirintoNode(void* userData) {
    (void)userData;

    setPlayerWorldScale(WORLD_LABIRINTO_PLAYER_SCALE);
    initParty(0, getWorldSpawnPosition(WORLD_LABIRINTO_PARTY_SPAWN));

    if (mapTexture.id != 0) {
        UnloadTexture(mapTexture);
    }

    mapTexture = LoadTexture(WORLD_LABIRINTO_MAP_PATH);
    rebuildWorldLabirintoBorderBlockers();
    configureCameraForCurrentWorld();
}

static void teardownWorldLabirintoNode(void) {
    setPlayerWorldScale(1.0f);

    if (mapTexture.id != 0) {
        UnloadTexture(mapTexture);
        mapTexture.id = 0;
    }

    WORLD_LABIRINTO_EDGE_BLOCKER_COUNT = 0;
    unloadParty();
}

void RegisterWorldLabirinto(void) {
    static WorldNode worldLabirintoNode = {
        .name = WORLD_NAME_LABIRINTO,
        .setup = setupWorldLabirintoNode,
        .teardown = teardownWorldLabirintoNode,
        .collectBlockers = collectWorldLabirintoBlockers,
        .drawOverlay = drawWorldLabirintoOverlay,
        .processTriggers = processWorldLabirintoTriggers,
        .getCameraBounds = getWorldLabirintoBounds,
        .userData = NULL,
        .prev = NULL,
        .next = NULL
    };

    registerWorldNode(&worldLabirintoNode);
}
