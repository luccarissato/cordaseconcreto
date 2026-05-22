#include "door.h"
#include "../core/state.h"
#include "../core/dialogue.h"
#include "../worlds/worlds.h"
#include "raylib.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static void queueDoorWorldTransition(DoorData* data) {
    if (data == NULL) return;

    switch (data->transitionMode) {
        case DOOR_TRANSITION_NEXT:
            requestWorldLoadNext();
            break;

        case DOOR_TRANSITION_PREVIOUS:
            requestWorldLoadPrevious();
            break;

        case DOOR_TRANSITION_TARGET:
            if (data->targetWorldName[0] != '\0') {
                requestWorldLoadByName(data->targetWorldName);
            }
            break;

        case DOOR_TRANSITION_NONE:
        default:
            break;
    }
}

// door_on_interact() - Executado quando jogador pressiona Z próximo
void door_on_interact(Interactable* self, void* playerData) {
    DoorData* data = (DoorData*) self->data;
    
    if (self->hasInteracted) {
        return;
    }
    
    startDialogue(data->questionTree);
    data->dialogueWasActive = 1;
}

// door_on_update() - Executado a cada frame
void door_on_update(Interactable* self, Vector2 playerPos) {
    DoorData* data = (DoorData*) self->data;
    
    if (self->hasInteracted) {
        return;
    }
    
    if (isDialogueActive()) {
        return;
    }
    
    if (data->dialogueWasActive && !isDialogueActive()) {
        data->dialogueWasActive = 0;
        
        int playerAnswerNode = data->questionTree->currentNode;
        
        if (playerAnswerNode == data->correctAnswerNodeIndex) {
            self->hasInteracted = 1;
            data->isOpen = 1;
            self->sprite = data->spriteOpen;

            queueDoorWorldTransition(data);
            
            #ifdef DEBUG_DOOR
            printf("[DOOR] Resposta correta! Porta aberta.\n");
            #endif
        } else {
            /* Resposta errada: mantém fechada */
            #ifdef DEBUG_DOOR
            printf("[DOOR] Resposta errada. Porta continua fechada.\n");
            #endif
        }
        
        data->questionTree->currentNode = 0;
        data->questionTree->selectedChoice = 0;
        data->questionTree->active = 0;
    }
    
    if (currentGameState != STATE_EXPLORATION) {
        return;
    }
    
    if (isPlayerNearInteractable(self, playerPos)) {
        if (IsKeyPressed(KEY_Z)) {
            self->on_interact(self, NULL);
        }
    }
}

// door_on_draw() - Renderiza o sprite atual 
void door_on_draw(Interactable* self) {
    DoorData* data = (DoorData*) self->data;
    
    DrawTexture(self->sprite, self->position.x, self->position.y, WHITE);
    
    Rectangle collider = getColliderRect(self->position, self->collider);
    Color debugColor = (data->isOpen) ? GREEN : RED;
    DrawRectangleLinesEx(collider, 2.0f, debugColor);
}

// door_on_unload() - Libera recursos
void door_on_unload(Interactable* self) {
    DoorData* data = (DoorData*) self->data;
    UnloadTexture(data->spriteClosed);
    UnloadTexture(data->spriteOpen);
    free(data);
}

void doorConfigureWorldTransition(Interactable* self, DoorTransitionMode mode, const char* targetWorldName) {
    if (self == NULL || self->data == NULL) return;

    DoorData* data = (DoorData*) self->data;
    data->transitionMode = mode;
    data->targetWorldName[0] = '\0';

    if (targetWorldName != NULL) {
        strncpy(data->targetWorldName, targetWorldName, sizeof(data->targetWorldName) - 1);
        data->targetWorldName[sizeof(data->targetWorldName) - 1] = '\0';
    }
}

Interactable createDoor(Vector2 position, const char* closedSpritePath, const char* openSpritePath, DialogueTree* questionTree, int correctAnswerNodeIndex) {
    DoorData* data = malloc(sizeof(DoorData));
    data->spriteClosed = LoadTexture(closedSpritePath);
    data->spriteOpen = LoadTexture(openSpritePath);
    data->isOpen = 0;
    data->questionTree = questionTree;
    data->dialogueWasActive = 0;
    data->correctAnswerNodeIndex = correctAnswerNodeIndex;
    data->transitionMode = DOOR_TRANSITION_NONE;
    data->targetWorldName[0] = '\0';
    
    Interactable door = {
        .type = INTERACTABLE_DOOR,
        .position = position,
        .collider = {
            .offset = {0.0f, 0.0f},
            .size = {400.0f, 400.0f}
        },
        .hasInteracted = 0,
        .interactionDistance = 80.0f,
        .sprite = data->spriteClosed,
        
        .on_interact = door_on_interact,
        .on_update = door_on_update,
        .on_draw = door_on_draw,
        .on_unload = door_on_unload,
        
        .data = (void*) data
    };
    
    return door;
}
