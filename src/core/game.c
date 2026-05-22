#include "raylib.h"
#include "state.h"
#include "game.h"
#include "dialogue.h"
#include "collision.h"
#include "../items/game_items.h"
#include "../items/inventory.h"
#include "../ui/menu.h"
#include "../entities/player.h"
#include "../entities/npc.h"
#include "../entities/enemy.h"
#include "../ui/game_menu.h"
#include "../interactables/interactable.h"
#include "../interactables/chest.h"
#include "../interactables/color_puzzle.h"
#include "../interactables/door.h"
#include "../interactables/trap.h"
#include "../data/dialogues/door_questions.h"
#include "../combat/combat.h"
#include "../combat/ability.h"
#include "../ui/combat_ui.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <stdio.h>

#include "../data/dialogues/teste_dialogue.h"

extern Item gotaSangreMaldita;
extern Item frascoMagicoCura;

#define HISTORY_SIZE 1000

NPC testNPC;
InteractableManager interactableManager;
EnemyManager enemyManager;
Combat combat;

Vector2 positionHistory[HISTORY_SIZE];
Direction directionHistory[HISTORY_SIZE];
int historyIndex = 0;
Texture2D mapTexture;
Camera2D camera;

Player party[PARTY_SIZE];
Inventory playerInventory;

void initParty();
void updateParty(const Rectangle* blockers, int blockerCount);
void drawParty();
void unloadParty();


void initEnemyManager();
void spawnEnemy(const char* name, Vector2 position, const char* texturePath, int maxEnemies);
void updateEnemies(const Rectangle* blockers, int blockerCount);
void drawEnemies();
void getEnemyBlockers(Rectangle* outBlockers, int* outCount);
void unloadEnemyManager();

void startCombat(Vector2 playerPos, float combatDistance);
int getCombatState();
Enemy** getEnemiesInCombat();
int getEnemyCombatCount();
void endCombat();

