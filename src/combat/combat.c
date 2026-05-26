#include "combat.h"
#include "../core/game.h"
#include "../core/state.h"
#include "../core/collision.h"
#include "../entities/status_condition.h"
#include "../items/inventory.h"
#include "../items/game_items.h"
#include "boss_ai.h"
#include "raymath.h"
#include <string.h>
#include <stdlib.h>
#include "../utils/sort.h"

CombatState combatState;

/* Variáveis globais de game.c que precisam estar acessíveis */
extern Player party[PARTY_SIZE];
extern EnemyManager enemyManager;
extern Combat combat;
extern GameState currentGameState;
extern Inventory playerInventory;

static int inventoryHasItem(Item* item) {
    if (item == NULL) return 0;

    ListNode* node = playerInventory.items.head;
    int count = playerInventory.items.size;

    for (int i = 0; i < count && node != NULL; i++) {
        InventoryItem* invItem = (InventoryItem*)node->data;
        if (invItem != NULL && invItem->baseItem == item) {
            return 1;
        }
        node = node->next;
    }

    return 0;
}

static void grantKeyItem(Item* item) {
    if (item == NULL) return;
    if (inventoryHasItem(item)) return;
    
    InventoryItem* newItem = malloc(sizeof(InventoryItem));
    if (newItem == NULL) return;

    newItem->baseItem = item;
    newItem->quantity = 1;
    addItemInventory(&playerInventory, newItem);
}

static void clearCombatStatusState(void) {
    for (int i = 0; i < PARTY_SIZE; i++) {
        clearAllStatus(&party[i].statusList);
        party[i].defenseGuardActive = 0;
        party[i].defenseDamageReductionPending = 0;
        party[i].extraTurnsPending = 0;
    }

    for (int i = 0; i < enemyManager.count; i++) {
        clearAllStatus(&enemyManager.enemies[i].statusList);
        enemyManager.enemies[i].inCombat = 0;
    }
}

static void restorePartyMinimumCombatHp(void) {
    for (int i = 0; i < PARTY_SIZE; i++) {
        if (party[i].stats.currentHP <= 0) {
            party[i].stats.currentHP = 1;
            party[i].isAlive = 1;
        }
    }
}

static int isBoss1EnemyName(const char* name) {
    return name != NULL &&
        (strcmp(name, "Boss 1") == 0 ||
         strcmp(name, "Mulher do Guarda Chuva Branco") == 0);
}

static int isBoss2EnemyName(const char* name) {
    return name != NULL &&
        (strcmp(name, "Boss 2") == 0 ||
         strcmp(name, "Papa figo") == 0);
}

//compareCombatSpeed - Compara a velocidade dos participantes
static int compareCombatantSpeed(const void* a, const void* b) {
    const Combatant* combA = (const Combatant*)a;
    const Combatant* combB = (const Combatant*)b;
    
    if (combA->speedStat > combB->speedStat) {
        return 1; 
    } else if (combA->speedStat < combB->speedStat) {
        return -1; 
    } else {
        return 0; 
    }
}

// initCombat - inicia o sistema de combate
void initCombat() {
    memset(&combatState, 0, sizeof(CombatState));
    combatState.inCombat = 0;
    combatState.combatantCount = 0;
    combatState.currentTurn = 0;
}

// startCombatWithEnemies - inicializa o combate
void startCombatWithEnemies(Player* players, int playerCount, Enemy* enemies, int enemyCount) {
    if (players == NULL || enemies == NULL) return;
    if (playerCount <= 0 || enemyCount <= 0) return;
    
    combatState.combatantCount = 0;
    combatState.currentTurn = 0;
    
    for (int i = 0; i < playerCount && combatState.combatantCount < MAX_COMBATANTS; i++) {
        combatState.combatants[combatState.combatantCount].type = COMBATANT_PLAYER;
        combatState.combatants[combatState.combatantCount].playerIndex = i;
        combatState.combatants[combatState.combatantCount].speedStat = players[i].stats.velocidade;
        combatState.combatantCount++;
    }
    
    for (int i = 0; i < enemyCount && combatState.combatantCount < MAX_COMBATANTS; i++) {
        combatState.combatants[combatState.combatantCount].type = COMBATANT_ENEMY;
        combatState.combatants[combatState.combatantCount].enemyIndex = i;
        combatState.combatants[combatState.combatantCount].speedStat = enemies[i].stats.velocidade;
        combatState.combatantCount++;
    }
    
    for (int i = 0; i < enemyCount; i++) {
        enemies[i].inCombat = 1;
    }
    
    calculateTurnOrder();

    for (int i = 0; i < enemyCount && combatState.combatantCount < MAX_COMBATANTS; i++) {
        if (isBoss2EnemyName(enemies[i].name)) {
            combatState.combatants[combatState.combatantCount].type = COMBATANT_ENEMY;
            combatState.combatants[combatState.combatantCount].enemyIndex = i;
            combatState.combatants[combatState.combatantCount].speedStat = -999;
            combatState.combatantCount++;
            break;
        }
    }
    
    combatState.inCombat = 1;
}

// calculateTurnOrder - determina a ordem da iniciativa
void calculateTurnOrder() {
    mergeSort(
        combatState.combatants,
        combatState.combatantCount,
        sizeof(Combatant),
        compareCombatantSpeed
    );
}

// getCurrentCombatant - Obtém o combatente de turno atual
Combatant* getCurrentCombatant() {
    if (!combatState.inCombat || combatState.currentTurn >= combatState.combatantCount) {
        return NULL;
    }
    
    return &combatState.combatants[combatState.currentTurn];
}

// nextTurn - Avança para o próximo turno
void nextTurn() {
    if (!combatState.inCombat) return;
    
    combatState.currentTurn++;
    
    if (combatState.currentTurn >= combatState.combatantCount) {
        combatState.currentTurn = 0;
    }
}

