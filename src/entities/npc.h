#ifndef NPC_H
#define NPC_H

#include "raylib.h"
#include "../core/dialogue.h"

typedef struct {
    Vector2 position;
    Texture2D sprite;
    DialogueTree* dialogueTree;
    float interactionDistance;
} NPC;

void initNPC(NPC* npc, Vector2 position, const char* spritePath, DialogueTree* tree);
void updateNPC(NPC* npc, Vector2 playerPos);
void drawNPC(NPC* npc);
void unloadNPC(NPC* npc);

#endif