/**
 * =============================================================================
 * STATUS_CONDITION.H - Sistema de Condições de Status
 * =============================================================================
 * 
 * Este arquivo define o sistema de condições de status do jogo.
 * Condições de status são efeitos temporários que afetam personagens durante
 * o combate, podendo ser positivos (buffs) ou negativos (debuffs).
 * 
 * TIPOS DE CONDIÇÕES:
 * - STATUS_NONE: Sem condição ativa
 * - STATUS_POISON: Envenenado - causa dano por turno
 * - STATUS_BURN: Queimando - causa dano por turno e reduz ataque
 * - STATUS_PARALYSIS: Paralisado - pode impedir ação no turno
 * - STATUS_SLEEP: Dormindo - impede ações até acordar
 * - STATUS_CONFUSION: Confuso - pode atacar a si mesmo
 * - STATUS_BLIND: Cego - reduz precisão dos ataques
 * - STATUS_REGEN: Regeneração - recupera HP por turno (buff)
 * - STATUS_STRENGTH_UP: Força aumentada - aumenta dano físico (buff)
 * - STATUS_DEFENSE_UP: Defesa aumentada - reduz dano recebido (buff)
 * - STATUS_SPEED_UP: Velocidade aumentada - aumenta velocidade (buff)
 * 
 * A estrutura utiliza uma Lista Encadeada (List) para armazenar múltiplas
 * condições de status ativas em um personagem, permitindo que várias
 * condições coexistam simultaneamente.
 * 
 * =============================================================================
 */

#ifndef STATUS_CONDITION_H
#define STATUS_CONDITION_H

#include "../utils/list.h"

/* -----------------------------------------------------------------------------
 * ENUMERAÇÃO: StatusType
 * -----------------------------------------------------------------------------
 * Define todos os tipos possíveis de condições de status no jogo.
 * Valores negativos (debuffs) e positivos (buffs) são diferenciados
 * para facilitar a lógica de cura e remoção.
 * -------------------------------------------------------------------------- */
typedef enum {
    STATUS_NONE = 0,        /* Sem condição - usado como valor padrão */
    
    /* === DEBUFFS (Condições Negativas) === */
    STATUS_POISON,          /* Envenenado: -10% HP max por turno */
    STATUS_BURN,            /* Queimando: -5% HP max por turno, -25% força */
    STATUS_PARALYSIS,       /* Paralisado: 25% chance de perder turno */
    STATUS_SLEEP,           /* Dormindo: perde turnos até acordar ou ser atacado */
    STATUS_CONFUSION,       /* Confuso: 30% chance de atacar aliado ou a si mesmo */
    STATUS_BLIND,           /* Cego: -50% precisão em ataques */
    
    /* === BUFFS (Condições Positivas) === */
    STATUS_REGEN,           /* Regeneração: +10% HP max por turno */
    STATUS_STRENGTH_UP,     /* Força Up: +50% dano físico */
    STATUS_DEFENSE_UP,      /* Defesa Up: +50% defesa */
    STATUS_SPEED_UP,        /* Velocidade Up: +50% velocidade */
    
    STATUS_COUNT            /* Total de tipos de status (usado para iteração) */
} StatusType;

/* -----------------------------------------------------------------------------
 * ESTRUTURA: StatusCondition
 * -----------------------------------------------------------------------------
 * Representa uma instância específica de uma condição de status aplicada
 * a um personagem. Contém informações sobre o tipo, duração e intensidade.
 * 
 * Campos:
 *   - type: O tipo da condição de status (StatusType)
 *   - turnsRemaining: Quantidade de turnos restantes para a condição expirar
 *                     Se for -1, a condição é permanente até ser curada
 *   - intensity: Multiplicador de efeito (1.0 = normal, 2.0 = dobro, etc.)
 *                Usado para condições que podem ser empilhadas ou fortalecidas
 * -------------------------------------------------------------------------- */
typedef struct {
    StatusType type;        /* Tipo da condição de status */
    int turnsRemaining;     /* Turnos restantes (-1 = permanente) */
    float intensity;        /* Intensidade do efeito (multiplicador) */
} StatusCondition;

