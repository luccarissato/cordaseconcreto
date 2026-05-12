/**
 * =============================================================================
 * STATUS_CONDITION.C - Implementação do Sistema de Condições de Status
 * =============================================================================
 * 
 * Este arquivo contém a implementação de todas as funções relacionadas
 * ao sistema de condições de status do jogo.
 * 
 * O sistema utiliza uma Lista Duplamente Encadeada para gerenciar múltiplas
 * condições de status simultâneas em cada personagem, atendendo ao requisito
 * de usar estruturas de dados como parte da lógica central do jogo.
 * 
 * =============================================================================
 */

#include "status_condition.h"
#include <stdlib.h>
#include <string.h>

/* =============================================================================
 * CONSTANTES DE CONFIGURAÇÃO DOS EFEITOS
 * ============================================================================= */

/* Porcentagem do HP máximo perdida por turno com envenenamento (10%) */
#define POISON_DAMAGE_PERCENT 0.10f

/* Porcentagem do HP máximo perdida por turno com queimadura (5%) */
#define BURN_DAMAGE_PERCENT 0.05f

/* Porcentagem do HP máximo recuperada por turno com regeneração (10%) */
#define REGEN_HEAL_PERCENT 0.10f

/* Chance de perder o turno quando paralisado (25%) */
#define PARALYSIS_SKIP_CHANCE 25

/* Chance de atacar a si mesmo ou aliado quando confuso (30%) */
#define CONFUSION_SELF_HIT_CHANCE 30

/* Modificadores de status para atributos */
#define BURN_STRENGTH_REDUCTION 0.25f    /* -25% força quando queimando */
#define STRENGTH_UP_BONUS 0.50f          /* +50% força com buff */
#define DEFENSE_UP_BONUS 0.50f           /* +50% defesa com buff */
#define SPEED_UP_BONUS 0.50f             /* +50% velocidade com buff */
#define BLIND_ACCURACY_REDUCTION 0.50f   /* -50% precisão quando cego */

/* =============================================================================
 * FUNÇÕES DE INICIALIZAÇÃO E GERENCIAMENTO
 * ============================================================================= */

/**
 * initStatusList - Inicializa uma lista de status vazia
 * 
 * Prepara a estrutura StatusList para uso, inicializando a lista encadeada
 * interna com valores padrão (head = tail = NULL, size = 0).
 */
void initStatusList(StatusList* statusList) {
    /* Verifica se o ponteiro é válido antes de inicializar */
    if (statusList == NULL) return;
    
    /* Inicializa a lista encadeada interna usando a função do módulo list.h */
    initList(&statusList->conditions);
}

/**
 * freeStatusList - Libera toda a memória alocada pela lista de status
 * 
 * Percorre a lista encadeada, liberando a memória de cada StatusCondition
 * alocado dinamicamente, e depois limpa a lista.
 */
void freeStatusList(StatusList* statusList) {
    if (statusList == NULL) return;
    
    /* Percorre todos os nós da lista liberando as condições */
    ListNode* current = statusList->conditions.head;
    while (current != NULL) {
        ListNode* next = current->next;
        
        /* Libera a estrutura StatusCondition armazenada no nó */
        if (current->data != NULL) {
            free(current->data);
        }
        
        current = next;
    }
    
    /* Limpa a lista (libera os nós, mas os dados já foram liberados) */
    clearList(&statusList->conditions);
}

/* =============================================================================
 * FUNÇÕES DE ADIÇÃO E REMOÇÃO DE STATUS
 * ============================================================================= */

/**
 * addStatusCondition - Adiciona uma nova condição de status ao personagem
 * 
 * Se a condição já existir, atualiza a duração (soma os turnos) e
 * usa a maior intensidade entre a existente e a nova.
 * Caso contrário, cria uma nova instância e adiciona à lista.
 */
