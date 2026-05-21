#ifndef TRAP_H
#define TRAP_H

#include <stdbool.h>
#include "interactable.h"
#include "../entities/player.h"

typedef struct Trap {
    Interactable base;
    Texture2D hiddenTex;
    Texture2D discoveredTex;
    bool discovered;
    int lastSteppedEntityId;
    int damage;
    int leaderInside;
} Trap;

void trap_init_from_asset(Trap* trap, const char* hidden_path, const char* discovered_path, int damage);
Interactable createTrap(Vector2 position, const char* hidden_path, const char* discovered_path, int damage);

void trap_onEnter(Interactable* self, Player* leader);
void trap_onLeave(Interactable* self, Player* leader);
void trap_on_update(Interactable* self, Vector2 playerPos);
void trap_on_draw(Interactable* self);
void trap_on_unload(Interactable* self);

#endif