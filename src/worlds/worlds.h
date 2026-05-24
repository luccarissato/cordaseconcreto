#ifndef WORLDS_H
#define WORLDS_H

#include "raylib.h"

typedef struct WorldNode WorldNode;

typedef void (*WorldSetupCallback)(void* userData);
typedef void (*WorldTeardownCallback)(void);
typedef void (*WorldCollectBlockersCallback)(Rectangle* outBlockers, int* outCount);
typedef void (*WorldDrawCallback)(void);
typedef int (*WorldTriggerCallback)(Vector2 playerPos);

typedef void (*WorldCameraBoundsCallback)(Rectangle* outBounds);

#define WORLD_NAME_MC "MZERO_ATUALIZADO"
#define WORLD_NAME_CS "CS_FRENTE"

typedef enum {
    WORLD_TRANSITION_NEXT = 0,
    WORLD_TRANSITION_PREVIOUS
} WorldTransitionDirection;

typedef struct {
    Rectangle bounds;
    WorldTransitionDirection direction;
    Vector2 targetSpawnPosition;
} WorldTransitionZone;

struct WorldNode {
    const char* name;
    WorldSetupCallback setup;
    WorldTeardownCallback teardown;
    WorldCollectBlockersCallback collectBlockers;
    WorldDrawCallback drawOverlay;
    WorldTriggerCallback processTriggers;
    WorldCameraBoundsCallback getCameraBounds;
    void* userData;
    WorldNode* prev;
    WorldNode* next;
};

void initWorldRegistry(void);
WorldNode* registerWorldNode(WorldNode* node);
WorldNode* getCurrentWorldNode(void);
void collectCurrentWorldBlockers(Rectangle* outBlockers, int* outCount);
void drawCurrentWorldOverlay(void);
int processCurrentWorldTriggers(Vector2 playerPos);
void getCurrentWorldCameraBounds(Rectangle* outBounds);

int loadCurrentWorld(void);
int loadNextWorld(void);
int loadPreviousWorld(void);

Vector2 getWorldSpawnPosition(Vector2 fallbackPosition);
void requestWorldLoadCurrent(void);
void requestWorldLoadNext(void);
void requestWorldLoadPrevious(void);
void requestWorldTransitionNext(Vector2 spawnPosition);
void requestWorldTransitionPrevious(Vector2 spawnPosition);
int processPendingWorldLoad(void);
int processWorldTransitionZones(Rectangle playerRect, const WorldTransitionZone* zones, int zoneCount);

void shutdownWorldRegistry(void);

#endif