void addStatusCondition(StatusList* statusList, StatusType type, int turns, float intensity) {
    if (statusList == NULL || type == STATUS_NONE) return;
    
    /* Verifica se a condição já existe para atualizar ao invés de duplicar */
    StatusCondition* existing = getStatusCondition(statusList, type);
    
    if (existing != NULL) {
        /* Condição já existe: atualiza duração e intensidade */
        
        /* Se a condição existente é permanente (-1), mantém permanente */
        /* Caso contrário, soma as durações */
        if (existing->turnsRemaining != -1 && turns != -1) {
            existing->turnsRemaining += turns;
        } else if (turns == -1) {
            existing->turnsRemaining = -1; /* Nova condição é permanente */
        }
        
        /* Usa a maior intensidade entre a existente e a nova */
        if (intensity > existing->intensity) {
            existing->intensity = intensity;
        }
    } else {
        /* Condição não existe: cria nova instância */
        
        /* Aloca memória para a nova condição */
        StatusCondition* newCondition = (StatusCondition*) malloc(sizeof(StatusCondition));
        if (newCondition == NULL) return; /* Falha na alocação */
        
        /* Inicializa os campos da condição */
        newCondition->type = type;
        newCondition->turnsRemaining = turns;
        newCondition->intensity = intensity;
        
        /* Adiciona à lista encadeada usando pushBack */
        pushBack(&statusList->conditions, newCondition);
    }
}

/**
 * removeStatusCondition - Remove uma condição específica do personagem
 * 
 * Percorre a lista procurando a condição do tipo especificado.
 * Remove a primeira ocorrência encontrada e libera a memória.
 */
void removeStatusCondition(StatusList* statusList, StatusType type) {
    if (statusList == NULL) return;
    
    ListNode* current = statusList->conditions.head;
    
    while (current != NULL) {
        StatusCondition* condition = (StatusCondition*) current->data;
        
        if (condition != NULL && condition->type == type) {
            /* Encontrou a condição: remove e libera memória */
            ListNode* toRemove = current;
            
            /* Libera a estrutura StatusCondition */
            free(condition);
            
            /* Remove o nó da lista */
            removeNode(&statusList->conditions, toRemove);
            
            return; /* Remove apenas a primeira ocorrência */
        }
        
        current = current->next;
    }
}

/**
 * removeAllDebuffs - Remove todas as condições negativas do personagem
 * 
 * Percorre a lista verificando cada condição. Se for um debuff
 * (usando a função isDebuff), remove e libera a memória.
 */
void removeAllDebuffs(StatusList* statusList) {
    if (statusList == NULL) return;
    
    ListNode* current = statusList->conditions.head;
    
    while (current != NULL) {
        ListNode* next = current->next; /* Guarda referência antes de remover */
        StatusCondition* condition = (StatusCondition*) current->data;
        
        if (condition != NULL && isDebuff(condition->type)) {
            /* É um debuff: remove */
            free(condition);
            removeNode(&statusList->conditions, current);
        }
        
        current = next;
    }
}

/**
 * removeAllBuffs - Remove todas as condições positivas do personagem
 * 
 * Similar a removeAllDebuffs, mas remove apenas buffs.
 */
void removeAllBuffs(StatusList* statusList) {
    if (statusList == NULL) return;
    
    ListNode* current = statusList->conditions.head;
    
    while (current != NULL) {
        ListNode* next = current->next;
        StatusCondition* condition = (StatusCondition*) current->data;
        
        if (condition != NULL && isBuff(condition->type)) {
            /* É um buff: remove */
            free(condition);
            removeNode(&statusList->conditions, current);
        }
        
        current = next;
    }
}

/**
 * clearAllStatus - Remove todas as condições de status do personagem
 * 
 * Limpa completamente a lista, liberando toda a memória alocada.
 */
void clearAllStatus(StatusList* statusList) {
    if (statusList == NULL) return;
    
    /* Usa freeStatusList que já faz a limpeza completa */
    freeStatusList(statusList);
    
    /* Reinicializa a lista para uso futuro */
    initStatusList(statusList);
}

/* =============================================================================
 * FUNÇÕES DE CONSULTA
 * ============================================================================= */