// isCombatActive - Verifica se há combate ativo
int isCombatActive() {
    return combatState.inCombat;
}

// endCombatBattle - Finaliza o combate
void endCombatBattle() {
    combatState.inCombat = 0;
    combatState.combatantCount = 0;
    combatState.currentTurn = 0;
}

void startCombat(Vector2 playerPos, float combatDistance) {
    if (combat.inCombat) return;
    
    combat.inCombat = 0;
    combat.enemyCount = 0;
    
    Collider playerCollider = {
        .offset = {75.0f, 0.0f},
        .size = {150.0f, 300.0f}
    };
    
    for (int i = 0; i < enemyManager.count; i++) {
        Enemy* enemy = &enemyManager.enemies[i];
        
        if (!enemy->isAlive) continue;
        

        int isNear = areCollidersNearEdgeBased(
            playerPos, playerCollider,
            enemy->position, enemy->collider,
            combatDistance
        );
        
        if (isNear) {
            combat.enemyIndices[combat.enemyCount] = i;
            combat.enemyCount++;
        }
    }
    
    if (combat.enemyCount > 0) {
        combat.inCombat = 1;
        
        Enemy enemies[MAX_ENEMIES];
        for (int i = 0; i < combat.enemyCount; i++) {
            enemies[i] = enemyManager.enemies[combat.enemyIndices[i]];
        }
        
        startCombatWithEnemies(party, PARTY_SIZE, enemies, combat.enemyCount);
        
        currentGameState = STATE_COMBAT;
    }
}

int getCombatState() {
    return combat.inCombat;
}

Enemy** getEnemiesInCombat() {
    static Enemy* combatEnemies[MAX_ENEMIES];

    for (int i = 0; i < combat.enemyCount; i++) {
        int enemyIndex = combat.enemyIndices[i];
        if (enemyIndex >= 0 && enemyIndex < enemyManager.count) {
            combatEnemies[i] = &enemyManager.enemies[enemyIndex];
        } else {
            combatEnemies[i] = NULL;
        }
    }

    return combatEnemies;
}

int getEnemyCombatCount() {
    return combat.enemyCount;
}

void endCombat() {
    if (!combat.inCombat) return;

    /* Verifica se foi vitória (todos inimigos derrotados) */
    int aliveEnemies = 0;
    int alivePlayers = 0;
    int defeatedBoss1 = 0;
    int defeatedBoss2 = 0;
    
    for (int i = 0; i < PARTY_SIZE; i++) {
        if (party[i].isAlive) {
            alivePlayers++;
        }
    }

    for (int i = 0; i < combat.enemyCount; i++) {
        int enemyIndex = combat.enemyIndices[i];
        if (enemyIndex >= 0 && enemyIndex < enemyManager.count) {
            Enemy* enemy = &enemyManager.enemies[enemyIndex];

            if (enemy->isAlive) {
                aliveEnemies++;
            } else {
                if (isBoss1EnemyName(enemy->name)) {
                    defeatedBoss1 = 1;
                }
                if (isBoss2EnemyName(enemy->name)) {
                    defeatedBoss2 = 1;
                }
            }
            /* Remove inimigos do combate */
            enemy->inCombat = 0;
        }
    }
    
    int partyDefeated = (alivePlayers == 0);

    /* Se nenhum inimigo vivo = jogadores venceram */
    if (!partyDefeated && aliveEnemies == 0) {
        if (defeatedBoss1) {
            grantKeyItem(&pedaçoDeChave1);
        }

        if (defeatedBoss2) {
            grantKeyItem(&pedaçoDeChave2);
        }

        /* Aplica level up a todos os jogadores, mesmo os derrotados */
        for (int i = 0; i < PARTY_SIZE; i++) {
            playerLevelUp(&party[i]);
        }
    }

    bossAiOnCombatEnd();
    clearCombatStatusState();

    if (partyDefeated) {
        combat.inCombat = 0;
        combat.enemyCount = 0;
        memset(combat.enemyIndices, 0, sizeof(combat.enemyIndices));
        endCombatBattle();
        resetGameState();
        return;
    }

     /* Garantir que qualquer personagem que terminou o combate com 0 HP volte para 1 HP
         antes de qualquer teardown/transition (previne crashes durante unload). */
     restorePartyMinimumCombatHp();

     /* Recalcula quantos jogadores estão vivos depois da restauração para decidir
         corretamente se devemos resetar o jogo (wipe) ou voltar à exploração. */
     alivePlayers = 0;
     for (int i = 0; i < PARTY_SIZE; i++) {
          if (party[i].isAlive) {
                alivePlayers++;
          }
     }

    combat.inCombat = 0;
    combat.enemyCount = 0;
    memset(combat.enemyIndices, 0, sizeof(combat.enemyIndices));
    endCombatBattle();

    if (alivePlayers == 0) {
        resetGameState();
        return;
    }

    currentGameState = STATE_EXPLORATION;
}

// checkPlayerEnemyCollision - Verifica se o jogador está próximo de algum inimigo vivo
int checkPlayerEnemyCollision(Player* player) {
    if (player == NULL) return 0;
    
    float interactionDistance = 80.0f;
    
    for (int i = 0; i < enemyManager.count; i++) {
        Enemy* enemy = &enemyManager.enemies[i];
        if (!enemy->isAlive) continue;
        
        /* Usa o sistema de distância EDGE-TO-EDGE */
        int isNear = areCollidersNearEdgeBased(
            player->position, player->collider,
            enemy->position, enemy->collider,
            interactionDistance
        );
        
        if (isNear) {
            return 1;
        }
    }
    
    return 0;
}
