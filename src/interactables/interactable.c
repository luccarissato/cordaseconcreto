#include "interactable.h"
#include "raymath.h"
#include <stdlib.h>
#include <string.h>

static float distance(Vector2 a, Vector2 b) {
    return Vector2Distance(a, b);
}

static Vector2 getInteractableCenter(const Interactable* inter) {
    return (Vector2){
        inter->position.x + inter->sprite.width / 2.0f,
        inter->position.y + inter->sprite.height / 2.0f
    };
}

static Vector2 getPlayerCenter(Vector2 playerPos) {
    return (Vector2){ playerPos.x + 150.0f, playerPos.y + 150.0f };
}

void initInteractableManager(InteractableManager* manager) {
    manager->capacity = 16;
    manager->count = 0;
    manager->list = malloc(sizeof(Interactable) * manager->capacity);
}

void addInteractable(InteractableManager* manager, Interactable* interactable) {
    if (manager->count >= manager->capacity) {
        manager->capacity *= 2;
        manager->list = realloc(manager->list, 
                                sizeof(Interactable) * manager->capacity);
    }
    
    manager->list[manager->count] = *interactable;
    manager->count++;
}

void updateInteractables(InteractableManager* manager, Vector2 playerPos) {
    for (int i = 0; i < manager->count; i++) {
        Interactable* inter = &manager->list[i];
        
        if (inter->on_update) {
            inter->on_update(inter, playerPos);
        }
    }
}

void drawInteractables(InteractableManager* manager) {
    for (int i = 0; i < manager->count; i++) {
        Interactable* inter = &manager->list[i];
        
        if (inter->on_draw) {
            inter->on_draw(inter);
        }
    }
}

Rectangle* getInteractableBlockers(InteractableManager* manager, int* outCount) {
    Rectangle* blockers = malloc(sizeof(Rectangle) * manager->count);
    int count = 0;
    
    for (int i = 0; i < manager->count; i++) {
        Interactable* inter = &manager->list[i];
        
        if (inter->type != INTERACTABLE_TRAP) {
            blockers[count] = getInteractableRect(inter);
            count++;
        }
    }
    
    *outCount = count;
    return blockers;
}

void unloadInteractableManager(InteractableManager* manager) {
    for (int i = 0; i < manager->count; i++) {
        Interactable* inter = &manager->list[i];
        
        if (inter->on_unload) {
            inter->on_unload(inter);
        }
    }
    
    free(manager->list);
    manager->list = NULL;
    manager->count = 0;
    manager->capacity = 0;
}

int isPlayerNearInteractable(const Interactable* interactable, Vector2 playerPos) {
    Vector2 interCenter = getInteractableCenter(interactable);
    Vector2 playerCenter = getPlayerCenter(playerPos);
    
    float dist = distance(interCenter, playerCenter);
    return dist <= interactable->interactionDistance;
}

Rectangle getInteractableRect(const Interactable* interactable) {
    return getColliderRect(interactable->position, interactable->collider);
}