/**
 * hasStatusCondition - Verifica se o personagem tem uma condição específica
 * 
 * Percorre a lista procurando a condição do tipo especificado.
 * Retorna 1 (verdadeiro) se encontrar, 0 (falso) caso contrário.
 */
int hasStatusCondition(StatusList* statusList, StatusType type) {
    return getStatusCondition(statusList, type) != NULL;
}

/**
 * getStatusCondition - Obtém os detalhes de uma condição específica
 * 
 * Percorre a lista procurando a condição do tipo especificado.
 * Retorna o ponteiro direto para a estrutura se encontrada, NULL caso contrário.
 */
StatusCondition* getStatusCondition(StatusList* statusList, StatusType type) {
    if (statusList == NULL) return NULL;
    
    ListNode* current = statusList->conditions.head;
    
    while (current != NULL) {
        StatusCondition* condition = (StatusCondition*) current->data;
        
        if (condition != NULL && condition->type == type) {
            return condition;
        }
        
        current = current->next;
    }
    
    return NULL;
}

/**
 * countActiveStatus - Conta quantas condições estão ativas
 * 
 * Retorna o tamanho atual da lista de condições.
 */
int countActiveStatus(StatusList* statusList) {
    if (statusList == NULL) return 0;
    return statusList->conditions.size;
}

/**
 * isDebuff - Verifica se um tipo de status é um debuff (condição negativa)
 * 
 * Debuffs são: POISON, BURN, PARALYSIS, SLEEP, CONFUSION, BLIND
 */
int isDebuff(StatusType type) {
    switch (type) {
        case STATUS_POISON:
        case STATUS_BURN:
        case STATUS_PARALYSIS:
        case STATUS_SLEEP:
        case STATUS_CONFUSION:
        case STATUS_BLIND:
            return 1;
        default:
            return 0;
    }
}

/**
 * isBuff - Verifica se um tipo de status é um buff (condição positiva)
 * 
 * Buffs são: REGEN, STRENGTH_UP, DEFENSE_UP, SPEED_UP
 */
int isBuff(StatusType type) {
    switch (type) {
        case STATUS_REGEN:
        case STATUS_STRENGTH_UP:
        case STATUS_DEFENSE_UP:
        case STATUS_SPEED_UP:
            return 1;
        default:
            return 0;
    }
}

/* =============================================================================
 * FUNÇÕES DE PROCESSAMENTO DE TURNO
 * ============================================================================= */

/**
 * processStatusEffects - Processa os efeitos de todas as condições no turno
 * 
 * Percorre todas as condições ativas e aplica seus efeitos:
 * - Veneno: causa dano baseado em % do HP máximo
 * - Queimadura: causa dano (menor que veneno)
 * - Regeneração: cura baseado em % do HP máximo
 * 
 * Retorna o dano total causado (negativo se houver cura líquida).
 */
int processStatusEffects(StatusList* statusList, int* currentHP, int maxHP) {
    if (statusList == NULL || currentHP == NULL) return 0;
    
    int totalDamage = 0;
    
    ListNode* current = statusList->conditions.head;
    
    while (current != NULL) {
        StatusCondition* condition = (StatusCondition*) current->data;
        
        if (condition != NULL) {
            switch (condition->type) {
                case STATUS_POISON: {
                    /* Veneno: causa dano baseado em % do HP máximo */
                    int poisonDamage = (int)(maxHP * POISON_DAMAGE_PERCENT * condition->intensity);
                    if (poisonDamage < 1) poisonDamage = 1; /* Mínimo 1 de dano */
                    
                    *currentHP -= poisonDamage;
                    totalDamage += poisonDamage;
                    break;
                }
                
                case STATUS_BURN: {
                    /* Queimadura: causa dano menor que veneno */
                    int burnDamage = (int)(maxHP * BURN_DAMAGE_PERCENT * condition->intensity);
                    if (burnDamage < 1) burnDamage = 1;
                    
                    *currentHP -= burnDamage;
                    totalDamage += burnDamage;
                    break;
                }
                
                case STATUS_REGEN: {
                    /* Regeneração: cura baseado em % do HP máximo */
                    int regenHeal = (int)(maxHP * REGEN_HEAL_PERCENT * condition->intensity);
                    if (regenHeal < 1) regenHeal = 1;
                    
                    *currentHP += regenHeal;
                    
                    /* Não permite ultrapassar HP máximo */
                    if (*currentHP > maxHP) {
                        *currentHP = maxHP;
                    }
                    
                    totalDamage -= regenHeal; /* Cura é "dano negativo" */
                    break;
                }
                
                default:
                    /* Outros status não causam dano direto por turno */
                    break;
            }
        }
        
        current = current->next;
    }
    
    /* Garante que HP não fique negativo */
    if (*currentHP < 0) {
        *currentHP = 0;
    }
    
    return totalDamage;
}

