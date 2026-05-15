#include "combat.h"
#include "../core/game.h"
#include "../core/state.h"
#include "../core/collision.h"
#include "raymath.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>
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

// checkPlayerEnemyCollision - Verifica se o jogador está colidindo com algum inimigo vivo
// Usa distância entre centros (simétrica, funciona de qualquer ângulo)
int checkPlayerEnemyCollision(Player* player) {
    if (player == NULL) return 0;
    
    // Calcula o centro do jogador (assume sprite 300x300)
    Vector2 playerCenter = {
        player->position.x + 150.0f,
        player->position.y + 150.0f
    };
    
    // Distância de interação: igual ao NPC e caixa
    float interactionDistance = 240.0f;
    
    for (int i = 0; i < enemyManager.count; i++) {
        Enemy* enemy = &enemyManager.enemies[i];
        if (!enemy->isAlive) continue;
        
        // Calcula o centro do inimigo (assume sprite 300x300)
        Vector2 enemyCenter = {
            enemy->position.x + 150.0f,
            enemy->position.y + 150.0f
        };
        
        // Verifica distância entre centros (simétrica como NPC e caixa)
        float distance = Vector2Distance(playerCenter, enemyCenter);
        if (distance <= interactionDistance) {
            return 1;
        }
    }
    
    return 0;
}