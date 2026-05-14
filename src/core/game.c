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
#include "../combat/combat.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include "../data/dialogues/teste_dialogue.h"

#define PARTY_SIZE 4
#define HISTORY_SIZE 1000
#define MAX_ENEMIES 32

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
void spawnEnemy(const char* name, Vector2 position, const char* texturePath);
void updateEnemies(const Rectangle* blockers, int blockerCount);
void drawEnemies();
void getEnemyBlockers(Rectangle* outBlockers, int* outCount);
void unloadEnemyManager();

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


    // teste
    initInteractableManager(&interactableManager);
    
    Interactable testChest = createChest(
        (Vector2){400, 400},
        "assets/interagiveis/caixa_fechada_placeholder.png",
        "assets/interagiveis/caixa_aberta_placeholder.png"
    );
    addInteractable(&interactableManager, &testChest);
    
    InventoryItem* cartolaItem = malloc(sizeof(InventoryItem));
    cartolaItem->baseItem = &cartola;
    cartolaItem->quantity = 3;
    addItemInventory(&playerInventory, cartolaItem);
    
    InventoryItem* tapiocaItem = malloc(sizeof(InventoryItem));
    tapiocaItem->baseItem = &tapiocaRecheada;
    tapiocaItem->quantity = 3;
    addItemInventory(&playerInventory, tapiocaItem);

    InventoryItem* pratoBuchadaItem = malloc(sizeof(InventoryItem));
    pratoBuchadaItem->baseItem = &pratoBuchada;
    pratoBuchadaItem->quantity = 3;
    addItemInventory(&playerInventory, pratoBuchadaItem);

    InventoryItem* pedaço1Item = malloc(sizeof(InventoryItem));
    pedaço1Item->baseItem = &pedaçoDeChave1;
    pedaço1Item->quantity = 1;
    addItemInventory(&playerInventory, pedaço1Item);

    InventoryItem* pedaço2Item = malloc(sizeof(InventoryItem));
    pedaço2Item->baseItem = &pedaçoDeChave2;
    pedaço2Item->quantity = 1;
    addItemInventory(&playerInventory, pedaço2Item);
    
    spawnEnemy("Boss 1", (Vector2){1400, 200}, "assets/antagonistas/boss1_placeholder.png");


    camera.target = party[0].position; // segue o líder
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
            
            /* Detecta combate com Z ou proximidade */
            if (IsKeyPressed(KEY_Z)) {
                startCombat(party[0].position, 999999.0f); /* Z força combate com qualquer inimigo próximo */
            } else {
                startCombat(party[0].position, COMBAT_DETECTION_DISTANCE);
            }
            
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
            /* Aqui será implementada a lógica de combate */
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
            /* Aqui será renderizada a interface de combate */
            BeginMode2D(camera);
            DrawTexture(mapTexture, 0, 0, WHITE);
            drawNPC(&testNPC);
            drawInteractables(&interactableManager);
            drawEnemies();
            drawParty();
            EndMode2D();
            DrawText("COMBAT STATE - TODO", 100, 100, 30, RED);
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
    party[0].stats.defCalor = 6;
    party[0].stats.defVento = 3;
    party[0].stats.defMare = 5;
    party[0].stats.defTerra = 8;

    //manguebeat
    party[1].stats.baseHP = 50;
    party[1].stats.baseMana = 25;
    party[1].stats.fortitude = 6;
    party[1].stats.mente = 4;
    party[1].stats.forca = 7;
    party[1].stats.defesa = 4;
    party[1].stats.velocidade = 7;
    party[1].stats.defCalor = 5;
    party[1].stats.defVento = 8;
    party[1].stats.defMare = 6;
    party[1].stats.defTerra = 3;

    //cirandeira
    party[2].stats.baseHP = 45;
    party[2].stats.baseMana = 35;
    party[2].stats.fortitude = 5;
    party[2].stats.mente = 10;
    party[2].stats.forca = 3;
    party[2].stats.defesa = 4;
    party[2].stats.velocidade = 6;
    party[2].stats.defCalor = 4;
    party[2].stats.defVento = 8;
    party[2].stats.defMare = 7;
    party[2].stats.defTerra = 5;

    //repentista
    party[3].stats.baseHP = 35;
    party[3].stats.baseMana = 40;
    party[3].stats.fortitude = 4;
    party[3].stats.mente = 9;
    party[3].stats.forca = 2;
    party[3].stats.defesa = 3;
    party[3].stats.velocidade = 7;
    party[3].stats.defCalor = 8;
    party[3].stats.defVento = 6;
    party[3].stats.defMare = 4;
    party[3].stats.defTerra = 6;

    for (int i = 0; i < PARTY_SIZE; i++) {
        calculateStats(&party[i].stats);
    }

    // inicializa histórico com posição inicial
    for (int i = 0; i < HISTORY_SIZE; i++) {
        positionHistory[i] = party[0].position;
        directionHistory[i] = party[0].direction;
    }

    // teste
    party[0].stats.currentHP = 30;
    party[0].stats.currentMana = 10;
    party[1].stats.currentHP = 30;
    party[1].stats.currentMana = 10;
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

