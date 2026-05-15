#ifndef DOOR_H
#define DOOR_H

#include "interactable.h"
#include "../core/dialogue.h"

typedef struct {
    Texture2D spriteClosed;
    Texture2D spriteOpen;
    int isOpen;                         /* 1 = aberta, 0 = fechada */
    DialogueTree* questionTree;         /* Pergunta/resposta */
    int dialogueWasActive;              /* Detectar se diálogo terminou */
    int correctAnswerNodeIndex;         /* Node que indica resposta correta */
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

#endif
