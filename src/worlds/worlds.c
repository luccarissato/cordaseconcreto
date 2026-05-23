#include "worlds.h"

#include <string.h>

static WorldNode* worldHead = NULL;
static WorldNode* worldCurrent = NULL;
static int worldCount = 0;
static int worldLoaded = 0;

typedef enum {
    PENDING_NONE = 0,
    PENDING_CURRENT,
    PENDING_NEXT,
    PENDING_PREVIOUS,
    PENDING_BY_NAME
} PendingWorldLoadType;

static PendingWorldLoadType pendingType = PENDING_NONE;
static char pendingWorldName[32];

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
    worldCurrent->setup(worldCurrent->userData);
    worldLoaded = 1;
    return 1;
}

void initWorldRegistry(void) {
    worldHead = NULL;
    worldCurrent = NULL;
    worldCount = 0;
    worldLoaded = 0;
    pendingType = PENDING_NONE;
    pendingWorldName[0] = '\0';
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

int loadCurrentWorld(void) {
    if (worldCurrent == NULL) {
        worldCurrent = worldHead;
    }

    if (worldCurrent == NULL) {
        return 0;
    }

    return loadWorldNode(worldCurrent);
}

int loadNextWorld(void) {
    if (worldCurrent == NULL) {
        worldCurrent = worldHead;
    } else {
        worldCurrent = worldCurrent->next;
    }

    if (worldCurrent == NULL) {
        return 0;
    }

    return loadWorldNode(worldCurrent);
}

int loadPreviousWorld(void) {
    if (worldCurrent == NULL) {
        worldCurrent = worldHead;
    } else {
        worldCurrent = worldCurrent->prev;
    }

    if (worldCurrent == NULL) {
        return 0;
    }

    return loadWorldNode(worldCurrent);
}

int loadWorldByName(const char* name) {
    if (name == NULL || worldHead == NULL) {
        return 0;
    }

    WorldNode* current = worldHead;
    for (int i = 0; i < worldCount; i++) {
        if (current->name != NULL && strcmp(current->name, name) == 0) {
            return loadWorldNode(current);
        }
        current = current->next;
    }

    return 0;
}

void requestWorldLoadCurrent(void) {
    pendingType = PENDING_CURRENT;
}

void requestWorldLoadNext(void) {
    pendingType = PENDING_NEXT;
}

void requestWorldLoadPrevious(void) {
    pendingType = PENDING_PREVIOUS;
}

void requestWorldLoadByName(const char* name) {
    pendingType = PENDING_BY_NAME;
    if (name == NULL) {
        pendingWorldName[0] = '\0';
        return;
    }

    strncpy(pendingWorldName, name, sizeof(pendingWorldName) - 1);
    pendingWorldName[sizeof(pendingWorldName) - 1] = '\0';
}

int processPendingWorldLoad(void) {
    switch (pendingType) {
        case PENDING_CURRENT:
            pendingType = PENDING_NONE;
            return loadCurrentWorld();

        case PENDING_NEXT:
            pendingType = PENDING_NONE;
            return loadNextWorld();

        case PENDING_PREVIOUS:
            pendingType = PENDING_NONE;
            return loadPreviousWorld();

        case PENDING_BY_NAME:
            pendingType = PENDING_NONE;
            return loadWorldByName(pendingWorldName);

        case PENDING_NONE:
        default:
            return 0;
    }
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
    pendingWorldName[0] = '\0';
}