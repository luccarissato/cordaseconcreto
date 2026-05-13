/**
 * =============================================================================
 * ENEMY.C - Implementação das Funções de Inimigos
 * =============================================================================
 * 
 * Implementa todas as funções relacionadas a inimigos, incluindo
 * inicialização, combate e gerenciamento de condições de status.
 * 
 * =============================================================================
 */

#include "enemy.h"
#include <string.h>
#include <stdlib.h>

/* =============================================================================
 * FUNÇÕES DE INICIALIZAÇÃO E GERENCIAMENTO
 * ============================================================================= */

/**
 * initEnemy - Inicializa um inimigo com valores padrão
 * 
 * Configura o nome, stats básicos e inicializa a lista de status vazia.
 */
void initEnemy(Enemy* enemy, const char* name) {
    if (enemy == NULL) return;
    
    /* Copia o nome (limitado a 31 caracteres + null terminator) */
    strncpy(enemy->name, name, 31);
    enemy->name[31] = '\0';
    
    /* Inicializa stats com valores padrão */
    enemy->stats.baseHP = 50;
    enemy->stats.maxHP = 50;
    enemy->stats.currentHP = 50;
    enemy->stats.baseMana = 20;
    enemy->stats.maxMana = 20;
    enemy->stats.currentMana = 20;
    enemy->stats.fortitude = 5;
    enemy->stats.mente = 5;
    enemy->stats.forca = 5;
    enemy->stats.defesa = 5;
    enemy->stats.velocidade = 5;
    enemy->stats.defCalor = 0;
    enemy->stats.defVento = 0;
    enemy->stats.defMare = 0;
    enemy->stats.defTerra = 0;
    
    /* Inicializa a lista de condições de status */
    initStatusList(&enemy->statusList);
    
    /* Inimigo começa vivo */
    enemy->isAlive = 1;
    
    /* Recompensas padrão */
    enemy->expReward = 10;
    enemy->goldReward = 5;
}

/**
 * freeEnemy - Libera recursos do inimigo
 * 
 * Libera a memória alocada pela lista de status.
 */
void freeEnemy(Enemy* enemy) {
    if (enemy == NULL) return;
    
    freeStatusList(&enemy->statusList);
}

/**
 * setEnemyStats - Configura os stats de um inimigo
 * 
 * Define os atributos principais de combate do inimigo.
 */
void setEnemyStats(Enemy* enemy, int hp, int forca, int defesa, int velocidade) {
    if (enemy == NULL) return;
    
    enemy->stats.baseHP = hp;
    enemy->stats.maxHP = hp;
    enemy->stats.currentHP = hp;
    enemy->stats.forca = forca;
    enemy->stats.defesa = defesa;
    enemy->stats.velocidade = velocidade;
}

/**
 * setEnemyRewards - Configura as recompensas do inimigo
 * 
 * Define experiência e ouro dados ao derrotar o inimigo.
 */
void setEnemyRewards(Enemy* enemy, int exp, int gold) {
    if (enemy == NULL) return;
    
    enemy->expReward = exp;
    enemy->goldReward = gold;
}

/* =============================================================================
 * FUNÇÕES DE COMBATE
 * ============================================================================= */

/**
 * damageEnemy - Aplica dano a um inimigo
 * 
 * Calcula o dano efetivo considerando a defesa do inimigo.
 * O dano mínimo é sempre 1 (para garantir progresso).
 * Se o HP chegar a 0 ou menos, marca o inimigo como derrotado.
 */
int damageEnemy(Enemy* enemy, int damage) {
    if (enemy == NULL || !enemy->isAlive) return 0;
    
    /* Obtém modificador de defesa baseado em status */
    float defenseModifier = getDefenseModifier(&enemy->statusList);
    
    /* Calcula dano efetivo (dano - defesa modificada) */
    int effectiveDefense = (int)(enemy->stats.defesa * defenseModifier);
    int effectiveDamage = damage - effectiveDefense;
    
    /* Dano mínimo de 1 */
    if (effectiveDamage < 1) effectiveDamage = 1;
    
    /* Aplica o dano */
    enemy->stats.currentHP -= effectiveDamage;
    
    /* Verifica se o inimigo foi derrotado */
    if (enemy->stats.currentHP <= 0) {
        enemy->stats.currentHP = 0;
        enemy->isAlive = 0;
    }
    
    return effectiveDamage;
}

/**
 * healEnemy - Cura HP de um inimigo
 * 
 * Restaura HP, limitando ao máximo.
 */
void healEnemy(Enemy* enemy, int amount) {
    if (enemy == NULL || !enemy->isAlive) return;
    
    enemy->stats.currentHP += amount;
    
    if (enemy->stats.currentHP > enemy->stats.maxHP) {
        enemy->stats.currentHP = enemy->stats.maxHP;
    }
}

/**
 * applyStatusToEnemy - Aplica uma condição de status ao inimigo
 * 
 * Adiciona uma condição de status à lista do inimigo.
 */
void applyStatusToEnemy(Enemy* enemy, StatusType type, int turns, float intensity) {
    if (enemy == NULL || !enemy->isAlive) return;
    
    addStatusCondition(&enemy->statusList, type, turns, intensity);
}

/**
 * processEnemyStatusEffects - Processa efeitos de status no turno
 * 
 * Aplica os efeitos de todas as condições de status ativas:
 * - Veneno: causa dano
 * - Queimadura: causa dano
 * - Regeneração: cura HP
 * 
 * Também atualiza as durações das condições.
 */
int processEnemyStatusEffects(Enemy* enemy) {
    if (enemy == NULL || !enemy->isAlive) return 0;
    
    /* Processa os efeitos e obtém o dano total */
    int damage = processStatusEffects(
        &enemy->statusList,
        &enemy->stats.currentHP,
        enemy->stats.maxHP
    );
    
    /* Verifica se o inimigo foi derrotado por dano de status */
    if (enemy->stats.currentHP <= 0) {
        enemy->stats.currentHP = 0;
        enemy->isAlive = 0;
    }
    
    /* Atualiza as durações das condições */
    updateStatusDurations(&enemy->statusList);
    
    return damage;
}

/**
 * canEnemyAct - Verifica se o inimigo pode agir neste turno
 * 
 * Considera condições como sono e paralisia que podem impedir ação.
 */
int canEnemyAct(Enemy* enemy) {
    if (enemy == NULL || !enemy->isAlive) return 0;
    
    return canActThisTurn(&enemy->statusList);
}

/**
 * getEnemyDamageModifier - Obtém o modificador de dano do inimigo
 * 
 * Retorna o multiplicador de dano baseado em condições de status
 * como força aumentada ou queimadura.
 */
float getEnemyDamageModifier(Enemy* enemy) {
    if (enemy == NULL) return 1.0f;
    
    return getStrengthModifier(&enemy->statusList);
}