/* =============================================================================
 * FUNÇÕES DE GERENCIAMENTO DE INIMIGOS
 * ============================================================================= */

/**
 * initEnemyManager - Inicializa o gerenciador de inimigos
 * 
 * Prepara o sistema para gerenciar inimigos no mapa.
 */
void initEnemyManager() {
    enemyManager.count = 0;
    memset(enemyManager.enemies, 0, sizeof(enemyManager.enemies));
}

/**
 * spawnEnemy - Spawna um novo inimigo no mapa
 * 
 * Adiciona um inimigo à lista de inimigos ativos.
 */
void spawnEnemy(const char* name, Vector2 position, const char* texturePath) {
    if (enemyManager.count >= MAX_ENEMIES) {
        return;
    }
    
    initEnemyWithTexture(&enemyManager.enemies[enemyManager.count], name, position, texturePath);
    enemyManager.count++;
}

/**
 * updateEnemies - Atualiza todos os inimigos
 * 
 * Processa IA, status e colisão dos inimigos (futura implementação).
 */
void updateEnemies(const Rectangle* blockers, int blockerCount) {
    for (int i = 0; i < enemyManager.count; i++) {
        Enemy* enemy = &enemyManager.enemies[i];
        
        if (!enemy->isAlive) continue;
        
        /* Processa status effects no inimigo */
        processEnemyStatusEffects(enemy);
        
        /* Aqui será adicionada lógica de IA no futuro */
    }
}

/**
 * drawEnemies - Renderiza todos os inimigos
 * 
 * Desenha cada inimigo ativo no mapa.
 */
void drawEnemies() {
    for (int i = 0; i < enemyManager.count; i++) {
        if (enemyManager.enemies[i].isAlive) {
            drawEnemy(&enemyManager.enemies[i]);
        }
    }
}

/**
 * getEnemyBlockers - Coleta retângulos de colisão de todos os inimigos
 * 
 * Retorna array de retângulos para colisão com jugadores/objetos.
 */
void getEnemyBlockers(Rectangle* outBlockers, int* outCount) {
    if (outBlockers == NULL || outCount == NULL) return;
    
    *outCount = 0;
    
    for (int i = 0; i < enemyManager.count; i++) {
        if (enemyManager.enemies[i].isAlive) {
            outBlockers[*outCount] = getEnemyCollider(&enemyManager.enemies[i]);
            (*outCount)++;
        }
    }
}

/**
 * unloadEnemyManager - Libera recursos do gerenciador de inimigos
 * 
 * Descarrega texturas e libera memória de todos os inimigos.
 */
void unloadEnemyManager() {
    for (int i = 0; i < enemyManager.count; i++) {
        unloadEnemy(&enemyManager.enemies[i]);
    }
    enemyManager.count = 0;
}

/* =============================================================================
 * FUNÇÕES DE COMBATE
 * ============================================================================= */

/**
 * startCombat - Inicia um combate com um inimigo
 * 
 * Detecta proximidade com inimigos e inicia combate se aplicável.
 */
void startCombat(Vector2 playerPos, float combatDistance) {
    /* Se já está em combate, não inicia outro */
    if (combat.inCombat) return;
    
    combat.inCombat = 0;
    combat.enemyCount = 0;
    
    /* Procura inimigos próximos */
    for (int i = 0; i < enemyManager.count; i++) {
        Enemy* enemy = &enemyManager.enemies[i];
        
        if (!enemy->isAlive) continue;
        
        /* Calcula distância entre jogador e inimigo */
        float dx = enemy->position.x - playerPos.x;
        float dy = enemy->position.y - playerPos.y;
        float distance = sqrtf(dx * dx + dy * dy);
        
        /* Se dentro da distância de combate, adiciona ao combate */
        if (distance < combatDistance) {
            combat.enemyIndices[combat.enemyCount] = i;
            combat.enemyCount++;
        }
    }
    
    /* Se encontrou inimigos, inicia combate */
    if (combat.enemyCount > 0) {
        combat.inCombat = 1;
        
        /* Coleta inimigos para passar ao combat system */
        Enemy enemies[MAX_ENEMIES];
        for (int i = 0; i < combat.enemyCount; i++) {
            enemies[i] = enemyManager.enemies[combat.enemyIndices[i]];
        }
        
        /* Inicia combate com sistema de turnos */
        startCombatWithEnemies(party, PARTY_SIZE, enemies, combat.enemyCount);
        
        /* Muda estado do jogo para combate */
        currentGameState = STATE_COMBAT;
    }
}