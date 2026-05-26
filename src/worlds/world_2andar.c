#include "world_2andar.h"

#include "worlds.h"
#include "../core/game.h"
#include "../core/collision.h"
#include "../core/state.h"
#include "../interactables/interactable.h"
#include "../interactables/door.h"
#include "../data/dialogues/door_questions.h"
#include "raylib.h"
#include <stddef.h>

extern InteractableManager interactableManager;

static const Vector2 WORLD_2ANDAR_PARTY_SPAWN = {1830.0f, 940.0f};
static const Vector2 WORLD_1ANDAR_RETURN_SPAWN = {1690.0f, 700.0f};
static const char* WORLD_2ANDAR_MAP_PATH = "assets/cenarios/A2_PCFREVO.png";
static const char* WORLD_2ANDAR_AGUA_PATH = "assets/cenarios/AGUA_A1_PCFREVO.png";
static const char* WORLD_2ANDAR_FOGO_PATH = "assets/cenarios/FOGO_A1_PCFREVO.png";
static const char* WORLD_2ANDAR_TERRA_PATH = "assets/cenarios/TERRA_A1_PCFREVO.png";
static const char* WORLD_2ANDAR_VENTO_PATH = "assets/cenarios/VENTO_A1_PCFREVO.png";
static const char* WORLD_2ANDAR_DOOR_CLOSED_PATH = "assets/interagiveis/PORTA_FECHADA_A2_PCFREVO.png";
static const char* WORLD_2ANDAR_DOOR_OPEN_PATH = "assets/interagiveis/PORTAO_ABERTO_A2_PCFREVO.png";
static const char* WORLD_2ANDAR_BOSS1_TEXTURE_PATH = "assets/antagonistas/MULHER_GUARDA_CHUVA.png";
static const float WORLD_2ANDAR_BORDER_THICKNESS = 128.0f;
static const float WORLD_2ANDAR_DOOR_HALF_WIDTH = 200.0f;

static const Rectangle WORLD_2ANDAR_PREVIOUS_WORLD_TRIGGER = {1755.0f, 1020.0f, 12.0f, 12.0f};
static const Rectangle WORLD_2ANDAR_BOSS1_TRIGGER = {150.0f, 900.0f, 12.0f, 12.0f};
static const Vector2 WORLD_2ANDAR_BOSS1_SPAWN = {150.0f, 900.0f};
static const WorldTransitionZone WORLD_2ANDAR_TRANSITIONS[] = {
    {WORLD_2ANDAR_PREVIOUS_WORLD_TRIGGER, WORLD_TRANSITION_PREVIOUS, WORLD_1ANDAR_RETURN_SPAWN}
};

static Rectangle WORLD_2ANDAR_EDGE_BLOCKERS[4];
static int WORLD_2ANDAR_EDGE_BLOCKER_COUNT = 0;
static Rectangle WORLD_2ANDAR_CUSTOM_BLOCKERS[2];
static int WORLD_2ANDAR_CUSTOM_BLOCKER_COUNT = 0;
static Interactable WORLD_2ANDAR_DOORS[3];
static int WORLD_2ANDAR_DOOR_COUNT = 0;
static int WORLD_2ANDAR_BOSS1_TRIGGER_USED = 0;
static Texture2D WORLD_2ANDAR_AGUA_TEXTURE = {0};
static Texture2D WORLD_2ANDAR_FOGO_TEXTURE = {0};
static Texture2D WORLD_2ANDAR_TERRA_TEXTURE = {0};
static Texture2D WORLD_2ANDAR_VENTO_TEXTURE = {0};

static const Vector2 WORLD_2ANDAR_AGUA_POSITION = {100.0f, -110.0f};
static const Vector2 WORLD_2ANDAR_FOGO_POSITION = {620.0f, -110.0f};
static const Vector2 WORLD_2ANDAR_TERRA_POSITION = {1300.0f, -110.0f};
static const Vector2 WORLD_2ANDAR_VENTO_POSITION = {1775.0f, -110.0f};
static const Vector2 WORLD_2ANDAR_DOOR_TOP_CENTERS[3] = {
    {600.0f, 365.0f},
    {1100.0f, 365.0f},
    {1600.0f, 365.0f}
};

static Vector2 getWorld2AndarDoorPosition(Vector2 topCenter) {
    return (Vector2){topCenter.x - WORLD_2ANDAR_DOOR_HALF_WIDTH, topCenter.y};
}

