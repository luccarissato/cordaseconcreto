#ifndef COMBAT_H
#define COMBAT_H

#include "../entities/player.h"
#include "../entities/enemy.h"

#define MAX_COMBATANTS 8

typedef enum {
    COMBATANT_PLAYER,
    COMBATANT_ENEMY
} CombatantType;

typedef struct {
    CombatantType type;
    int playerIndex;
    int enemyIndex;
    int speedStat;
} Combatant;

typedef struct {
    int inCombat;
    Combatant combatants[MAX_COMBATANTS];
    int combatantCount;
    int currentTurn;
} CombatState;

extern CombatState combatState;

 // initCombat - Inicializa a estrutura de combate
void initCombat();

// startCombatWithEnemies - Inicia um combate com inimigos específicos
void startCombatWithEnemies(Player* players, int playerCount, Enemy* enemies, int enemyCount);

// calculateTurnOrder - Calcula a ordem de turnos baseada em velocidade
void calculateTurnOrder();

// getCurrentCombatant - Obtém o combatente de turno atual
Combatant* getCurrentCombatant();

// nextTurn - Avança para o próximo turno
void nextTurn();

// isCombatActive - Verifica se há combate ativo
int isCombatActive();

// endCombatBattle - Finaliza o combate
void endCombatBattle();

// checkPlayerEnemyCollision - Verifica se o jogador está colidindo com algum inimigo vivo
// Retorna 1 se há colisão, 0 caso contrário
int checkPlayerEnemyCollision(Player* player);

#endif
