#include "world_labirinto.h"

#include "worlds.h"
#include "../core/game.h"
#include "../core/collision.h"
#include "../entities/player.h"
#include "../interactables/interactable.h"
#include "../interactables/trap.h"
#include "raylib.h"
#include <stddef.h>
#include <stdlib.h>

extern InteractableManager interactableManager;

static const Vector2 WORLD_CS_LABIRINTO_RETURN_SPAWN = {880.0f, 410.0f};
static const char* WORLD_LABIRINTO_MAP_PATH = "assets/cenarios/PUZZLE_CS.png";
static const char* WORLD_LABIRINTO_BOSS2_TEXTURE_PATH = "assets/antagonistas/PAPA_FIGO.png";
static const char* WORLD_LABIRINTO_TRAP_PATH = "assets/icones/trap.png";
static const float WORLD_LABIRINTO_PLAYER_SCALE = 35.0f / 184.0f;
static const float WORLD_LABIRINTO_BORDER_THICKNESS = 64.0f;
static const int WORLD_LABIRINTO_TRAP_DAMAGE = 30;

static const Rectangle WORLD_LABIRINTO_RETURN_TRIGGER = {915.0f, 60.0f, 80.0f, 12.0f};
static const Rectangle WORLD_LABIRINTO_BOSS2_TRIGGER = {915.0f, 1055.0f, 80.0f, 12.0f};

static Rectangle WORLD_LABIRINTO_EDGE_BLOCKERS[4];
static int WORLD_LABIRINTO_EDGE_BLOCKER_COUNT = 0;
static Rectangle* WORLD_LABIRINTO_BLACK_BLOCKERS = NULL;
static int WORLD_LABIRINTO_BLACK_BLOCKER_COUNT = 0;
static int WORLD_LABIRINTO_BLACK_BLOCKER_CAPACITY = 0;
static int WORLD_LABIRINTO_BOSS2_TRIGGER_USED = 0;

static const Vector2 WORLD_LABIRINTO_TRAP_POSITIONS[] = {
    {250.0f, 90.0f},
    {380.0f, 280.0f},
    {1070.0f, 290.0f},
    {1650.0f, 220.0f},
    {750.0f, 730.0f},
    {1420.0f, 730.0f},
    {1270.0f, 860.0f},
    {1420.0f, 350.0f},
    {1050.0f, 600.0f}
};

static const Rectangle WORLD_LABIRINTO_BLACK_COLLISION_EXCEPTIONS[] = {
    {900.0f, 900.0f, 101.0f, 180.0f},
    {557.0f, 216.0f, 82.0f, 42.0f},
    {1200.0f, 391.0f, 60.0f, 44.0f}
};

typedef struct BlackRun {
    int x;
    int width;
    int rectIndex;
} BlackRun;

static int isInsideExceptionAreaPixel(int x, int y) {
    for (int i = 0; i < (int)(sizeof(WORLD_LABIRINTO_BLACK_COLLISION_EXCEPTIONS) / sizeof(WORLD_LABIRINTO_BLACK_COLLISION_EXCEPTIONS[0])); i++) {
        Rectangle exception = WORLD_LABIRINTO_BLACK_COLLISION_EXCEPTIONS[i];
        if ((float)x >= exception.x &&
            (float)x < exception.x + exception.width &&
            (float)y >= exception.y &&
            (float)y < exception.y + exception.height) {
            return 1;
        }
    }

    return 0;
}

static int isPureBlackPixel(Color color) {
    return color.a > 0 && color.r == 0 && color.g == 0 && color.b == 0;
}

static void setupWorldLabirintoTraps(void) {
    initInteractableManager(&interactableManager);

    for (int i = 0; i < (int)(sizeof(WORLD_LABIRINTO_TRAP_POSITIONS) / sizeof(WORLD_LABIRINTO_TRAP_POSITIONS[0])); i++) {
        Interactable trap = createTrap(
            WORLD_LABIRINTO_TRAP_POSITIONS[i],
            WORLD_LABIRINTO_TRAP_PATH,
            WORLD_LABIRINTO_TRAP_PATH,
            WORLD_LABIRINTO_TRAP_DAMAGE
        );
        addInteractable(&interactableManager, &trap);
    }
}

