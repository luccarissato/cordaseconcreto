#include "worlds.h"

#include <stddef.h>

static WorldNode* worldHead = NULL;
static WorldNode* worldCurrent = NULL;
static int worldCount = 0;
static int worldLoaded = 0;

typedef enum {
    PENDING_NONE = 0,
    PENDING_FIRST,
    PENDING_CURRENT,
    PENDING_NEXT,
    PENDING_PREVIOUS
} PendingWorldLoadType;

static PendingWorldLoadType pendingType = PENDING_NONE;
static int pendingHasSpawnOverride = 0;
static Vector2 pendingSpawnPosition = {0.0f, 0.0f};
static int setupHasSpawnOverride = 0;
static Vector2 setupSpawnPosition = {0.0f, 0.0f};

static int loadWorldNode(WorldNode* node) {
    if (node == NULL || node->setup == NULL) {
        return 0;
    }

    WorldNode* previousWorld = worldCurrent;

    if (worldLoaded && previousWorld != NULL && previousWorld->teardown != NULL) {
        worldLoaded = 0;
        previousWorld->teardown();
    }

    worldCurrent = node;
    setupHasSpawnOverride = pendingHasSpawnOverride;
    setupSpawnPosition = pendingSpawnPosition;
    pendingHasSpawnOverride = 0;
    worldCurrent->setup(worldCurrent->userData);
    setupHasSpawnOverride = 0;
    worldLoaded = 1;
    return 1;
}

void initWorldRegistry(void) {
    worldHead = NULL;
    worldCurrent = NULL;
    worldCount = 0;
    worldLoaded = 0;
    pendingType = PENDING_NONE;
    pendingHasSpawnOverride = 0;
    setupHasSpawnOverride = 0;
}

WorldNode* registerWorldNode(WorldNode* node) {
    if (node == NULL || node->setup == NULL) {
        return NULL;
    }

    node->prev = node;
    node->next = node;

    if (worldHead == NULL) {
        worldHead = node;
        worldCurrent = node;
    } else {
        WorldNode* tail = worldHead->prev;
        tail->next = node;
        node->prev = tail;
        node->next = worldHead;
        worldHead->prev = node;
    }

    worldCount++;
    return node;
}

WorldNode* getCurrentWorldNode(void) {
    return worldCurrent;
}

void collectCurrentWorldBlockers(Rectangle* outBlockers, int* outCount) {
    if (outCount != NULL) {
        *outCount = 0;
    }

    if (worldCurrent == NULL || worldCurrent->collectBlockers == NULL) {
        return;
    }

    worldCurrent->collectBlockers(outBlockers, outCount);
}

void drawCurrentWorldOverlay(void) {
    if (worldCurrent == NULL || worldCurrent->drawOverlay == NULL) {
        return;
    }

    worldCurrent->drawOverlay();
}

int processCurrentWorldTriggers(Vector2 playerPos) {
    if (worldCurrent == NULL || worldCurrent->processTriggers == NULL) {
        return 0;
    }

    return worldCurrent->processTriggers(playerPos);
}

void getCurrentWorldCameraBounds(Rectangle* outBounds) {
    if (outBounds != NULL) {
        *outBounds = (Rectangle){0.0f, 0.0f, 0.0f, 0.0f};
    }

    if (outBounds == NULL || worldCurrent == NULL || worldCurrent->getCameraBounds == NULL) {
        return;
    }

    worldCurrent->getCameraBounds(outBounds);
}

int loadCurrentWorld(void) {
    if (worldCurrent == NULL) {
        worldCurrent = worldHead;
    }

    if (worldCurrent == NULL) {
        return 0;
    }

    return loadWorldNode(worldCurrent);
}

int loadFirstWorld(void) {
    if (worldHead == NULL) {
        return 0;
    }

    return loadWorldNode(worldHead);
}

int loadNextWorld(void) {
    WorldNode* target = (worldCurrent == NULL) ? worldHead : worldCurrent->next;

    if (target == NULL) {
        return 0;
    }

    return loadWorldNode(target);
}

int loadPreviousWorld(void) {
    WorldNode* target = (worldCurrent == NULL) ? worldHead : worldCurrent->prev;

    if (target == NULL) {
        return 0;
    }

    return loadWorldNode(target);
}

Vector2 getWorldSpawnPosition(Vector2 fallbackPosition) {
    return setupHasSpawnOverride ? setupSpawnPosition : fallbackPosition;
}

void requestWorldLoadCurrent(void) {
    pendingType = PENDING_CURRENT;
    pendingHasSpawnOverride = 0;
}

void requestWorldLoadFirst(void) {
    pendingType = PENDING_FIRST;
    pendingHasSpawnOverride = 0;
}

void requestWorldLoadNext(void) {
    pendingType = PENDING_NEXT;
    pendingHasSpawnOverride = 0;
}

void requestWorldLoadPrevious(void) {
    pendingType = PENDING_PREVIOUS;
    pendingHasSpawnOverride = 0;
}

void requestWorldTransitionNext(Vector2 spawnPosition) {
    pendingType = PENDING_NEXT;
    pendingHasSpawnOverride = 1;
    pendingSpawnPosition = spawnPosition;
}

void requestWorldTransitionPrevious(Vector2 spawnPosition) {
    pendingType = PENDING_PREVIOUS;
    pendingHasSpawnOverride = 1;
    pendingSpawnPosition = spawnPosition;
}

int processPendingWorldLoad(void) {
    switch (pendingType) {
        case PENDING_FIRST:
            pendingType = PENDING_NONE;
            return loadFirstWorld();

        case PENDING_CURRENT:
            pendingType = PENDING_NONE;
            return loadCurrentWorld();

        case PENDING_NEXT:
            pendingType = PENDING_NONE;
            return loadNextWorld();

        case PENDING_PREVIOUS:
            pendingType = PENDING_NONE;
            return loadPreviousWorld();

        case PENDING_NONE:
        default:
            return 0;
    }
}

int processWorldTransitionZones(Rectangle playerRect, const WorldTransitionZone* zones, int zoneCount) {
    if (zones == NULL || zoneCount <= 0) {
        return 0;
    }

    for (int i = 0; i < zoneCount; i++) {
        if (CheckCollisionRecs(playerRect, zones[i].bounds)) {
            if (zones[i].direction == WORLD_TRANSITION_PREVIOUS) {
                requestWorldTransitionPrevious(zones[i].targetSpawnPosition);
            } else {
                requestWorldTransitionNext(zones[i].targetSpawnPosition);
            }
            return 1;
        }
    }

    return 0;
}

void shutdownWorldRegistry(void) {
    if (worldLoaded && worldCurrent != NULL && worldCurrent->teardown != NULL) {
        worldLoaded = 0;
        worldCurrent->teardown();
    }

    worldHead = NULL;
    worldCurrent = NULL;
    worldCount = 0;
    worldLoaded = 0;
    pendingType = PENDING_NONE;
    pendingHasSpawnOverride = 0;
    setupHasSpawnOverride = 0;
}
