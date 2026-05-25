#include "test_world.h"

#include "worlds.h"
#include "../core/game.h"
#include "../core/dialogue.h"
#include "../items/game_items.h"
#include "../items/inventory.h"
#include "../entities/player.h"
#include "../entities/npc.h"
#include "../entities/enemy.h"
#include "../interactables/interactable.h"
#include "../interactables/chest.h"
#include "../interactables/color_puzzle.h"
#include "../interactables/door.h"
#include "../interactables/trap.h"
#include "../combat/combat.h"
#include "../combat/ability.h"
#include "../ui/combat_ui.h"
#include "../data/dialogues/door_questions.h"
#include "../data/dialogues/teste_dialogue.h"

#include <stdlib.h>

extern Item gotaSangreMaldita;
extern Item frascoMagicoCura;

extern NPC testNPC;
extern InteractableManager interactableManager;
extern EnemyManager enemyManager;
extern Combat combat;
extern Texture2D mapTexture;
extern Camera2D camera;
extern Inventory playerInventory;

void initParty(int applyGrowth, Vector2 spawnPosition);
void unloadParty(void);

static int testWorldIncludeStarterItems = 0;

static void clearInventoryContents(void) {
    if (playerInventory.items.head == NULL || playerInventory.items.size == 0) {
        initInventory(&playerInventory);
        return;
    }

    ListNode* current = playerInventory.items.head;
    int iterations = playerInventory.items.size;

    for (int i = 0; i < iterations && current != NULL; i++) {
        ListNode* next = current->next;
        InventoryItem* item = (InventoryItem*)current->data;
        if (item != NULL) {
            free(item);
        }
        current = next;
    }

    clearList(&playerInventory.items);
}

static void getTestWorldBounds(Rectangle* outBounds) {
    if (outBounds == NULL) {
        return;
    }

    *outBounds = (Rectangle){0.0f, 0.0f, (float)mapTexture.width, (float)mapTexture.height};
}

static void addStarterItem(Item* baseItem, int quantity) {
    InventoryItem* item = malloc(sizeof(InventoryItem));
    if (item == NULL) {
        return;
    }

    item->baseItem = baseItem;
    item->quantity = quantity;
    addItemInventory(&playerInventory, item);
}

void SetupTestWorldContent(int includeStarterItems) {
    initParty(0, (Vector2){960.0f, 540.0f});
    initNPC(&testNPC, (Vector2){1400, 700}, "assets/NPCs/npc_placeholder.png", &testeTree, "Test NPC");
    initInventory(&playerInventory);
    initEnemyManager();
    initInteractableManager(&interactableManager);

    Interactable testChest = createChest(
        (Vector2){400, 400},
        "assets/interagiveis/caixa_fechada_placeholder.png",
        "assets/interagiveis/caixa_aberta_placeholder.png"
    );
    addInteractable(&interactableManager, &testChest);

    Interactable testDoor = createDoor(
        (Vector2){0, 0},
        "assets/interagiveis/porta_fechada_placeholder.png",
        "assets/interagiveis/porta_aberta_placeholder.png",
        &doorQuestion1,
        1
    );
    addInteractable(&interactableManager, &testDoor);

    Interactable testTrap = createTrap(
        (Vector2){700, 400},
        "assets/interagiveis/trap_escondida.png",
        "assets/interagiveis/trap_descoberta.png",
        50
    );
    addInteractable(&interactableManager, &testTrap);

    Interactable colorPuzzle = createColorPuzzle(
        (Vector2){500, 700},
        "assets/interagiveis/chao_azul_placeholder.png",
        "assets/interagiveis/chao_verde_placeholder.png",
        "assets/interagiveis/chao_amarelo_placeholdert.png",
        "assets/interagiveis/chao_vermelho_placeholder.png",
        2.0f,
        50
    );
    addInteractable(&interactableManager, &colorPuzzle);

    if (includeStarterItems) {
        addStarterItem(&pimentaMalagueta, 2);
        addStarterItem(&gotaSangreMaldita, 2);
        addStarterItem(&poDeEnvenenar, 2);
        addStarterItem(&pomadaCicatrizante, 2);
        addStarterItem(&antiveneno, 2);
        addStarterItem(&frascoMagicoCura, 2);
    }

    spawnEnemy("inimigoTeste", (Vector2){1400, 0}, "assets/antagonistas/boss1_placeholder.png", MAX_ENEMIES);

    if (enemyManager.count > 0) {
        setEnemyStats(
            &enemyManager.enemies[enemyManager.count - 1],
            100,
            1,
            0,
            10
        );
    }

    if (mapTexture.id != 0) {
        UnloadTexture(mapTexture);
    }
    mapTexture = LoadTexture("assets/cenarios/bg_placeholder.png");

    configureCameraForCurrentWorld();

    testWorldIncludeStarterItems = includeStarterItems;
}

void TeardownTestWorldContent(void) {
    if (mapTexture.id != 0) {
        UnloadTexture(mapTexture);
        mapTexture.id = 0;
    }

    unloadInteractableManager(&interactableManager);
    unloadEnemyManager();
    unloadParty();
    unloadNPC(&testNPC);
    clearInventoryContents();
}

static void setupTestWorldNode(void* userData) {
    int includeStarterItems = 0;

    if (userData != NULL) {
        includeStarterItems = *(int*)userData;
    }

    SetupTestWorldContent(includeStarterItems);
}

static void teardownTestWorldNode(void) {
    TeardownTestWorldContent();
}

void RegisterTestWorld(void) {
    static WorldNode testWorldNode = {
        .name = "Test World",
        .setup = setupTestWorldNode,
        .teardown = teardownTestWorldNode,
        .collectBlockers = NULL,
        .drawOverlay = NULL,
        .getCameraBounds = getTestWorldBounds,
        .userData = &testWorldIncludeStarterItems,
        .prev = NULL,
        .next = NULL
    };

    registerWorldNode(&testWorldNode);
}