/* -----------------------------------------------------------------------------
 * ESTRUTURA: StatusList
 * -----------------------------------------------------------------------------
 * Gerencia todas as condições de status ativas em um personagem.
 * Utiliza a estrutura de Lista Duplamente Encadeada (List) definida em list.h
 * para permitir adição, remoção e iteração eficientes.
 * 
 * Esta estrutura é essencial para o requisito do projeto de usar estruturas
 * de dados como parte da lógica central do jogo.
 * 
 * Campos:
 *   - conditions: Lista encadeada contendo ponteiros para StatusCondition
 * -------------------------------------------------------------------------- */
typedef struct {
    List conditions;        /* Lista de StatusCondition* ativos */
} StatusList;

/* =============================================================================
 * FUNÇÕES DE INICIALIZAÇÃO E GERENCIAMENTO
 * ============================================================================= */

/**
 * initStatusList - Inicializa uma lista de status vazia
 * @param statusList: Ponteiro para a StatusList a ser inicializada
 * 
 * Deve ser chamada antes de usar qualquer outra função de status.
 * Inicializa a lista encadeada interna com valores padrão.
 */
void initStatusList(StatusList* statusList);

/**
 * freeStatusList - Libera toda a memória alocada pela lista de status
 * @param statusList: Ponteiro para a StatusList a ser liberada
 * 
 * Remove todas as condições ativas e libera a memória associada.
 * Deve ser chamada quando o personagem é destruído ou no fim do jogo.
 */
void freeStatusList(StatusList* statusList);

/* =============================================================================
 * FUNÇÕES DE ADIÇÃO E REMOÇÃO DE STATUS
 * ============================================================================= */

/**
 * addStatusCondition - Adiciona uma nova condição de status ao personagem
 * @param statusList: Ponteiro para a StatusList do personagem
 * @param type: Tipo da condição a ser adicionada
 * @param turns: Duração em turnos (-1 para permanente)
 * @param intensity: Intensidade do efeito (1.0 = normal)
 * 
 * Se o personagem já tiver a mesma condição, a duração é estendida
 * e a intensidade é recalculada (usa o maior valor).
 */
void addStatusCondition(StatusList* statusList, StatusType type, int turns, float intensity);

/**
 * removeStatusCondition - Remove uma condição específica do personagem
 * @param statusList: Ponteiro para a StatusList do personagem
 * @param type: Tipo da condição a ser removida
 * 
 * Remove a primeira ocorrência da condição especificada.
 * Não faz nada se a condição não estiver presente.
 */
void removeStatusCondition(StatusList* statusList, StatusType type);

/**
 * removeAllDebuffs - Remove todas as condições negativas do personagem
 * @param statusList: Ponteiro para a StatusList do personagem
 * 
 * Útil para itens ou habilidades de cura completa.
 * Mantém os buffs (condições positivas) ativos.
 */
void removeAllDebuffs(StatusList* statusList);

/**
 * removeAllBuffs - Remove todas as condições positivas do personagem
 * @param statusList: Ponteiro para a StatusList do personagem
 * 
 * Usado quando um efeito de dispel é aplicado.
 * Mantém os debuffs (condições negativas) ativos.
 */
void removeAllBuffs(StatusList* statusList);

/**
 * clearAllStatus - Remove todas as condições de status do personagem
 * @param statusList: Ponteiro para a StatusList do personagem
 * 
 * Limpa completamente a lista de status, tanto buffs quanto debuffs.
 */
void clearAllStatus(StatusList* statusList);

/* =============================================================================
 * FUNÇÕES DE CONSULTA
 * ============================================================================= */

/**
 * hasStatusCondition - Verifica se o personagem tem uma condição específica
 * @param statusList: Ponteiro para a StatusList do personagem
 * @param type: Tipo da condição a ser verificada
 * @return: 1 se a condição está ativa, 0 caso contrário
 * 
 * Função de verificação rápida para lógica de combate.
 */
int hasStatusCondition(StatusList* statusList, StatusType type);

/**
 * getStatusCondition - Obtém os detalhes de uma condição específica
 * @param statusList: Ponteiro para a StatusList do personagem
 * @param type: Tipo da condição a ser buscada
 * @return: Ponteiro para StatusCondition ou NULL se não encontrada
 * 
 * Retorna o ponteiro direto para a estrutura, permitindo modificações.
 */
StatusCondition* getStatusCondition(StatusList* statusList, StatusType type);

/**
 * countActiveStatus - Conta quantas condições estão ativas
 * @param statusList: Ponteiro para a StatusList do personagem
 * @return: Número de condições ativas
 */
