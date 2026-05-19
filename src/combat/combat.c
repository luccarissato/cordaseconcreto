#include "combat.h"
#include "../core/game.h"
#include "../core/state.h"
#include "../core/collision.h"
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

//compareCombatSpeed - Compara a velocidade dos participantes
static int compareCombatantSpeed(const void* a, const void* b) {
    const Combatant* combA = (const Combatant*)a;
    const Combatant* combB = (const Combatant*)b;
    
    if (combA->speedStat > combB->speedStat) {
        return -1; 
    } else if (combA->speedStat < combB->speedStat) {
        return 1; 
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
    for (int i = 0; i < combat.enemyCount; i++) {
        int enemyIndex = combat.enemyIndices[i];
        if (enemyIndex >= 0 && enemyIndex < enemyManager.count) {
            enemyManager.enemies[enemyIndex].inCombat = 0;
        }
    }

    combat.inCombat = 0;
    combat.enemyCount = 0;
    memset(combat.enemyIndices, 0, sizeof(combat.enemyIndices));
    endCombatBattle();
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