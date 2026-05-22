#ifndef WORLDS_H
#define WORLDS_H

typedef struct WorldNode WorldNode;

typedef void (*WorldSetupCallback)(void* userData);
typedef void (*WorldTeardownCallback)(void);

struct WorldNode {
    const char* name;
    WorldSetupCallback setup;
    WorldTeardownCallback teardown;
    void* userData;
    WorldNode* prev;
    WorldNode* next;
};

void initWorldRegistry(void);
WorldNode* registerWorldNode(WorldNode* node);
WorldNode* getCurrentWorldNode(void);

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