static void setupWorld2AndarDoors(void) {
    initInteractableManager(&interactableManager);

    WORLD_2ANDAR_DOORS[0] = createDoor(
        getWorld2AndarDoorPosition(WORLD_2ANDAR_DOOR_TOP_CENTERS[0]),
        WORLD_2ANDAR_DOOR_CLOSED_PATH,
        WORLD_2ANDAR_DOOR_OPEN_PATH,
        &doorQuestion4,
        1
    );

    WORLD_2ANDAR_DOORS[1] = createDoor(
        getWorld2AndarDoorPosition(WORLD_2ANDAR_DOOR_TOP_CENTERS[1]),
        WORLD_2ANDAR_DOOR_CLOSED_PATH,
        WORLD_2ANDAR_DOOR_OPEN_PATH,
        &doorQuestion5,
        1
    );

    WORLD_2ANDAR_DOORS[2] = createDoor(
        getWorld2AndarDoorPosition(WORLD_2ANDAR_DOOR_TOP_CENTERS[2]),
        WORLD_2ANDAR_DOOR_CLOSED_PATH,
        WORLD_2ANDAR_DOOR_OPEN_PATH,
        &doorQuestion6,
        1
    );

    for (int i = 0; i < 3; i++) {
        addInteractable(&interactableManager, &WORLD_2ANDAR_DOORS[i]);
    }

    WORLD_2ANDAR_DOOR_COUNT = 3;
}

static void drawTriggerRect(Rectangle rect, Color color) {
    (void)rect;
    (void)color;
}

static void drawSpriteAtTopCenter(Texture2D texture, Vector2 topCenterPosition) {
    if (texture.id == 0) {
        return;
    }

    Vector2 drawPosition = {
        topCenterPosition.x - ((float)texture.width * 0.5f),
        topCenterPosition.y
    };

    DrawTextureV(texture, drawPosition, WHITE);
}

static void rebuildWorld2AndarBorderBlockers(void) {
    float mapWidth = (float)mapTexture.width;
    float mapHeight = (float)mapTexture.height;

    if (mapWidth <= 0.0f || mapHeight <= 0.0f) {
        WORLD_2ANDAR_EDGE_BLOCKER_COUNT = 0;
        return;
    }

    WORLD_2ANDAR_EDGE_BLOCKERS[0] = (Rectangle){-WORLD_2ANDAR_BORDER_THICKNESS, 0.0f, WORLD_2ANDAR_BORDER_THICKNESS, mapHeight};
    WORLD_2ANDAR_EDGE_BLOCKERS[1] = (Rectangle){mapWidth, 0.0f, WORLD_2ANDAR_BORDER_THICKNESS, mapHeight};
    WORLD_2ANDAR_EDGE_BLOCKERS[2] = (Rectangle){0.0f, -WORLD_2ANDAR_BORDER_THICKNESS, mapWidth, WORLD_2ANDAR_BORDER_THICKNESS};
    WORLD_2ANDAR_EDGE_BLOCKERS[3] = (Rectangle){0.0f, mapHeight, mapWidth, WORLD_2ANDAR_BORDER_THICKNESS};
    WORLD_2ANDAR_EDGE_BLOCKER_COUNT = 4;
}

static void collectWorld2AndarBlockers(Rectangle* outBlockers, int* outCount) {
    if (outCount != NULL) {
        *outCount = WORLD_2ANDAR_EDGE_BLOCKER_COUNT + WORLD_2ANDAR_CUSTOM_BLOCKER_COUNT;
    }

    if (outBlockers == NULL) {
        return;
    }

    int idx = 0;
    for (int i = 0; i < WORLD_2ANDAR_EDGE_BLOCKER_COUNT; i++) {
        outBlockers[idx++] = WORLD_2ANDAR_EDGE_BLOCKERS[i];
    }

    for (int i = 0; i < WORLD_2ANDAR_CUSTOM_BLOCKER_COUNT; i++) {
        outBlockers[idx++] = WORLD_2ANDAR_CUSTOM_BLOCKERS[i];
    }
}

static void drawWorld2AndarOverlay(void) {
    drawSpriteAtTopCenter(WORLD_2ANDAR_AGUA_TEXTURE, WORLD_2ANDAR_AGUA_POSITION);
    drawSpriteAtTopCenter(WORLD_2ANDAR_FOGO_TEXTURE, WORLD_2ANDAR_FOGO_POSITION);
    drawSpriteAtTopCenter(WORLD_2ANDAR_TERRA_TEXTURE, WORLD_2ANDAR_TERRA_POSITION);
    drawSpriteAtTopCenter(WORLD_2ANDAR_VENTO_TEXTURE, WORLD_2ANDAR_VENTO_POSITION);
}

static void getWorld2AndarBounds(Rectangle* outBounds) {
    if (outBounds == NULL) {
        return;
    }

    *outBounds = (Rectangle){0.0f, 0.0f, (float)mapTexture.width, (float)mapTexture.height};
}

static int processWorld2AndarTriggers(Vector2 playerPos) {
    Collider playerCollider = {
        .offset = {75.0f, 0.0f},
        .size = {150.0f, 200.0f}
    };

    Rectangle playerRect = getColliderRect(playerPos, playerCollider);

    if (!WORLD_2ANDAR_BOSS1_TRIGGER_USED && currentGameState == STATE_EXPLORATION) {
        if (CheckCollisionRecs(playerRect, WORLD_2ANDAR_BOSS1_TRIGGER)) {
            int bossIndex = enemyManager.count;

            spawnEnemy("Mulher do Guarda Chuva Branco", WORLD_2ANDAR_BOSS1_SPAWN, WORLD_2ANDAR_BOSS1_TEXTURE_PATH, MAX_ENEMIES);
            if (enemyManager.count > bossIndex) {
                setEnemyStats(&enemyManager.enemies[bossIndex], 700, 34, 5, 12);
                setEnemyElementalResistances(&enemyManager.enemies[bossIndex], 20, 20, 20, 20);
            }

            WORLD_2ANDAR_BOSS1_TRIGGER_USED = 1;
            startCombat(playerPos, COMBAT_DETECTION_DISTANCE);
            return 1;
        }
    }

    return processWorldTransitionZones(
        playerRect,
        WORLD_2ANDAR_TRANSITIONS,
        (int)(sizeof(WORLD_2ANDAR_TRANSITIONS) / sizeof(WORLD_2ANDAR_TRANSITIONS[0]))
    );
}

