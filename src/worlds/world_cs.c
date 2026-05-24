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

static const char* WORLD_CS_TREE_PATH = "assets/cenarios/ARVORE_CSFRENTE.png";
static const Vector2 WORLD_CS_TREE_POSITION = {1750.0f, 780.0f};
static const char* WORLD_CS_POST_PATH = "assets/cenarios/POSTE_CSFRENTE.png";
static const Vector2 WORLD_CS_POST_POSITION = {90.0f, 780.0f};

/* Faixa horizontal interpretada como um retangulo fino em y=1075. */
static const Rectangle WORLD_CS_NEXT_WORLD_TRIGGER = {740.0f, 1075.0f, 285.0f, 12.0f};
static const Rectangle WORLD_CS_PREVIOUS_WORLD_TRIGGER = {1835.0f, 800.0f, 160.0f, 270.0f};
static const WorldTransitionZone WORLD_CS_TRANSITIONS[] = {
    {WORLD_CS_NEXT_WORLD_TRIGGER, WORLD_TRANSITION_NEXT, {760.0f, 300.0f}},
    {WORLD_CS_PREVIOUS_WORLD_TRIGGER, WORLD_TRANSITION_PREVIOUS, WORLD_MC_LEFT_EDGE_SPAWN}
};

static Rectangle WORLD_CS_EDGE_BLOCKERS[4];
static int WORLD_CS_EDGE_BLOCKER_COUNT = 0;

static Rectangle WORLD_CS_TREE_BLOCKERS[1];
static int WORLD_CS_TREE_BLOCKER_COUNT = 0;
static Texture2D WORLD_CS_TREE_TEXTURE = {0};
static Rectangle WORLD_CS_POST_BLOCKERS[1];
static int WORLD_CS_POST_BLOCKER_COUNT = 0;
static Texture2D WORLD_CS_POST_TEXTURE = {0};
static Rectangle WORLD_CS_CUSTOM_BLOCKERS[3];
static int WORLD_CS_CUSTOM_BLOCKER_COUNT = 0;

static void drawWorldCSTreeAtBaseCenter(Vector2 baseCenter) {
    if (WORLD_CS_TREE_TEXTURE.id == 0) {
        return;
    }

    Vector2 drawPosition = {
        baseCenter.x - ((float)WORLD_CS_TREE_TEXTURE.width * 0.5f),
        baseCenter.y - (float)WORLD_CS_TREE_TEXTURE.height
    };

    DrawTextureV(WORLD_CS_TREE_TEXTURE, drawPosition, WHITE);
}

static Rectangle getWorldCSTreeRect(Vector2 baseCenter) {
    if (WORLD_CS_TREE_TEXTURE.id == 0) {
        return (Rectangle){0.0f, 0.0f, 0.0f, 0.0f};
    }

    return (Rectangle){
        baseCenter.x - ((float)WORLD_CS_TREE_TEXTURE.width * 0.325f),
        baseCenter.y - (float)WORLD_CS_TREE_TEXTURE.height,
        (float)WORLD_CS_TREE_TEXTURE.width * 0.65f,
        (float)WORLD_CS_TREE_TEXTURE.height * 0.9f
    };
}

static void rebuildWorldCSTreeBlockers(void) {
    if (WORLD_CS_TREE_TEXTURE.id == 0 || WORLD_CS_TREE_TEXTURE.width <= 0 || WORLD_CS_TREE_TEXTURE.height <= 0) {
        WORLD_CS_TREE_BLOCKER_COUNT = 0;
        return;
    }

    WORLD_CS_TREE_BLOCKERS[0] = getWorldCSTreeRect(WORLD_CS_TREE_POSITION);
    WORLD_CS_TREE_BLOCKER_COUNT = 1;
}

static void rebuildWorldCSCustomBlockers(void) {
    /* Horizontal blocker from 1145 x 500 to 1915 x 500 (thin horizontal band) */
    WORLD_CS_CUSTOM_BLOCKERS[0] = (Rectangle){1145.0f, 520.0f, 1915.0f - 1145.0f, 12.0f};

    /* Horizontal blocker from 0 x 245 to 1080 x 245 */
    WORLD_CS_CUSTOM_BLOCKERS[1] = (Rectangle){0.0f, 245.0f, 1080.0f, 12.0f};

    /* Square with top-left at 1085 x 300 to 1140 x 360 */
    WORLD_CS_CUSTOM_BLOCKERS[2] = (Rectangle){1085.0f, 300.0f, 1140.0f - 1085.0f, 360.0f - 300.0f};

    WORLD_CS_CUSTOM_BLOCKER_COUNT = 3;
}

static void drawWorldCSPostAtBaseCenter(Vector2 baseCenter) {
    if (WORLD_CS_POST_TEXTURE.id == 0) {
        return;
    }

    Vector2 drawPosition = {
        baseCenter.x - ((float)WORLD_CS_POST_TEXTURE.width * 0.5f),
        baseCenter.y - (float)WORLD_CS_POST_TEXTURE.height
    };

    DrawTextureV(WORLD_CS_POST_TEXTURE, drawPosition, WHITE);
}

