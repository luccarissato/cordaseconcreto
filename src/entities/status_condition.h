#ifndef STATUS_CONDITION_H
#define STATUS_CONDITION_H

#include "../utils/list.h"

typedef enum {
    STATUS_NONE = 0,        /* Sem condição - usado como valor padrão */
    
    /* === DEBUFFS (Condições Negativas) === */
    STATUS_POISON,          /* Envenenado: -10% HP max por turno */
    STATUS_BURN,            /* Queimando: -5% HP max por turno, -25% força */
    STATUS_PARALYSIS,       /* Paralisado: 25% chance de perder turno */
    STATUS_SLEEP,           /* Dormindo: perde turnos até acordar ou ser atacado */
    STATUS_CONFUSION,       /* Confuso: 30% chance de atacar aliado ou a si mesmo */
    STATUS_BLIND,           /* Cego: -50% precisão em ataques */
    STATUS_BLEED,           /* Sangramento: dano escalável por turno, removível por cleanse */
    STATUS_WEAKEN,          /* Fraqueza: reduz força */
    STATUS_SLOW,            /* Lentidão: reduz velocidade */
    STATUS_DEFENSE_DOWN,    /* Defesa Down: reduz defesa */
    
    /* === BUFFS (Condições Positivas) === */
    STATUS_REGEN,           /* Regeneração: +10% HP max por turno */
    STATUS_STRENGTH_UP,     /* Força Up: +50% dano físico */
    STATUS_DEFENSE_UP,      /* Defesa Up: +50% defesa */
    STATUS_SPEED_UP,        /* Velocidade Up: +50% velocidade */
    
    STATUS_COUNT            /* Total de tipos de status (usado para iteração) */
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