int countActiveStatus(StatusList* statusList);

/**
 * isDebuff - Verifica se um tipo de status é um debuff (condição negativa)
 * @param type: Tipo da condição a ser verificada
 * @return: 1 se é debuff, 0 se é buff ou NONE
 */
int isDebuff(StatusType type);

/**
 * isBuff - Verifica se um tipo de status é um buff (condição positiva)
 * @param type: Tipo da condição a ser verificada
 * @return: 1 se é buff, 0 se é debuff ou NONE
 */
int isBuff(StatusType type);

/* =============================================================================
 * FUNÇÕES DE PROCESSAMENTO DE TURNO
 * ============================================================================= */

/**
 * processStatusEffects - Processa os efeitos de todas as condições no turno
 * @param statusList: Ponteiro para a StatusList do personagem
 * @param currentHP: Ponteiro para o HP atual (será modificado)
 * @param maxHP: HP máximo do personagem (para cálculos de porcentagem)
 * @return: Dano total causado por condições de status no turno
 * 
 * Esta função deve ser chamada no início ou fim de cada turno do personagem.
 * Aplica os efeitos de cada condição (dano de veneno, cura de regeneração, etc.)
 * e decrementa os contadores de turnos restantes.
 */
int processStatusEffects(StatusList* statusList, int* currentHP, int maxHP);

/**
 * updateStatusDurations - Atualiza as durações e remove status expirados
 * @param statusList: Ponteiro para a StatusList do personagem
 * 
 * Decrementa turnsRemaining de cada condição e remove as que chegaram a 0.
 * Condições com turnsRemaining = -1 (permanentes) não são afetadas.
 */
void updateStatusDurations(StatusList* statusList);

/**
 * canActThisTurn - Verifica se o personagem pode agir no turno atual
 * @param statusList: Ponteiro para a StatusList do personagem
 * @return: 1 se pode agir, 0 se está impedido (paralisia, sono, etc.)
 * 
 * Considera condições como paralisia (chance de falhar) e sono (sempre falha).
 * Usa números aleatórios para condições com chance.
 */
int canActThisTurn(StatusList* statusList);

/* =============================================================================
 * FUNÇÕES DE MODIFICADORES DE STATS
 * ============================================================================= */

/**
 * getStrengthModifier - Calcula o modificador de força baseado nos status
 * @param statusList: Ponteiro para a StatusList do personagem
 * @return: Multiplicador de força (ex: 1.5 para +50%, 0.75 para -25%)
 * 
 * Considera: STATUS_BURN (-25%), STATUS_STRENGTH_UP (+50%)
 */
float getStrengthModifier(StatusList* statusList);

/**
 * getDefenseModifier - Calcula o modificador de defesa baseado nos status
 * @param statusList: Ponteiro para a StatusList do personagem
 * @return: Multiplicador de defesa (ex: 1.5 para +50%)
 * 
 * Considera: STATUS_DEFENSE_UP (+50%)
 */
float getDefenseModifier(StatusList* statusList);

/**
 * getSpeedModifier - Calcula o modificador de velocidade baseado nos status
 * @param statusList: Ponteiro para a StatusList do personagem
 * @return: Multiplicador de velocidade (ex: 1.5 para +50%)
 * 
 * Considera: STATUS_SPEED_UP (+50%)
 */
float getSpeedModifier(StatusList* statusList);

/**
 * getAccuracyModifier - Calcula o modificador de precisão baseado nos status
 * @param statusList: Ponteiro para a StatusList do personagem
 * @return: Multiplicador de precisão (ex: 0.5 para -50%)
 * 
 * Considera: STATUS_BLIND (-50%)
 */
float getAccuracyModifier(StatusList* statusList);

/* =============================================================================
 * FUNÇÕES UTILITÁRIAS
 * ============================================================================= */

/**
 * getStatusName - Retorna o nome de exibição de um tipo de status
 * @param type: Tipo da condição
 * @return: String com o nome em português
 * 
 * Usado para exibição na interface do jogo.
 */
const char* getStatusName(StatusType type);

/**
 * getStatusDescription - Retorna a descrição de um tipo de status
 * @param type: Tipo da condição
 * @return: String com a descrição do efeito
 * 
 * Usado para tooltips e menus de informação.
 */
const char* getStatusDescription(StatusType type);

#endif /* STATUS_CONDITION_H */
