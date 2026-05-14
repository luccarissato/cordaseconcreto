#ifndef INTERACTABLE_H
#define INTERACTABLE_H

#include "raylib.h"
#include "../core/collision.h"

typedef enum {
    INTERACTABLE_CHEST,
    INTERACTABLE_DOOR,      /* futuro: diálogo + pergunta */
    INTERACTABLE_TRAP,      /* futuro: sem colisão, trigger ao pisar */
} InteractableType;

typedef struct Interactable Interactable;

typedef void (*InteractCallback)(Interactable* self, void* playerData);
typedef void (*UpdateCallback)(Interactable* self, Vector2 playerPos);
typedef void (*DrawCallback)(Interactable* self);
typedef void (*UnloadCallback)(Interactable* self);

typedef struct Interactable {
    InteractableType type;
    Vector2 position;
    Collider collider;
    int hasInteracted;              /* Flag: já foi ativado? */
    float interactionDistance;
    
    Texture2D sprite;               /* Sprite atual (pode mudar) */
    
    InteractCallback on_interact;
    UpdateCallback on_update;
    DrawCallback on_draw;
    UnloadCallback on_unload;
    
    void* data;
} Interactable;

typedef struct {
    Interactable* list;
    int count;
    int capacity;
} InteractableManager;

/* Inicializa gerenciador vazio (capacity padrão = 16) */
void initInteractableManager(InteractableManager* manager);

/* Adiciona um interagível à lista */
void addInteractable(InteractableManager* manager, Interactable* interactable);

/* Update de todos os interagíveis (chama on_update callback) */
void updateInteractables(InteractableManager* manager, Vector2 playerPos);

/* Draw de todos os interagíveis (chama on_draw callback) */
void drawInteractables(InteractableManager* manager);

/* Gera array de Rectangle (bloqueadores de colisão) apenas de interagíveis
   que têm colisão (ex: caixas, portas; NOT traps) */
Rectangle* getInteractableBlockers(InteractableManager* manager, int* outCount);

/* Libera memória de todos os interagíveis */
void unloadInteractableManager(InteractableManager* manager);

/* Utilitário: detecta proximidade entre interagível e jogador */
int isPlayerNearInteractable(const Interactable* interactable, Vector2 playerPos);

/* Utilitário: retorna retângulo de colisão */
Rectangle getInteractableRect(const Interactable* interactable);

#endif