/**
 * updateStatusDurations - Atualiza as durações e remove status expirados
 * 
 * Decrementa o contador de turnos de cada condição.
 * Remove as condições que chegaram a 0 turnos.
 * Condições permanentes (turnsRemaining = -1) não são afetadas.
 */
void updateStatusDurations(StatusList* statusList) {
    if (statusList == NULL) return;
    
    ListNode* current = statusList->conditions.head;
    
    while (current != NULL) {
        ListNode* next = current->next;
        StatusCondition* condition = (StatusCondition*) current->data;
        
        if (condition != NULL) {
            /* Só decrementa se não for permanente */
            if (condition->turnsRemaining > 0) {
                condition->turnsRemaining--;
                
                /* Se chegou a zero, remove a condição */
                if (condition->turnsRemaining == 0) {
                    free(condition);
                    removeNode(&statusList->conditions, current);
                }
            }
            /* Se turnsRemaining == -1, é permanente e não é decrementado */
        }
        
        current = next;
    }
}

/**
 * canActThisTurn - Verifica se o personagem pode agir no turno atual
 * 
 * Verifica condições que impedem ação:
 * - Sono: sempre impede (até acordar)
 * - Paralisia: chance de impedir (25%)
 * 
 * Retorna 1 se pode agir, 0 se está impedido.
 */
int canActThisTurn(StatusList* statusList) {
    if (statusList == NULL) return 1;
    
    /* Sono sempre impede a ação */
    if (hasStatusCondition(statusList, STATUS_SLEEP)) {
        return 0;
    }
    
    /* Paralisia tem chance de impedir */
    if (hasStatusCondition(statusList, STATUS_PARALYSIS)) {
        int roll = rand() % 100;
        if (roll < PARALYSIS_SKIP_CHANCE) {
            return 0; /* Paralisado este turno */
        }
    }
    
    return 1; /* Pode agir normalmente */
}

/* =============================================================================
 * FUNÇÕES DE MODIFICADORES DE STATS
 * ============================================================================= */

/**
 * getStrengthModifier - Calcula o modificador de força baseado nos status
 * 
 * Retorna um multiplicador para ser aplicado ao dano físico.
 * Considera queimadura (-25%) e buff de força (+50%).
 */
float getStrengthModifier(StatusList* statusList) {
    float modifier = 1.0f;
    
    if (statusList == NULL) return modifier;
    
    /* Queimadura reduz força */
    if (hasStatusCondition(statusList, STATUS_BURN)) {
        modifier -= BURN_STRENGTH_REDUCTION;
    }
    
    /* Buff de força aumenta */
    StatusCondition* strengthUp = getStatusCondition(statusList, STATUS_STRENGTH_UP);
    if (strengthUp != NULL) {
        modifier += STRENGTH_UP_BONUS * strengthUp->intensity;
    }
    
    /* Garante que o modificador não seja negativo */
    if (modifier < 0.1f) modifier = 0.1f;
    
    return modifier;
}

/**
 * getDefenseModifier - Calcula o modificador de defesa baseado nos status
 * 
 * Retorna um multiplicador para ser aplicado à defesa.
 */
