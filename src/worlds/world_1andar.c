#include "world_1andar.h"

#include "worlds.h"
#include "../core/game.h"
#include "../core/collision.h"
#include "raylib.h"
#include "../interactables/interactable.h"
#include "../interactables/color_puzzle.h"
#include <stddef.h>

extern InteractableManager interactableManager;

static const Vector2 WORLD_1ANDAR_PARTY_SPAWN = {500.0f, 1000.0f};
static const Vector2 WORLD_PC_RETURN_SPAWN = {1325.0f, 240.0f};
static const char* WORLD_1ANDAR_MAP_PATH = "assets/cenarios/1ANDAR_PCFREVO.png";
static const float WORLD_1ANDAR_BORDER_THICKNESS = 128.0f;

/* A faixa de retorno foi interpretada como um retangulo fino vertical em x=35. */
static const Rectangle WORLD_1ANDAR_PREVIOUS_WORLD_TRIGGER = {35.0f, 405.0f, 12.0f, 155.0f};
static const WorldTransitionZone WORLD_1ANDAR_TRANSITIONS[] = {
    {WORLD_1ANDAR_PREVIOUS_WORLD_TRIGGER, WORLD_TRANSITION_PREVIOUS, WORLD_PC_RETURN_SPAWN}
};

static Rectangle WORLD_1ANDAR_EDGE_BLOCKERS[4];
static int WORLD_1ANDAR_EDGE_BLOCKER_COUNT = 0;
static Rectangle WORLD_1ANDAR_CUSTOM_BLOCKERS[5];
static int WORLD_1ANDAR_CUSTOM_BLOCKER_COUNT = 0;

static void drawTriggerRect(Rectangle rect, Color color) {
    DrawRectangleLinesEx(rect, 2.0f, color);
}

static void rebuildWorld1AndarBorderBlockers(void) {
    float mapWidth = (float)mapTexture.width;
    float mapHeight = (float)mapTexture.height;

    if (mapWidth <= 0.0f || mapHeight <= 0.0f) {
        WORLD_1ANDAR_EDGE_BLOCKER_COUNT = 0;
        return;
    }

    WORLD_1ANDAR_EDGE_BLOCKERS[0] = (Rectangle){-WORLD_1ANDAR_BORDER_THICKNESS, 0.0f, WORLD_1ANDAR_BORDER_THICKNESS, mapHeight};
    WORLD_1ANDAR_EDGE_BLOCKERS[1] = (Rectangle){mapWidth, 0.0f, WORLD_1ANDAR_BORDER_THICKNESS, mapHeight};
    WORLD_1ANDAR_EDGE_BLOCKERS[2] = (Rectangle){0.0f, -WORLD_1ANDAR_BORDER_THICKNESS, mapWidth, WORLD_1ANDAR_BORDER_THICKNESS};
    WORLD_1ANDAR_EDGE_BLOCKERS[3] = (Rectangle){0.0f, mapHeight, mapWidth, WORLD_1ANDAR_BORDER_THICKNESS};
    WORLD_1ANDAR_EDGE_BLOCKER_COUNT = 4;
}

static void rebuildWorld1AndarCustomBlockers(void) {
    /* 1) vertical at x=1080 from y=840..1075 */
    WORLD_1ANDAR_CUSTOM_BLOCKERS[0] = (Rectangle){1080.0f, 840.0f, 12.0f, 1075.0f - 840.0f};

    /* 2) horizontal at y=825 from x=1080..1745 */
    WORLD_1ANDAR_CUSTOM_BLOCKERS[1] = (Rectangle){1080.0f, 825.0f, 1745.0f - 1080.0f, 12.0f};

    /* 3) horizontal at y=600 from x=35..720 */
    WORLD_1ANDAR_CUSTOM_BLOCKERS[2] = (Rectangle){35.0f, 600.0f, 720.0f - 35.0f, 12.0f};

    /* 4) horizontal at y=705 from x=60..720 */
    WORLD_1ANDAR_CUSTOM_BLOCKERS[3] = (Rectangle){60.0f, 705.0f, 720.0f - 60.0f, 12.0f};

    /* 5) horizontal at y=225 from x=5..1895 */
    WORLD_1ANDAR_CUSTOM_BLOCKERS[4] = (Rectangle){5.0f, 225.0f, 1895.0f - 5.0f, 12.0f};

    WORLD_1ANDAR_CUSTOM_BLOCKER_COUNT = 5;
}