static void setupWorld2AndarNode(void* userData) {
    (void)userData;

    initParty(0, getWorldSpawnPosition(WORLD_2ANDAR_PARTY_SPAWN));
    setupWorld2AndarDoors();

    if (mapTexture.id != 0) {
        UnloadTexture(mapTexture);
    }

    mapTexture = LoadTexture(WORLD_2ANDAR_MAP_PATH);
    if (WORLD_2ANDAR_AGUA_TEXTURE.id != 0) {
        UnloadTexture(WORLD_2ANDAR_AGUA_TEXTURE);
    }
    if (WORLD_2ANDAR_FOGO_TEXTURE.id != 0) {
        UnloadTexture(WORLD_2ANDAR_FOGO_TEXTURE);
    }
    if (WORLD_2ANDAR_TERRA_TEXTURE.id != 0) {
        UnloadTexture(WORLD_2ANDAR_TERRA_TEXTURE);
    }
    if (WORLD_2ANDAR_VENTO_TEXTURE.id != 0) {
        UnloadTexture(WORLD_2ANDAR_VENTO_TEXTURE);
    }

    WORLD_2ANDAR_AGUA_TEXTURE = LoadTexture(WORLD_2ANDAR_AGUA_PATH);
    WORLD_2ANDAR_FOGO_TEXTURE = LoadTexture(WORLD_2ANDAR_FOGO_PATH);
    WORLD_2ANDAR_TERRA_TEXTURE = LoadTexture(WORLD_2ANDAR_TERRA_PATH);
    WORLD_2ANDAR_VENTO_TEXTURE = LoadTexture(WORLD_2ANDAR_VENTO_PATH);

    WORLD_2ANDAR_CUSTOM_BLOCKERS[0] = (Rectangle){5.0f, 220.0f, 1910.0f, 12.0f};
    WORLD_2ANDAR_CUSTOM_BLOCKERS[1] = (Rectangle){175.0f, 845.0f, 1570.0f, 12.0f};
    WORLD_2ANDAR_CUSTOM_BLOCKER_COUNT = 2;
    WORLD_2ANDAR_BOSS1_TRIGGER_USED = 0;

    rebuildWorld2AndarBorderBlockers();
    configureCameraForCurrentWorld();
}

static void teardownWorld2AndarNode(void) {
    if (mapTexture.id != 0) {
        UnloadTexture(mapTexture);
        mapTexture.id = 0;
    }

    if (WORLD_2ANDAR_AGUA_TEXTURE.id != 0) {
        UnloadTexture(WORLD_2ANDAR_AGUA_TEXTURE);
        WORLD_2ANDAR_AGUA_TEXTURE = (Texture2D){0};
    }

    if (WORLD_2ANDAR_FOGO_TEXTURE.id != 0) {
        UnloadTexture(WORLD_2ANDAR_FOGO_TEXTURE);
        WORLD_2ANDAR_FOGO_TEXTURE = (Texture2D){0};
    }

    if (WORLD_2ANDAR_TERRA_TEXTURE.id != 0) {
        UnloadTexture(WORLD_2ANDAR_TERRA_TEXTURE);
        WORLD_2ANDAR_TERRA_TEXTURE = (Texture2D){0};
    }

    if (WORLD_2ANDAR_VENTO_TEXTURE.id != 0) {
        UnloadTexture(WORLD_2ANDAR_VENTO_TEXTURE);
        WORLD_2ANDAR_VENTO_TEXTURE = (Texture2D){0};
    }

    WORLD_2ANDAR_EDGE_BLOCKER_COUNT = 0;
    WORLD_2ANDAR_CUSTOM_BLOCKER_COUNT = 0;
    WORLD_2ANDAR_DOOR_COUNT = 0;
    WORLD_2ANDAR_BOSS1_TRIGGER_USED = 0;
    unloadInteractableManager(&interactableManager);
    unloadParty();
}

void RegisterWorld2Andar(void) {
    static WorldNode world2AndarNode = {
        .name = "2ANDAR_PCFREVO",
        .setup = setupWorld2AndarNode,
        .teardown = teardownWorld2AndarNode,
        .collectBlockers = collectWorld2AndarBlockers,
        .drawOverlay = drawWorld2AndarOverlay,
        .processTriggers = processWorld2AndarTriggers,
        .getCameraBounds = getWorld2AndarBounds,
        .userData = NULL,
        .prev = NULL,
        .next = NULL
    };

    registerWorldNode(&world2AndarNode);
}
