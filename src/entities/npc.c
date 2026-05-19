#include "npc.h"
#include "../core/state.h"
#include "raymath.h"
#include <stdio.h>

void initNPC(NPC* npc, Vector2 position, const char* spritePath, DialogueTree* tree) {
    npc->position = position;
    npc->sprite = LoadTexture(spritePath);
    npc->collider.offset = (Vector2){ 0.0f, 0.0f };
    npc->collider.size = (Vector2){ 300.0f, 300.0f };
    npc->dialogueTree = tree;
    npc->interactionDistance = 80.0f;
}

void updateNPC(NPC* npc, Vector2 playerPos) {
    if (currentGameState != STATE_EXPLORATION) return;

    Collider playerCollider = {
        .offset = {75.0f, 0.0f},
        .size = {150.0f, 300.0f}
    };

    int isNear = areCollidersNearEdgeBased(playerPos, playerCollider, npc->position, npc->collider, npc->interactionDistance);
    
    if (isNear) {
        if (IsKeyPressed(KEY_Z)) {
            startDialogue(npc->dialogueTree);
        }
    }
}

void drawNPC(NPC* npc) {
    DrawTexture(npc->sprite, npc->position.x, npc->position.y, WHITE);

    /* colision debug */
    Rectangle colliderRect = getColliderRect(npc->position, npc->collider);
    DrawRectangleLinesEx(colliderRect, 2.0f, RED);

    /* DEBUG: mostra raio de interação (comentado por padrão) */
    // Vector2 center = getColliderCenter(npc->position, npc->collider);
    // DrawCircleLines(center.x, center.y, npc->interactionDistance, RED);
}

void unloadNPC(NPC* npc) {
    UnloadTexture(npc->sprite);
}