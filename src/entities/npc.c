#include "npc.h"
#include "../core/state.h"
#include "raymath.h"
#include <stdio.h>

void initNPC(NPC* npc, Vector2 position, const char* spritePath, DialogueTree* tree) {
    npc->position = position;
    npc->sprite = LoadTexture(spritePath);
    npc->dialogueTree = tree;
    npc->interactionDistance = 240.0f;
}

void updateNPC(NPC* npc, Vector2 playerPos) {
    if (currentGameState != STATE_EXPLORATION) return;

    Vector2 npcCenter = {
        npc->position.x + npc->sprite.width / 2,
        npc->position.y + npc->sprite.height / 2
    };

    Vector2 playerCenter = {
        playerPos.x + 150,
        playerPos.y + 150
    };

    float distance = Vector2Distance(
        npcCenter,
        playerCenter
    );

    if (distance <= npc->interactionDistance) {
        if (IsKeyPressed(KEY_Z)) {
            startDialogue(npc->dialogueTree);
        }
    }
}

void drawNPC(NPC* npc) {
    DrawTexture(npc->sprite, npc->position.x, npc->position.y, WHITE);

    // DrawCircleLines(
    // npc->position.x + npc->sprite.width/2,
    // npc->position.y + npc->sprite.height/2,
    // npc->interactionDistance,
    // RED
    // );
}

void unloadNPC(NPC* npc) {
    UnloadTexture(npc->sprite);
}