void initGame() {
    InitWindow(1920, 1080, "Cordas & Concreto");
    SetTargetFPS(60);
    srand((unsigned int) time(NULL));  /* Seed para random number generator */

    initNPC(&testNPC, (Vector2){1400, 700}, "assets/NPCs/npc_placeholder.png", &testeTree);
    initMenu();
    initParty();
    initGameMenu();
    initDialogue();
    initInventory(&playerInventory);
    initEnemyManager();
    initCombat();
    initCombatUI();


    // teste
    initInteractableManager(&interactableManager);
    
    Interactable testChest = createChest(
        (Vector2){400, 400},
        "assets/interagiveis/caixa_fechada_placeholder.png",
        "assets/interagiveis/caixa_aberta_placeholder.png"
    );
    addInteractable(&interactableManager, &testChest);
    
    /* Cria porta teste com pergunta */
    Interactable testDoor = createDoor(
        (Vector2){0, 0},
        "assets/interagiveis/porta_fechada_placeholder.png",
        "assets/interagiveis/porta_aberta_placeholder.png",
        &doorQuestion1,      /* Pergunta: "Quanto é 2 + 2?" */
        1                    /* Node 1 = resposta correta */
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
    
    /* Itens de teste para status effects */
    InventoryItem* pimentaMalagItem = malloc(sizeof(InventoryItem));
    pimentaMalagItem->baseItem = &pimentaMalagueta;
    pimentaMalagItem->quantity = 2;
    addItemInventory(&playerInventory, pimentaMalagItem);
    
    InventoryItem* gotaSangueItem = malloc(sizeof(InventoryItem));
    gotaSangueItem->baseItem = &gotaSangreMaldita;
    gotaSangueItem->quantity = 2;
    addItemInventory(&playerInventory, gotaSangueItem);
    
    InventoryItem* poEnvenenItem = malloc(sizeof(InventoryItem));
    poEnvenenItem->baseItem = &poDeEnvenenar;
    poEnvenenItem->quantity = 2;
    addItemInventory(&playerInventory, poEnvenenItem);
    
    InventoryItem* pomadaItem = malloc(sizeof(InventoryItem));
    pomadaItem->baseItem = &pomadaCicatrizante;
    pomadaItem->quantity = 2;
    addItemInventory(&playerInventory, pomadaItem);
    
    InventoryItem* antivenItem = malloc(sizeof(InventoryItem));
    antivenItem->baseItem = &antiveneno;
    antivenItem->quantity = 2;
    addItemInventory(&playerInventory, antivenItem);
    
    InventoryItem* frascoMagicoItem = malloc(sizeof(InventoryItem));
    frascoMagicoItem->baseItem = &frascoMagicoCura;
    frascoMagicoItem->quantity = 2;
    addItemInventory(&playerInventory, frascoMagicoItem);
    
    spawnEnemy("Boss 1", (Vector2){1400, 0}, "assets/antagonistas/boss1_placeholder.png", 32);
    
    /* Configuração do Boss 1: 1000 HP, defesa física baixa, defesa elemental alta */
    if (enemyManager.count > 0) {
        setEnemyStats(
            &enemyManager.enemies[enemyManager.count - 1],
            1000,  /* HP */
            32,    /* Força */
            3,     /* Defesa física baixa */
            0      /* Velocidade mínima para agir no fim da iniciativa */
        );

        setEnemyElementalResistances(
            &enemyManager.enemies[enemyManager.count - 1],
            85,   /* Calor */
            80,   /* Vento */
            90,   /* Maré */
            75    /* Terra */
        );
    }


    camera.target = party[0].position; /* segue o líder */
    camera.offset = (Vector2){800, 540};
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    mapTexture = LoadTexture("assets/cenarios/bg_placeholder.png");
}

void updateGame() {
    switch (currentGameState) {
        case STATE_MENU:
            updateMenu();
            break;

        case STATE_EXPLORATION:
            if (IsKeyPressed(KEY_X)) {
                openGameMenu();
            }

            /* Coleta blockers de NPC, interagíveis e inimigos */
            int interactableBlockerCount = 0;
            Rectangle* interactableBlockers = getInteractableBlockers(
                &interactableManager, 
                &interactableBlockerCount
            );
            
            /* Coleta blockers de inimigos */
            Rectangle enemyBlockers[MAX_ENEMIES];
            int enemyBlockerCount = 0;
            getEnemyBlockers(enemyBlockers, &enemyBlockerCount);
            
            /* Monta array com todos os blockers (NPC + interactables + enemies) */
            int totalBlockerCount = 1 + interactableBlockerCount + enemyBlockerCount;
            Rectangle* allBlockers = malloc(sizeof(Rectangle) * totalBlockerCount);
            allBlockers[0] = getColliderRect(testNPC.position, testNPC.collider);
            for (int i = 0; i < interactableBlockerCount; i++) {
                allBlockers[1 + i] = interactableBlockers[i];
            }
            for (int i = 0; i < enemyBlockerCount; i++) {
                allBlockers[1 + interactableBlockerCount + i] = enemyBlockers[i];
            }
            
            updateParty(allBlockers, totalBlockerCount);
            updateEnemies(allBlockers, totalBlockerCount);
            
            free(allBlockers);
            free(interactableBlockers);
            
            camera.target = party[0].position;
            updateNPC(&testNPC, party[0].position);
            updateInteractables(&interactableManager, party[0].position);
            
            updateEnemiesInteraction(party[0].position);
            
            break;

        case STATE_DIALOGUE:
            updateDialogue();
            break;

        case STATE_GAME_MENU:
            updateGameMenu();
            if (IsKeyPressed(KEY_X)) {
                closeGameMenu();
            }
            break;
        
        case STATE_COMBAT:
            updateCombatUI();
            break;
        }
}

void drawGame() {
    BeginDrawing();
    ClearBackground(BLACK);

    switch (currentGameState) {
        case STATE_MENU:
            drawMenu();
            break;

        case STATE_EXPLORATION:
            BeginMode2D(camera);
            DrawTexture(mapTexture, 0, 0, WHITE);
            drawNPC(&testNPC);
            drawInteractables(&interactableManager);
            drawEnemies();
            drawParty();
            EndMode2D();
            break;
        
        case STATE_DIALOGUE:
            BeginMode2D(camera);
            DrawTexture(mapTexture, 0, 0, WHITE);
            drawNPC(&testNPC);
            drawInteractables(&interactableManager);
            drawEnemies();
            drawParty();
            EndMode2D();
            drawDialogue();
            break;
        
        case STATE_COMBAT:
            drawCombatUI();
            break;
        
        case STATE_GAME_MENU:
            BeginMode2D(camera);
            DrawTexture(mapTexture, 0, 0, WHITE);
            drawNPC(&testNPC);
            drawInteractables(&interactableManager);
            drawEnemies();
            drawParty();
            EndMode2D();
            drawGameMenu();
            break;
        }

    EndDrawing();
}

void closeGame() {
    unloadNPC(&testNPC);
    unloadMenu();
    closeDialogue();
    unloadParty();
    unloadEnemyManager();
    unloadCombatUI();
    UnloadTexture(mapTexture);
    unloadGameMenu();
    unloadInteractableManager(&interactableManager);
    CloseWindow();
}

void initParty() {
    initPlayer(&party[0], "p1", (Vector2){960, 540}, "1");
    initPlayer(&party[1], "p2", (Vector2){960, 540}, "2");
    initPlayer(&party[2], "p3", (Vector2){960, 540}, "3");
    initPlayer(&party[3], "p4", (Vector2){960, 540}, "4");

    //maracatu
    party[0].stats.baseHP = 60;
    party[0].stats.baseMana = 25;
    party[0].stats.fortitude = 8;
    party[0].stats.mente = 5;
    party[0].stats.forca = 6;
    party[0].stats.defesa = 7;
    party[0].stats.velocidade = 2;
    party[0].stats.defCalor = 60;
    party[0].stats.defVento = 30;
    party[0].stats.defMare = 50;
    party[0].stats.defTerra = 80;

    //manguebeat
    party[1].stats.baseHP = 50;
    party[1].stats.baseMana = 25;
    party[1].stats.fortitude = 6;
    party[1].stats.mente = 4;
    party[1].stats.forca = 7;
    party[1].stats.defesa = 4;
    party[1].stats.velocidade = 7;
    party[1].stats.defCalor = 50;
    party[1].stats.defVento = 80;
    party[1].stats.defMare = 60;
    party[1].stats.defTerra = 30;

    //cirandeira
    party[2].stats.baseHP = 45;
    party[2].stats.baseMana = 35;
    party[2].stats.fortitude = 5;
    party[2].stats.mente = 10;
    party[2].stats.forca = 3;
    party[2].stats.defesa = 4;
    party[2].stats.velocidade = 6;
    party[2].stats.defCalor = 40;
    party[2].stats.defVento = 80;
    party[2].stats.defMare = 70;
    party[2].stats.defTerra = 50;

    //repentista
    party[3].stats.baseHP = 35;
    party[3].stats.baseMana = 40;
    party[3].stats.fortitude = 4;
    party[3].stats.mente = 9;
    party[3].stats.forca = 2;
    party[3].stats.defesa = 3;
    party[3].stats.velocidade = 7;
    party[3].stats.defCalor = 80;
    party[3].stats.defVento = 60;
    party[3].stats.defMare = 40;
    party[3].stats.defTerra = 60;

    for (int i = 0; i < PARTY_SIZE; i++) {
        calculateStats(&party[i].stats);
        /* Inicializa o tipo de personagem (Tank, DPS, Healer, Mage) */
        party[i].characterID = i;
        /* Personagem começa nível 1 */
        party[i].level = 1;
        
        /* Aplica growth para atingir nível 4 (3 level ups: 1->2->3->4) */
        for (int levelUp = 0; levelUp < 3; levelUp++) {
            applyLevelGrowth(&party[i].stats);
            party[i].level++;
        }
        
        /* Recalcula stats derivados com o nível final */
        calculateStats(&party[i].stats);
    }

    // inicializa histórico com posição inicial
    for (int i = 0; i < HISTORY_SIZE; i++) {
        positionHistory[i] = party[0].position;
        directionHistory[i] = party[0].direction;
    }
}

void updateParty(const Rectangle* blockers, int blockerCount) {
    // guarda posição anterior do líder
    Vector2 oldLeaderPos = party[0].position;

    // atualiza líder (input + movimento + animação)
    updatePlayer(&party[0], blockers, blockerCount);

    // verifica se o líder se moveu
    int leaderMoved = (oldLeaderPos.x != party[0].position.x || 
                       oldLeaderPos.y != party[0].position.y);

    // salva no histórico apenas se moveu
    if (leaderMoved) {
        positionHistory[historyIndex] = party[0].position;
        directionHistory[historyIndex] = party[0].direction;

        historyIndex = (historyIndex + 1) % HISTORY_SIZE;
    }

    // delay base entre personagens
    int delay = 30;

    // followers
    for (int i = 1; i < PARTY_SIZE; i++) {
        int index = historyIndex - (i * delay);

        while (index < 0) index += HISTORY_SIZE;

        Vector2 targetPos = positionHistory[index];
        Direction targetDir = directionHistory[index];

        // verifica se esse personagem se moveu
        int isMoving = (targetPos.x != party[i].position.x || 
                        targetPos.y != party[i].position.y);

        // aplica posição e direção
        party[i].position = targetPos;
        party[i].direction = targetDir;

        // animação dos followers
        updatePlayerAnimation(&party[i], isMoving);
    }
}

void drawParty() {
    for (int i = 0; i < PARTY_SIZE; i++) {
        drawPlayer(&party[i]);
    }
}

void unloadParty() {
    for (int i = 0; i < PARTY_SIZE; i++) {
        unloadPlayer(&party[i]);
    }
}

Player* getPartyMembers(int* outCount) {
    if (outCount != NULL) {
        *outCount = PARTY_SIZE;
    }

    return party;
}