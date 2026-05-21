#include "trap.h"
#include "../core/game.h"
#include "../core/collision.h"
#include "../entities/player.h"
#include "raylib.h"
#include <stdlib.h>

static void damagePartyAllMembers(int damage) {
    int partyCount = 0;
    Player* partyMembers = getPartyMembers(&partyCount);

    if (partyMembers == NULL || partyCount <= 0) {
        return;
    }

    for (int i = 0; i < partyCount; i++) {
        Player* member = &partyMembers[i];
        if (member->stats.currentHP > 0) {
            member->stats.currentHP -= damage;
            if (member->stats.currentHP < 0) {
                member->stats.currentHP = 0;
            }
            member->isAlive = (member->stats.currentHP > 0);
        }
    }
}

void trap_init_from_asset(Trap* trap, const char* hidden_path, const char* discovered_path, int damage) {
    if (trap == NULL) {
        return;
    }

    trap->hiddenTex = LoadTexture(hidden_path);
    trap->discoveredTex = LoadTexture(discovered_path);
    trap->discovered = false;
    trap->lastSteppedEntityId = -1;
    trap->damage = damage;
    trap->leaderInside = 0;

    trap->base.type = INTERACTABLE_TRAP;
    trap->base.position = (Vector2){0.0f, 0.0f};
    trap->base.collider.offset = (Vector2){0.0f, 0.0f};
    trap->base.collider.size = (Vector2){(float)trap->hiddenTex.width, (float)trap->hiddenTex.height};
    trap->base.hasInteracted = 0;
    trap->base.interactionDistance = 0.0f;
    trap->base.sprite = trap->hiddenTex;
    trap->base.on_interact = NULL;
    trap->base.on_update = trap_on_update;
    trap->base.on_draw = trap_on_draw;
    trap->base.on_unload = trap_on_unload;
    trap->base.data = trap;
}

static Trap* getTrapData(Interactable* self) {
    if (self == NULL) {
        return NULL;
    }

    return (Trap*)self->data;
}

void trap_onEnter(Interactable* self, Player* leader) {
    Trap* trap = getTrapData(self);
    if (trap == NULL || leader == NULL) {
        return;
    }

    int partyCount = 0;
    Player* partyMembers = getPartyMembers(&partyCount);
    if (partyMembers == NULL || partyCount <= 0 || leader != &partyMembers[0]) {
        return;
    }

    trap->discovered = true;
    trap->lastSteppedEntityId = leader->characterID;
    trap->base.sprite = trap->discoveredTex;
    self->sprite = trap->discoveredTex;

    damagePartyAllMembers(trap->damage);
}

void trap_onLeave(Interactable* self, Player* leader) {
    Trap* trap = getTrapData(self);
    if (trap == NULL || leader == NULL) {
        return;
    }

    int partyCount = 0;
    Player* partyMembers = getPartyMembers(&partyCount);
    if (partyMembers == NULL || partyCount <= 0 || leader != &partyMembers[0]) {
        return;
    }

    if (trap->lastSteppedEntityId == leader->characterID) {
        trap->lastSteppedEntityId = -1;
    }

    trap->leaderInside = 0;
}

void trap_on_update(Interactable* self, Vector2 playerPos) {
    Trap* trap = getTrapData(self);
    if (trap == NULL) {
        return;
    }

    int partyCount = 0;
    Player* partyMembers = getPartyMembers(&partyCount);
    if (partyMembers == NULL || partyCount <= 0) {
        return;
    }

    Player* leader = &partyMembers[0];
    Collider leaderCollider = leader->collider;
    Rectangle leaderRect = getColliderRect(playerPos, leaderCollider);
    Rectangle trapRect = getInteractableRect(self);
    int isOverlapping = rectsCollide(leaderRect, trapRect);

    if (isOverlapping && !trap->leaderInside) {
        trap_onEnter(self, leader);
    } else if (!isOverlapping && trap->leaderInside) {
        trap_onLeave(self, leader);
    }

    trap->leaderInside = isOverlapping;

    if (trap->discovered) {
        self->sprite = trap->discoveredTex;
    } else {
        self->sprite = trap->hiddenTex;
    }
}

void trap_on_draw(Interactable* self) {
    if (self == NULL) {
        return;
    }

    Trap* trap = getTrapData(self);
    if (trap == NULL) {
        return;
    }

    Texture2D spriteToDraw = trap->discovered ? trap->discoveredTex : trap->hiddenTex;
    if (spriteToDraw.id == 0) {
        spriteToDraw = self->sprite;
    }

    if (spriteToDraw.id == 0) {
        return;
    }

    DrawTexture(spriteToDraw, (int)self->position.x, (int)self->position.y, WHITE);

    Rectangle collider = getColliderRect(self->position, self->collider);
    Color debugColor = trap->discovered ? RED : ORANGE;
    DrawRectangleLinesEx(collider, 2.0f, debugColor);
}

void trap_on_unload(Interactable* self) {
    Trap* trap = getTrapData(self);
    if (trap == NULL) {
        return;
    }

    UnloadTexture(trap->hiddenTex);
    UnloadTexture(trap->discoveredTex);
    free(trap);
}

Interactable createTrap(Vector2 position, const char* hidden_path, const char* discovered_path, int damage) {
    Trap* trap = (Trap*)calloc(1, sizeof(Trap));
    if (trap == NULL) {
        Interactable empty = {0};
        return empty;
    }

    trap_init_from_asset(trap, hidden_path, discovered_path, damage);
    trap->base.position = position;

    return trap->base;
}