static void clearWorldLabirintoBlackBlockers(void) {
    free(WORLD_LABIRINTO_BLACK_BLOCKERS);
    WORLD_LABIRINTO_BLACK_BLOCKERS = NULL;
    WORLD_LABIRINTO_BLACK_BLOCKER_COUNT = 0;
    WORLD_LABIRINTO_BLACK_BLOCKER_CAPACITY = 0;
}

static int appendWorldLabirintoBlackBlocker(Rectangle blocker) {
    if (WORLD_LABIRINTO_BLACK_BLOCKER_COUNT >= WORLD_LABIRINTO_BLACK_BLOCKER_CAPACITY) {
        int nextCapacity = (WORLD_LABIRINTO_BLACK_BLOCKER_CAPACITY == 0) ? 128 : WORLD_LABIRINTO_BLACK_BLOCKER_CAPACITY * 2;
        Rectangle* nextBlockers = realloc(WORLD_LABIRINTO_BLACK_BLOCKERS, sizeof(Rectangle) * nextCapacity);
        if (nextBlockers == NULL) {
            return -1;
        }

        WORLD_LABIRINTO_BLACK_BLOCKERS = nextBlockers;
        WORLD_LABIRINTO_BLACK_BLOCKER_CAPACITY = nextCapacity;
    }

    WORLD_LABIRINTO_BLACK_BLOCKERS[WORLD_LABIRINTO_BLACK_BLOCKER_COUNT] = blocker;
    return WORLD_LABIRINTO_BLACK_BLOCKER_COUNT++;
}

static int appendBlackRun(BlackRun** runs, int* count, int* capacity, BlackRun run) {
    if (*count >= *capacity) {
        int nextCapacity = (*capacity == 0) ? 32 : (*capacity * 2);
        BlackRun* nextRuns = realloc(*runs, sizeof(BlackRun) * nextCapacity);
        if (nextRuns == NULL) {
            return 0;
        }

        *runs = nextRuns;
        *capacity = nextCapacity;
    }

    (*runs)[(*count)++] = run;
    return 1;
}

static int findActiveRun(const BlackRun* activeRuns, int activeRunCount, int x, int width) {
    for (int i = 0; i < activeRunCount; i++) {
        if (activeRuns[i].x == x && activeRuns[i].width == width) {
            return i;
        }
    }

    return -1;
}

