#ifndef CHEST_H
#define CHEST_H

#include "interactable.h"
#include "../items/inventory.h"

typedef struct {
    Texture2D spriteClosed;
    Texture2D spriteOpen;
    int isClosed;        
    int itemCount;          
} ChestData;


Interactable createChest(
    Vector2 position,
    const char* closedSpritePath,
    const char* openSpritePath
);

void chest_on_interact(Interactable* self, void* playerData);
void chest_on_update(Interactable* self, Vector2 playerPos);
void chest_on_draw(Interactable* self);
void chest_on_unload(Interactable* self);

#endif