float getDefenseModifier(StatusList* statusList) {
    float modifier = 1.0f;
    
    if (statusList == NULL) return modifier;
    
    /* Buff de defesa aumenta */
    StatusCondition* defenseUp = getStatusCondition(statusList, STATUS_DEFENSE_UP);
    if (defenseUp != NULL) {
        modifier += DEFENSE_UP_BONUS * defenseUp->intensity;
    }
    
    return modifier;
}

/**
 * getSpeedModifier - Calcula o modificador de velocidade baseado nos status
 * 
 * Retorna um multiplicador para ser aplicado à velocidade.
 */
float getSpeedModifier(StatusList* statusList) {
    float modifier = 1.0f;
    
    if (statusList == NULL) return modifier;
    
    /* Buff de velocidade aumenta */
    StatusCondition* speedUp = getStatusCondition(statusList, STATUS_SPEED_UP);
    if (speedUp != NULL) {
        modifier += SPEED_UP_BONUS * speedUp->intensity;
    }
    
    return modifier;
}

/**
 * getAccuracyModifier - Calcula o modificador de precisão baseado nos status
 * 
 * Retorna um multiplicador para ser aplicado à chance de acerto.
 */
float getAccuracyModifier(StatusList* statusList) {
    float modifier = 1.0f;
    
    if (statusList == NULL) return modifier;
    
    /* Cegueira reduz precisão */
    if (hasStatusCondition(statusList, STATUS_BLIND)) {
        modifier -= BLIND_ACCURACY_REDUCTION;
    }
    
    /* Garante que a precisão não seja negativa */
    if (modifier < 0.1f) modifier = 0.1f;
    
    return modifier;
}

/* =============================================================================
 * FUNÇÕES UTILITÁRIAS
 * ============================================================================= */

/**
 * getStatusName - Retorna o nome de exibição de um tipo de status
 * 
 * Usado para mostrar o nome da condição na interface do jogo.
 */
const char* getStatusName(StatusType type) {
    switch (type) {
        case STATUS_NONE:        return "Nenhum";
        case STATUS_POISON:      return "Envenenado";
        case STATUS_BURN:        return "Queimando";
        case STATUS_PARALYSIS:   return "Paralisado";
        case STATUS_SLEEP:       return "Dormindo";
        case STATUS_CONFUSION:   return "Confuso";
        case STATUS_BLIND:       return "Cego";
        case STATUS_REGEN:       return "Regenerando";
        case STATUS_STRENGTH_UP: return "Forca Aumentada";
        case STATUS_DEFENSE_UP:  return "Defesa Aumentada";
        case STATUS_SPEED_UP:    return "Velocidade Aumentada";
        default:                 return "Desconhecido";
    }
}

/**
 * getStatusDescription - Retorna a descrição de um tipo de status
 * 
 * Usado para tooltips e menus de informação.
 */
const char* getStatusDescription(StatusType type) {
    switch (type) {
        case STATUS_NONE:
            return "Sem efeitos de status ativos.";
        case STATUS_POISON:
            return "Perde 10%% do HP maximo a cada turno.";
        case STATUS_BURN:
            return "Perde 5%% do HP maximo por turno e tem forca reduzida em 25%%.";
        case STATUS_PARALYSIS:
            return "Tem 25%% de chance de perder o turno.";
        case STATUS_SLEEP:
            return "Nao pode agir ate acordar ou ser atacado.";
        case STATUS_CONFUSION:
            return "Tem 30%% de chance de atacar a si mesmo ou aliados.";
        case STATUS_BLIND:
            return "Precisao dos ataques reduzida em 50%%.";
        case STATUS_REGEN:
            return "Recupera 10%% do HP maximo a cada turno.";
        case STATUS_STRENGTH_UP:
            return "Dano fisico aumentado em 50%%.";
        case STATUS_DEFENSE_UP:
            return "Defesa aumentada em 50%%.";
        case STATUS_SPEED_UP:
            return "Velocidade aumentada em 50%%.";
        default:
            return "Efeito desconhecido.";
    }
}
