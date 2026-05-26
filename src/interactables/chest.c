#include "chest.h"
#include "../items/game_items.h"
#include "../core/state.h"
#include "raylib.h"
#include <stdlib.h>
#include <stdio.h>

extern Inventory playerInventory;

static Item* chestLootPool[] = {
    &caldinhoFeijao,
    &tapiocaRecheada,
    &pratoBuchada,
    &cozidoNordestino,
    &churrascadaCompleta,
    &cocada,
    &canjica,
    &broaMilho,
    &boloDeRolo,
    &pudimLeite,
    &cafeBoloRolo,
    &queijoGoiabada,
    &cartola,
    &baiaoDeDois,
    &festaDeSaoJoao,
    &ervaCidreira,
    &chaCamomila,
    &garrafadaNordestina,
    &aguaBenta,
    &cachaçaMedicinal,
    &rapadura,
    &cuscuzReforçado,
    &cafeExpresso,
    &feijoada,
    &antiveneno,
    &pomadaCicatrizante,
    &panaceiaUniversal,
    &vitaminaFortitude,
    &elixirMental,
    &proteínaConcentrada,
    &mineralResistente,
    &suplementoVelocidade,
    &tonicoPerseveranca,
    &chaIluminacao,
    &poDeEnvenenar,
    &pimentaMalagueta,
    &cogumelo,
};

static const int CHEST_LOOT_POOL_SIZE = sizeof(chestLootPool) / sizeof(chestLootPool[0]);

static Item* selectRandomItem(void) {
    int randomIndex = rand() % CHEST_LOOT_POOL_SIZE;
    return chestLootPool[randomIndex];
}


static void addItemToInventory(Item* baseItem) {
    InventoryItem* newItem = malloc(sizeof(InventoryItem));
    newItem->baseItem = baseItem;
    newItem->quantity = 1;
    addItemInventory(&playerInventory, newItem);
}


void chest_on_interact(Interactable* self, void* playerData) {
    ChestData* data = (ChestData*) self->data;
    
    if (!self->hasInteracted) {
        self->hasInteracted = 1;
        data->isClosed = 0;
        
        Item* selectedItem = selectRandomItem();
        addItemToInventory(selectedItem);
        
        self->sprite = data->spriteOpen;
    }
}

void chest_on_update(Interactable* self, Vector2 playerPos) {
    if (currentGameState != STATE_EXPLORATION) return;
    
    if (isPlayerNearInteractable(self, playerPos)) {
        if (IsKeyPressed(KEY_Z)) {
            self->on_interact(self, NULL);
        }
    }
}

void chest_on_draw(Interactable* self) {
    DrawTexture(self->sprite, self->position.x, self->position.y, WHITE);
    
    /* DEBUG: mostra colisão */
    
    /* DEBUG: mostra raio de interação (comentado por padrão) */
    // Vector2 center = (Vector2){
    //     self->position.x + self->sprite.width / 2,
    //     self->position.y + self->sprite.height / 2
    // };
    // DrawCircleLines(center.x, center.y, self->interactionDistance, BLUE);
}

void chest_on_unload(Interactable* self) {
    ChestData* data = (ChestData*) self->data;
    UnloadTexture(data->spriteClosed);
    UnloadTexture(data->spriteOpen);
    free(data);
}

Interactable createChest(
    Vector2 position,
    const char* closedSpritePath,
    const char* openSpritePath
) {
    ChestData* data = malloc(sizeof(ChestData));
    data->spriteClosed = LoadTexture(closedSpritePath);
    data->spriteOpen = LoadTexture(openSpritePath);
    data->isClosed = 1;
    data->itemCount = CHEST_LOOT_POOL_SIZE;
    
    Interactable chest = {
        .type = INTERACTABLE_CHEST,
        .position = position,
        .collider = {
            .offset = {0.0f, 0.0f},
            .size = {(float)data->spriteClosed.width, (float)data->spriteClosed.height}
        },
        .hasInteracted = 0,
        .interactionDistance = 80.0f,
        .sprite = data->spriteClosed,
        
        .on_interact = chest_on_interact,
        .on_update = chest_on_update,
        .on_draw = chest_on_draw,
        .on_unload = chest_on_unload,
        
        .data = (void*) data
    };
    
    return chest;
}

void addChestSequence(
    InteractableManager* manager,
    Vector2 firstBaseLeftPosition,
    int count,
    float gap,
    const char* closedSpritePath,
    const char* openSpritePath
) {
    Vector2 basePosition = firstBaseLeftPosition;

    for (int i = 0; i < count; i++) {
        Interactable chest = createChest(basePosition, closedSpritePath, openSpritePath);
        chest.position.y -= (float)chest.sprite.height;
        addInteractable(manager, &chest);
        basePosition.x += (float)chest.sprite.width + gap;
    }
}
