#include "npc.h"
#include "../core/state.h"
#include "raymath.h"
#include <stdio.h>

extern NPC testNPC;
extern NPC worldCSNpc;
extern NPC worldPCNpc1;
extern NPC worldPCNpc2;
extern NPC worldMCQuestNpc;

void initNPC(NPC* npc, Vector2 position, const char* spritePath, DialogueTree* tree, const char* name) {
    npc->position = position;
    npc->sprite = LoadTexture(spritePath);
    npc->collider.offset = (Vector2){ 32.0f, 32.0f };
    npc->collider.size = (Vector2){ 120.0f, 120.0f };
    npc->dialogueTree = tree;
    npc->name = name;
    npc->interactionDistance = 50.0f;
    npc->preInteract = NULL;
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
            if (npc->preInteract != NULL) {
                int handled = npc->preInteract(npc);
                if (handled) return;
            }
            startDialogue(npc->dialogueTree, npc->name);
        }
    }
}

void drawNPC(NPC* npc) {
    DrawTexture(npc->sprite, npc->position.x, npc->position.y, WHITE);

    /* colision debug */

    /* DEBUG: mostra raio de interação (comentado por padrão) */
    // Vector2 center = getColliderCenter(npc->position, npc->collider);
    // DrawCircleLines(center.x, center.y, npc->interactionDistance, RED);
}

void unloadNPC(NPC* npc) {
    UnloadTexture(npc->sprite);
    npc->sprite = (Texture2D){0};
    npc->position = (Vector2){0.0f, 0.0f};
    npc->collider.offset = (Vector2){0.0f, 0.0f};
    npc->collider.size = (Vector2){0.0f, 0.0f};
    npc->dialogueTree = NULL;
    npc->interactionDistance = 0.0f;
}

static int isNpcLoaded(const NPC* npc) {
    return npc != NULL && npc->sprite.id != 0;
}

int collectLoadedNpcBlockers(Rectangle* outBlockers, int* outCount) {
    int count = 0;

    if (isNpcLoaded(&testNPC)) {
        if (outBlockers != NULL) {
            outBlockers[count] = getColliderRect(testNPC.position, testNPC.collider);
        }
        count++;
    }

    if (isNpcLoaded(&worldCSNpc)) {
        if (outBlockers != NULL) {
            outBlockers[count] = getColliderRect(worldCSNpc.position, worldCSNpc.collider);
        }
        count++;
    }

    if (isNpcLoaded(&worldPCNpc1)) {
        if (outBlockers != NULL) {
            outBlockers[count] = getColliderRect(worldPCNpc1.position, worldPCNpc1.collider);
        }
        count++;
    }

    if (isNpcLoaded(&worldPCNpc2)) {
        if (outBlockers != NULL) {
            outBlockers[count] = getColliderRect(worldPCNpc2.position, worldPCNpc2.collider);
        }
        count++;
    }

    if (isNpcLoaded(&worldMCQuestNpc)) {
        if (outBlockers != NULL) {
            outBlockers[count] = getColliderRect(worldMCQuestNpc.position, worldMCQuestNpc.collider);
        }
        count++;
    }

    if (outCount != NULL) {
        *outCount = count;
    }

    return count;
}

void updateLoadedNpcs(Vector2 playerPos) {
    if (isNpcLoaded(&testNPC)) {
        updateNPC(&testNPC, playerPos);
    }

    if (isNpcLoaded(&worldCSNpc)) {
        updateNPC(&worldCSNpc, playerPos);
    }

    if (isNpcLoaded(&worldPCNpc1)) {
        updateNPC(&worldPCNpc1, playerPos);
    }

    if (isNpcLoaded(&worldPCNpc2)) {
        updateNPC(&worldPCNpc2, playerPos);
    }

    if (isNpcLoaded(&worldMCQuestNpc)) {
        updateNPC(&worldMCQuestNpc, playerPos);
    }
}

void drawLoadedNpcs(void) {
    if (isNpcLoaded(&testNPC)) {
        drawNPC(&testNPC);
    }

    if (isNpcLoaded(&worldCSNpc)) {
        drawNPC(&worldCSNpc);
    }

    if (isNpcLoaded(&worldPCNpc1)) {
        drawNPC(&worldPCNpc1);
    }

    if (isNpcLoaded(&worldPCNpc2)) {
        drawNPC(&worldPCNpc2);
    }

    if (isNpcLoaded(&worldMCQuestNpc)) {
        drawNPC(&worldMCQuestNpc);
    }
}
