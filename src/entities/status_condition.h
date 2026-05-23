#ifndef STATUS_CONDITION_H
#define STATUS_CONDITION_H

#include "../utils/list.h"

typedef enum {
    STATUS_NONE = 0,
    
    /* DEBUFFS */
    STATUS_POISON,              /* Envenenado: 12% HP max por turno, removível */
    STATUS_ENSOLACAO,           /* Ensolação: 15% HP max por turno, NÃO removível */
    STATUS_CONFUSION,           /* Confuso: 30% chance atacar aliado */
    STATUS_BLEED,               /* Sangramento: dano escalável, removível */
    STATUS_WEAKEN,              /* Fraqueza: -25% força */
    STATUS_SLOW,                /* Lentidão: -25% velocidade */
    STATUS_DEFENSE_DOWN,        /* Defesa Down: -25% defesa */
    STATUS_ENCHARCADO,          /* Encharcado: +25% dano mágico recebido */
    
    /* BUFFS */
    STATUS_REGEN,               /* Regeneração: +10% HP/turno */
    STATUS_STRENGTH_UP,         /* Força Up: +50% dano */
    STATUS_DEFENSE_UP,          /* Defesa Up: +50% defesa */
    STATUS_SPEED_UP,            /* Velocidade Up: +50% velocidade */
    
    /* NEW STATUSES FOR CORRECTED ABILITIES */
    STATUS_PROTECT,             /* Proteção: Redireciona dano recebido para o Tank */
    STATUS_COUNTER,             /* Contra-ataque: Retalia com 50% do dano e bleed */
    STATUS_REFLECT,             /* Bastião: Reduz dano e reflete como true damage */
    STATUS_ARCANE_MARK,         /* Marca Arcana: Dobra o próximo dano elemental */
    STATUS_ASCENSAO_MAGICA,     /* Ascensão Mágica: +10% dano elemental cumulativo por round */
    STATUS_HASTE,               /* Aceleração: Concede turno extra por round */
    
    STATUS_COUNT
} StatusType;

typedef struct {
    StatusType type;        /* Tipo da condição de status */
    int turnsRemaining;     /* Turnos restantes (-1 = permanente) */
    float intensity;        /* Intensidade do efeito (multiplicador) */
} StatusCondition;

typedef struct {
    List conditions;        /* Lista de StatusCondition* ativos */
} StatusList;

// initStatusList - Inicializa uma lista de status vazia
void initStatusList(StatusList* statusList);

// freeStatusList - Libera toda a memória alocada pela lista de status
void freeStatusList(StatusList* statusList);

// addStatusCondition - Adiciona uma nova condição de status ao personagem
void addStatusCondition(StatusList* statusList, StatusType type, int turns, float intensity);

// removeStatusCondition - Remove uma condição específica do personagem
void removeStatusCondition(StatusList* statusList, StatusType type);

// removeAllDebuffs - Remove todas as condições negativas do personagem
void removeAllDebuffs(StatusList* statusList);

// removeAllBuffs - Remove todas as condições positivas do personagem
void removeAllBuffs(StatusList* statusList);

// clearAllStatus - Remove todas as condições de status do personagem
void clearAllStatus(StatusList* statusList);

// hasStatusCondition - Verifica se o personagem tem uma condição específica
int hasStatusCondition(StatusList* statusList, StatusType type);

// getStatusCondition - Obtém os detalhes de uma condição específica
StatusCondition* getStatusCondition(StatusList* statusList, StatusType type);

// countActiveStatus - Conta quantas condições estão ativas
int countActiveStatus(StatusList* statusList);

// isDebuff - Verifica se um tipo de status é um debuff (condição negativa)
int isDebuff(StatusType type);

// isBuff - Verifica se um tipo de status é um buff (condição positiva)
int isBuff(StatusType type);

// processStatusEffects - Processa os efeitos de todas as condições no turno
int processStatusEffects(StatusList* statusList, int* currentHP, int maxHP);

// updateStatusDurations - Atualiza as durações e remove status expirados
void updateStatusDurations(StatusList* statusList);

// canActThisTurn - Verifica se o personagem pode agir no turno atual
int canActThisTurn(StatusList* statusList);

// getStrengthModifier - Calcula o modificador de força baseado nos status
float getStrengthModifier(StatusList* statusList);

// getDefenseModifier - Calcula o modificador de defesa baseado nos status
float getDefenseModifier(StatusList* statusList);

// getSpeedModifier - Calcula o modificador de velocidade baseado nos status
float getSpeedModifier(StatusList* statusList);

// getAccuracyModifier - Calcula o modificador de precisão baseado nos status
float getAccuracyModifier(StatusList* statusList);

//getStatusName - Retorna o nome de exibição de um tipo de status
const char* getStatusName(StatusType type);


// * getStatusDescription - Retorna a descrição de um tipo de status
const char* getStatusDescription(StatusType type);

#endif
