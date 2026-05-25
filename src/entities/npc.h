#ifndef NPC_H
#define NPC_H

#include "raylib.h"
#include "../core/collision.h"
#include "../core/dialogue.h"

typedef struct NPC {
    Vector2 position;
    Texture2D sprite;
    Collider collider;
    const char* name;
    DialogueTree* dialogueTree;
    float interactionDistance;
    /* Optional pre-interaction callback: return 1 if handled (do not start dialogue), 0 to continue normal dialogue */
    int (*preInteract)(struct NPC* npc);
} NPC;

void initNPC(NPC* npc, Vector2 position, const char* spritePath, DialogueTree* tree, const char* name);
void updateNPC(NPC* npc, Vector2 playerPos);
void drawNPC(NPC* npc);
void unloadNPC(NPC* npc);

int collectLoadedNpcBlockers(Rectangle* outBlockers, int* outCount);
void updateLoadedNpcs(Vector2 playerPos);
void drawLoadedNpcs(void);

#endif