static Rectangle getWorldCSPostRect(Vector2 baseCenter) {
    if (WORLD_CS_POST_TEXTURE.id == 0) {
        return (Rectangle){0.0f, 0.0f, 0.0f, 0.0f};
    }

    return (Rectangle){
        baseCenter.x - ((float)WORLD_CS_POST_TEXTURE.width * 0.25f),
        baseCenter.y - (float)WORLD_CS_POST_TEXTURE.height,
        (float)WORLD_CS_POST_TEXTURE.width * 0.7f,
        (float)WORLD_CS_POST_TEXTURE.height * 0.9f
    };
}

static void rebuildWorldCSPostBlockers(void) {
    if (WORLD_CS_POST_TEXTURE.id == 0 || WORLD_CS_POST_TEXTURE.width <= 0 || WORLD_CS_POST_TEXTURE.height <= 0) {
        WORLD_CS_POST_BLOCKER_COUNT = 0;
        return;
    }

    WORLD_CS_POST_BLOCKERS[0] = getWorldCSPostRect(WORLD_CS_POST_POSITION);
    WORLD_CS_POST_BLOCKER_COUNT = 1;
}

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
        *outCount = WORLD_CS_EDGE_BLOCKER_COUNT + WORLD_CS_TREE_BLOCKER_COUNT + WORLD_CS_POST_BLOCKER_COUNT + WORLD_CS_CUSTOM_BLOCKER_COUNT;
    }

    if (outBlockers == NULL || (WORLD_CS_EDGE_BLOCKER_COUNT + WORLD_CS_TREE_BLOCKER_COUNT + WORLD_CS_POST_BLOCKER_COUNT + WORLD_CS_CUSTOM_BLOCKER_COUNT) <= 0) {
        return;
    }

    int idx = 0;
    for (int i = 0; i < WORLD_CS_EDGE_BLOCKER_COUNT; i++) {
        outBlockers[idx++] = WORLD_CS_EDGE_BLOCKERS[i];
    }

    for (int i = 0; i < WORLD_CS_TREE_BLOCKER_COUNT; i++) {
        outBlockers[idx++] = WORLD_CS_TREE_BLOCKERS[i];
    }

    for (int i = 0; i < WORLD_CS_POST_BLOCKER_COUNT; i++) {
        outBlockers[idx++] = WORLD_CS_POST_BLOCKERS[i];
    }

    for (int i = 0; i < WORLD_CS_CUSTOM_BLOCKER_COUNT; i++) {
        outBlockers[idx++] = WORLD_CS_CUSTOM_BLOCKERS[i];
    }
}

static void drawWorldCSOverlay(void) {
    DrawRectangleLinesEx(WORLD_CS_NEXT_WORLD_TRIGGER, 2.0f, ORANGE);
    DrawRectangleLinesEx(WORLD_CS_PREVIOUS_WORLD_TRIGGER, 2.0f, ORANGE);

    drawWorldCSTreeAtBaseCenter(WORLD_CS_TREE_POSITION);
    drawWorldCSPostAtBaseCenter(WORLD_CS_POST_POSITION);
    for (int i = 0; i < WORLD_CS_CUSTOM_BLOCKER_COUNT; i++) {
        DrawRectangleLinesEx(WORLD_CS_CUSTOM_BLOCKERS[i], 2.0f, RED);
    }

    for (int i = 0; i < WORLD_CS_EDGE_BLOCKER_COUNT; i++) {
        DrawRectangleLinesEx(WORLD_CS_EDGE_BLOCKERS[i], 2.0f, GREEN);
    }

    for (int i = 0; i < WORLD_CS_TREE_BLOCKER_COUNT; i++) {
        DrawRectangleLinesEx(WORLD_CS_TREE_BLOCKERS[i], 2.0f, GREEN);
    }
    for (int i = 0; i < WORLD_CS_POST_BLOCKER_COUNT; i++) {
        DrawRectangleLinesEx(WORLD_CS_POST_BLOCKERS[i], 2.0f, GREEN);
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
    if (WORLD_CS_TREE_TEXTURE.id != 0) {
        UnloadTexture(WORLD_CS_TREE_TEXTURE);
    }

    WORLD_CS_TREE_TEXTURE = LoadTexture(WORLD_CS_TREE_PATH);
    if (WORLD_CS_POST_TEXTURE.id != 0) {
        UnloadTexture(WORLD_CS_POST_TEXTURE);
    }

    WORLD_CS_POST_TEXTURE = LoadTexture(WORLD_CS_POST_PATH);
    rebuildWorldCSBorderBlockers();
    rebuildWorldCSTreeBlockers();
    rebuildWorldCSPostBlockers();
    rebuildWorldCSCustomBlockers();
    configureCameraForCurrentWorld();
}

static void teardownWorldCSNode(void) {
    if (mapTexture.id != 0) {
        UnloadTexture(mapTexture);
        mapTexture.id = 0;
    }

    if (WORLD_CS_TREE_TEXTURE.id != 0) {
        UnloadTexture(WORLD_CS_TREE_TEXTURE);
        WORLD_CS_TREE_TEXTURE = (Texture2D){0};
    }

    if (WORLD_CS_POST_TEXTURE.id != 0) {
        UnloadTexture(WORLD_CS_POST_TEXTURE);
        WORLD_CS_POST_TEXTURE = (Texture2D){0};
    }

    WORLD_CS_EDGE_BLOCKER_COUNT = 0;
    WORLD_CS_TREE_BLOCKER_COUNT = 0;
    WORLD_CS_POST_BLOCKER_COUNT = 0;
    WORLD_CS_CUSTOM_BLOCKER_COUNT = 0;
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
