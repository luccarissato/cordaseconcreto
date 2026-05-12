/**
 * =============================================================================
 * ENEMY.H - Estrutura e Funções de Inimigos
 * =============================================================================
 * 
 * Define a estrutura de inimigos do jogo e funções de combate.
 * Inclui o sistema de condições de status para efeitos durante batalhas.
 * 
 * =============================================================================
 */

#ifndef ENEMY_H
#define ENEMY_H

#include "stats.h"
#include "status_condition.h"

/* -----------------------------------------------------------------------------
 * ESTRUTURA: Enemy
 * -----------------------------------------------------------------------------
 * Representa um inimigo no jogo.
 * 
 * Campos:
 *   - name: Nome do inimigo exibido em combate
 *   - stats: Atributos do inimigo (HP, força, defesa, etc.)
 *   - statusList: Lista de condições de status ativas
 *   - isAlive: Flag indicando se o inimigo está vivo (HP > 0)
 *   - expReward: Experiência concedida ao derrotar
 *   - goldReward: Ouro concedido ao derrotar
 * -------------------------------------------------------------------------- */
typedef struct Enemy {
    char name[32];              /* Nome do inimigo */
    Stats stats;                /* Atributos de combate */
    StatusList statusList;      /* Lista de condições de status ativas */
    int isAlive;                /* 1 se HP > 0, 0 se derrotado */
    int expReward;              /* XP dado ao derrotar */
    int goldReward;             /* Ouro dado ao derrotar */
} Enemy;

/* =============================================================================
 * FUNÇÕES DE INICIALIZAÇÃO E GERENCIAMENTO
 * ============================================================================= */

/**
 * initEnemy - Inicializa um inimigo com valores padrão
 * @param enemy: Ponteiro para o Enemy a ser inicializado
 * @param name: Nome do inimigo
 * 
 * Configura stats básicos e inicializa a lista de status.
 */
void initEnemy(Enemy* enemy, const char* name);

/**
 * freeEnemy - Libera recursos do inimigo
 * @param enemy: Ponteiro para o Enemy a ser liberado
 * 
 * Libera a memória da lista de status.
 */
void freeEnemy(Enemy* enemy);

/**
 * setEnemyStats - Configura os stats de um inimigo
 * @param enemy: Ponteiro para o Enemy
 * @param hp: HP base
 * @param forca: Força do inimigo
 * @param defesa: Defesa do inimigo
 * @param velocidade: Velocidade do inimigo
 */
void setEnemyStats(Enemy* enemy, int hp, int forca, int defesa, int velocidade);

/**
 * setEnemyRewards - Configura as recompensas do inimigo
 * @param enemy: Ponteiro para o Enemy
 * @param exp: Experiência concedida
 * @param gold: Ouro concedido
 */
void setEnemyRewards(Enemy* enemy, int exp, int gold);

/* =============================================================================
 * FUNÇÕES DE COMBATE
 * ============================================================================= */

/**
 * damageEnemy - Aplica dano a um inimigo
 * @param enemy: Ponteiro para o Enemy
 * @param damage: Quantidade de dano a aplicar
 * @return: Dano efetivo aplicado (após defesa)
 * 
 * O dano é reduzido pela defesa do inimigo.
 * Se HP chegar a 0, isAlive é setado para 0.
 */
int damageEnemy(Enemy* enemy, int damage);

/**
 * healEnemy - Cura HP de um inimigo
 * @param enemy: Ponteiro para o Enemy
 * @param amount: Quantidade de HP a restaurar
 */
void healEnemy(Enemy* enemy, int amount);

/**
 * applyStatusToEnemy - Aplica uma condição de status ao inimigo
 * @param enemy: Ponteiro para o Enemy
 * @param type: Tipo de status a aplicar
 * @param turns: Duração em turnos
 * @param intensity: Intensidade do efeito
 */
void applyStatusToEnemy(Enemy* enemy, StatusType type, int turns, float intensity);

/**
 * processEnemyStatusEffects - Processa efeitos de status no turno
 * @param enemy: Ponteiro para o Enemy
 * @return: Dano total causado por condições de status
 * 
 * Aplica dano de veneno, queimadura, regeneração, etc.
 */
int processEnemyStatusEffects(Enemy* enemy);

/**
 * canEnemyAct - Verifica se o inimigo pode agir neste turno
 * @param enemy: Ponteiro para o Enemy
 * @return: 1 se pode agir, 0 se impedido (paralisia, sono)
 */
int canEnemyAct(Enemy* enemy);

/**
 * getEnemyDamageModifier - Obtém o modificador de dano do inimigo
 * @param enemy: Ponteiro para o Enemy
 * @return: Multiplicador de dano baseado em status
 */
float getEnemyDamageModifier(Enemy* enemy);

#endif /* ENEMY_H */