static void collectWorld1AndarBlockers(Rectangle* outBlockers, int* outCount) {
    if (outCount != NULL) {
        *outCount = WORLD_1ANDAR_EDGE_BLOCKER_COUNT + WORLD_1ANDAR_CUSTOM_BLOCKER_COUNT;
    }

    if (outBlockers == NULL || (WORLD_1ANDAR_EDGE_BLOCKER_COUNT + WORLD_1ANDAR_CUSTOM_BLOCKER_COUNT) <= 0) {
        return;
    }

    int idx = 0;
    for (int i = 0; i < WORLD_1ANDAR_EDGE_BLOCKER_COUNT; i++) {
        outBlockers[idx++] = WORLD_1ANDAR_EDGE_BLOCKERS[i];
    }

    for (int i = 0; i < WORLD_1ANDAR_CUSTOM_BLOCKER_COUNT; i++) {
        outBlockers[idx++] = WORLD_1ANDAR_CUSTOM_BLOCKERS[i];
    }
}

static void drawWorld1AndarOverlay(void) {
    drawTriggerRect(WORLD_1ANDAR_PREVIOUS_WORLD_TRIGGER, ORANGE);

    for (int i = 0; i < WORLD_1ANDAR_EDGE_BLOCKER_COUNT; i++) {
        DrawRectangleLinesEx(WORLD_1ANDAR_EDGE_BLOCKERS[i], 2.0f, GREEN);
    }
    for (int i = 0; i < WORLD_1ANDAR_CUSTOM_BLOCKER_COUNT; i++) {
        DrawRectangleLinesEx(WORLD_1ANDAR_CUSTOM_BLOCKERS[i], 2.0f, RED);
    }
}

static void getWorld1AndarBounds(Rectangle* outBounds) {
    if (outBounds == NULL) {
        return;
    }

    *outBounds = (Rectangle){0.0f, 0.0f, (float)mapTexture.width, (float)mapTexture.height};
}

static int processWorld1AndarTriggers(Vector2 playerPos) {
    Collider playerCollider = {
        .offset = {75.0f, 0.0f},
        .size = {150.0f, 200.0f}
    };

    return processWorldTransitionZones(
        getColliderRect(playerPos, playerCollider),
        WORLD_1ANDAR_TRANSITIONS,
        (int)(sizeof(WORLD_1ANDAR_TRANSITIONS) / sizeof(WORLD_1ANDAR_TRANSITIONS[0]))
    );
}

static void setupWorld1AndarNode(void* userData) {
    (void)userData;

    initParty(1, getWorldSpawnPosition(WORLD_1ANDAR_PARTY_SPAWN));

    /* Initialize interactables for this world and add the color puzzle */
    initInteractableManager(&interactableManager);

    Interactable colorPuzzle = createColorPuzzle(
        (Vector2){1105.0f, 395.0f},
        "assets/interagiveis/chao_azul.png",
        "assets/interagiveis/chao_verde.png",
        "assets/interagiveis/chao_amarelo.png",
        "assets/interagiveis/chao_vermelho.png",
        2.0f,
        50
    );
    addInteractable(&interactableManager, &colorPuzzle);

    if (mapTexture.id != 0) {
        UnloadTexture(mapTexture);
    }

    mapTexture = LoadTexture(WORLD_1ANDAR_MAP_PATH);
    rebuildWorld1AndarBorderBlockers();
    rebuildWorld1AndarCustomBlockers();
    configureCameraForCurrentWorld();
}

static void teardownWorld1AndarNode(void) {
    if (mapTexture.id != 0) {
        UnloadTexture(mapTexture);
        mapTexture.id = 0;
    }

    WORLD_1ANDAR_EDGE_BLOCKER_COUNT = 0;
    WORLD_1ANDAR_CUSTOM_BLOCKER_COUNT = 0;
    unloadInteractableManager(&interactableManager);
    unloadParty();
}

void RegisterWorld1Andar(void) {
    static WorldNode world1AndarNode = {
        .name = WORLD_NAME_1ANDAR,
        .setup = setupWorld1AndarNode,
        .teardown = teardownWorld1AndarNode,
        .collectBlockers = collectWorld1AndarBlockers,
        .drawOverlay = drawWorld1AndarOverlay,
        .processTriggers = processWorld1AndarTriggers,
        .getCameraBounds = getWorld1AndarBounds,
        .userData = NULL,
        .prev = NULL,
        .next = NULL
    };

    registerWorldNode(&world1AndarNode);
}