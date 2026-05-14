#include "combat.h"
#include <string.h>
#include <stdlib.h>
#include "../utils/sort.h"

CombatState combatState;

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
