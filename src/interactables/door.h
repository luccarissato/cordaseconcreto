#ifndef DOOR_H
#define DOOR_H

#include "interactable.h"
#include "../core/dialogue.h"

typedef enum {
    DOOR_TRANSITION_NONE = 0,
    DOOR_TRANSITION_NEXT,
    DOOR_TRANSITION_PREVIOUS,
    DOOR_TRANSITION_TARGET
} DoorTransitionMode;

typedef struct {
    Texture2D spriteClosed;
    Texture2D spriteOpen;
    int isOpen;                         /* 1 = aberta, 0 = fechada */
    DialogueTree* questionTree;         /* Pergunta/resposta */
    int dialogueWasActive;              /* Detectar se diálogo terminou */
    int correctAnswerNodeIndex;         /* Node que indica resposta correta */
    DoorTransitionMode transitionMode;  /* Tipo de transição de mundo */
    char targetWorldName[32];           /* Mundo alvo para transição direta */
} DoorData;

Interactable createDoor(
    Vector2 position,
    const char* closedSpritePath,
    const char* openSpritePath,
    DialogueTree* questionTree,
    int correctAnswerNodeIndex
);

void door_on_interact(Interactable* self, void* playerData);
void door_on_update(Interactable* self, Vector2 playerPos);
void door_on_draw(Interactable* self);
void door_on_unload(Interactable* self);
void doorConfigureWorldTransition(Interactable* self, DoorTransitionMode mode, const char* targetWorldName);

#endif
