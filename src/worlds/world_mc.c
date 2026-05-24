#include "world_mc.h"

#include "worlds.h"
#include "../core/game.h"
#include "../core/collision.h"
#include "raylib.h"
#include <stddef.h>

static const Vector2 WORLD_MC_PARTY_SPAWN = {1680.0f, 680.0f};
/* Spawn fica na borda direita do CS, mas fora do trigger de retorno para evitar troca imediata. */
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
    {WORLD_MC_NEXT_WORLD_TRIGGER, WORLD_NAME_CS, WORLD_CS_RIGHT_EDGE_SPAWN}
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

    return processWorldTransitionZones(
        getColliderRect(playerPos, playerCollider),
        WORLD_MC_TRANSITIONS,
        (int)(sizeof(WORLD_MC_TRANSITIONS) / sizeof(WORLD_MC_TRANSITIONS[0]))
    );
}

static void setupWorldMCNode(void* userData) {
    (void)userData;

    initParty(1, getWorldSpawnPosition(WORLD_MC_PARTY_SPAWN));

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
