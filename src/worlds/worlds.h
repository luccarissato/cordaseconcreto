#ifndef WORLDS_H
#define WORLDS_H

#include "raylib.h"

typedef struct WorldNode WorldNode;

typedef void (*WorldSetupCallback)(void* userData);
typedef void (*WorldTeardownCallback)(void);
typedef void (*WorldCollectBlockersCallback)(Rectangle* outBlockers, int* outCount);
typedef void (*WorldDrawCallback)(void);

struct WorldNode {
    const char* name;
    WorldSetupCallback setup;
    WorldTeardownCallback teardown;
    WorldCollectBlockersCallback collectBlockers;
    WorldDrawCallback drawOverlay;
    void* userData;
    WorldNode* prev;
    WorldNode* next;
};

void initWorldRegistry(void);
WorldNode* registerWorldNode(WorldNode* node);
WorldNode* getCurrentWorldNode(void);
void collectCurrentWorldBlockers(Rectangle* outBlockers, int* outCount);
void drawCurrentWorldOverlay(void);

int loadCurrentWorld(void);
int loadNextWorld(void);
int loadPreviousWorld(void);
int loadWorldByName(const char* name);

void requestWorldLoadCurrent(void);
void requestWorldLoadNext(void);
void requestWorldLoadPrevious(void);
void requestWorldLoadByName(const char* name);
int processPendingWorldLoad(void);

void shutdownWorldRegistry(void);

#endif