static void rebuildWorldLabirintoBlackBlockers(void) {
    clearWorldLabirintoBlackBlockers();

    Image image = LoadImage(WORLD_LABIRINTO_MAP_PATH);
    if (image.data == NULL || image.width <= 0 || image.height <= 0) {
        return;
    }

    Color* pixels = LoadImageColors(image);
    if (pixels == NULL) {
        UnloadImage(image);
        return;
    }

    BlackRun* activeRuns = NULL;
    int activeRunCount = 0;
    int activeRunCapacity = 0;

    for (int y = 0; y < image.height; y++) {
        BlackRun* currentRuns = NULL;
        int currentRunCount = 0;
        int currentRunCapacity = 0;

        int x = 0;
        while (x < image.width) {
            Color color = pixels[y * image.width + x];
            int isBlack = isPureBlackPixel(color) && !isInsideExceptionAreaPixel(x, y);
            if (!isBlack) {
                x++;
                continue;
            }

            int runStart = x;
            while (x < image.width) {
                color = pixels[y * image.width + x];
                isBlack = isPureBlackPixel(color) && !isInsideExceptionAreaPixel(x, y);
                if (!isBlack) {
                    break;
                }
                x++;
            }

            int runWidth = x - runStart;
            int activeIndex = findActiveRun(activeRuns, activeRunCount, runStart, runWidth);
            int rectIndex = -1;

            if (activeIndex >= 0) {
                rectIndex = activeRuns[activeIndex].rectIndex;
                WORLD_LABIRINTO_BLACK_BLOCKERS[rectIndex].height += 1.0f;
            } else {
                rectIndex = appendWorldLabirintoBlackBlocker((Rectangle){(float)runStart, (float)y, (float)runWidth, 1.0f});
                if (rectIndex < 0) {
                    free(currentRuns);
                    free(activeRuns);
                    UnloadImageColors(pixels);
                    UnloadImage(image);
                    clearWorldLabirintoBlackBlockers();
                    return;
                }
            }

            if (!appendBlackRun(&currentRuns, &currentRunCount, &currentRunCapacity, (BlackRun){runStart, runWidth, rectIndex})) {
                free(currentRuns);
                free(activeRuns);
                UnloadImageColors(pixels);
                UnloadImage(image);
                clearWorldLabirintoBlackBlockers();
                return;
            }
        }

        free(activeRuns);
        activeRuns = currentRuns;
        activeRunCount = currentRunCount;
        activeRunCapacity = currentRunCapacity;
    }

    free(activeRuns);
    UnloadImageColors(pixels);
    UnloadImage(image);

}

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
        *outCount = WORLD_LABIRINTO_EDGE_BLOCKER_COUNT + WORLD_LABIRINTO_BLACK_BLOCKER_COUNT;
    }

    if (outBlockers == NULL) {
        return;
    }

    for (int i = 0; i < WORLD_LABIRINTO_EDGE_BLOCKER_COUNT; i++) {
        outBlockers[i] = WORLD_LABIRINTO_EDGE_BLOCKERS[i];
    }

    for (int i = 0; i < WORLD_LABIRINTO_BLACK_BLOCKER_COUNT; i++) {
        outBlockers[WORLD_LABIRINTO_EDGE_BLOCKER_COUNT + i] = WORLD_LABIRINTO_BLACK_BLOCKERS[i];
    }
}

static void drawWorldLabirintoOverlay(void) {
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

    if (!WORLD_LABIRINTO_BOSS2_TRIGGER_USED && CheckCollisionRecs(playerRect, WORLD_LABIRINTO_BOSS2_TRIGGER)) {
        int bossIndex = enemyManager.count;
        spawnEnemy(
            "Papa figo",
            (Vector2){WORLD_LABIRINTO_BOSS2_TRIGGER.x, WORLD_LABIRINTO_BOSS2_TRIGGER.y},
            WORLD_LABIRINTO_BOSS2_TEXTURE_PATH,
            MAX_ENEMIES
        );

        if (enemyManager.count > bossIndex) {
            setEnemyStats(&enemyManager.enemies[bossIndex], 700, 36, 14, 11);
            setEnemyElementalResistances(&enemyManager.enemies[bossIndex], -10, -10, -10, -10);
        }

        WORLD_LABIRINTO_BOSS2_TRIGGER_USED = 1;
        startCombat(playerPos, COMBAT_DETECTION_DISTANCE);
        return 1;
    }

    return 0;
}

static void setupWorldLabirintoNode(void* userData) {
    (void)userData;

    setPlayerWorldScale(WORLD_LABIRINTO_PLAYER_SCALE);
    initParty(0, getWorldSpawnPosition(WORLD_LABIRINTO_PARTY_SPAWN));
    setupWorldLabirintoTraps();

    if (mapTexture.id != 0) {
        UnloadTexture(mapTexture);
    }

    mapTexture = LoadTexture(WORLD_LABIRINTO_MAP_PATH);
    WORLD_LABIRINTO_BOSS2_TRIGGER_USED = 0;
    rebuildWorldLabirintoBorderBlockers();
    rebuildWorldLabirintoBlackBlockers();
    configureCameraForCurrentWorld();
}

static void teardownWorldLabirintoNode(void) {
    setPlayerWorldScale(1.0f);

    if (mapTexture.id != 0) {
        UnloadTexture(mapTexture);
        mapTexture.id = 0;
    }

    WORLD_LABIRINTO_EDGE_BLOCKER_COUNT = 0;
    clearWorldLabirintoBlackBlockers();
    unloadInteractableManager(&interactableManager);
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
