
#include "status_condition.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
/* For debug flag reuse */
#include "../ui/status_visuals.h"

/* Porcentagem do HP máximo perdida por turno com envenenamento (12%) */
#define POISON_DAMAGE_PERCENT 0.12f

/* Porcentagem do HP máximo perdida por turno com ensolação (15%) */
#define ENSOLACAO_DAMAGE_PERCENT 0.15f

/* Dano base de sangramento (5 HP inicial, cresce +5/turno) */
#define BLEED_BASE_DAMAGE 5

/* Porcentagem do HP máximo recuperada por turno com regeneração (10%) */
#define REGEN_HEAL_PERCENT 0.10f

/* Chance de perder o turno quando paralisado (25%) */
#define PARALYSIS_SKIP_CHANCE 25

/* Chance de atacar a si mesmo ou aliado quando confuso (30%) */
#define CONFUSION_SELF_HIT_CHANCE 30

#define STRENGTH_UP_BONUS 0.50f          /* +50% força com buff */
#define DEFENSE_UP_BONUS 0.50f           /* +50% defesa com buff */
#define SPEED_UP_BONUS 0.50f             /* +50% velocidade com buff */

void initStatusList(StatusList* statusList) {
    if (statusList == NULL) return;
    
    initList(&statusList->conditions);
}

void freeStatusList(StatusList* statusList) {
    if (statusList == NULL) return;
    
    ListNode* current = statusList->conditions.head;
    while (current != NULL) {
        ListNode* next = current->next;
        
        if (current->data != NULL) {
            free(current->data);
        }
        
        current = next;
    }
    
    clearList(&statusList->conditions);
}

void addStatusCondition(StatusList* statusList, StatusType type, int turns, float intensity) {
    if (statusList == NULL || type == STATUS_NONE) return;
    
    StatusCondition* existing = getStatusCondition(statusList, type);
    
    if (existing != NULL) {
        if (existing->turnsRemaining != -1 && turns != -1) {
            existing->turnsRemaining += turns;
        } else if (turns == -1) {
            existing->turnsRemaining = -1;
        }
        
        if (intensity > existing->intensity) {
            existing->intensity = intensity;
        }
        #if STATUS_VIS_DEBUG
        fprintf(stderr, "[STATUS_COND] updated existing %d turns=%d intensity=%.2f\n", type, existing->turnsRemaining, existing->intensity);
        #endif
        
    } else {  
        StatusCondition* newCondition = (StatusCondition*) malloc(sizeof(StatusCondition));
        if (newCondition == NULL) return;
        
        newCondition->type = type;
        newCondition->turnsRemaining = turns;
        newCondition->intensity = intensity;
        
        pushBack(&statusList->conditions, newCondition);
        #if STATUS_VIS_DEBUG
        fprintf(stderr, "[STATUS_COND] added new %d turns=%d intensity=%.2f\n", type, turns, intensity);
        #endif
    }
}

void removeStatusCondition(StatusList* statusList, StatusType type) {
    if (statusList == NULL) return;
    
    ListNode* current = statusList->conditions.head;
    int safety = 0;
    int maxSafety = statusList->conditions.size + 1;
    
    while (current != NULL && safety < maxSafety) {
        StatusCondition* condition = (StatusCondition*) current->data;
        
        if (condition != NULL && condition->type == type) {
            ListNode* toRemove = current;
            
            free(condition);
            
            removeNode(&statusList->conditions, toRemove);
            
            return; 
        }
        
        current = current->next;
        safety++;
    }
}

static int isRemovableByDebuffRemoval(StatusType type) {
    switch (type) {
        case STATUS_ENSOLACAO:  return 0;
        case STATUS_POISON:
        case STATUS_BLEED:
        case STATUS_CONFUSION:
        case STATUS_WEAKEN:
        case STATUS_SLOW:
        case STATUS_DEFENSE_DOWN:
        case STATUS_ENCHARCADO:
            return 1;
        default:
            return 0;
    }
}

void removeAllDebuffs(StatusList* statusList) {
    if (statusList == NULL) return;
    
    ListNode* current = statusList->conditions.head;
    int iterations = statusList->conditions.size;
    
    for (int i = 0; i < iterations && current != NULL && statusList->conditions.size > 0; i++) {
        ListNode* next = current->next;
        StatusCondition* condition = (StatusCondition*) current->data;
        
        if (condition != NULL && isDebuff(condition->type) && isRemovableByDebuffRemoval(condition->type)) {
            free(condition);
            removeNode(&statusList->conditions, current);
            
            if (statusList->conditions.size == 0) break;
        }
        
        current = next;
    }
}


void removeAllBuffs(StatusList* statusList) {
    if (statusList == NULL) return;
    
    ListNode* current = statusList->conditions.head;
    
    while (current != NULL) {
        ListNode* next = current->next;
        StatusCondition* condition = (StatusCondition*) current->data;
        
        if (condition != NULL && isBuff(condition->type)) {
            free(condition);
            removeNode(&statusList->conditions, current);
        }
        
        current = next;
    }
}

void clearAllStatus(StatusList* statusList) {
    if (statusList == NULL) return;
    

    freeStatusList(statusList);
    

    initStatusList(statusList);
}

int hasStatusCondition(StatusList* statusList, StatusType type) {
    return getStatusCondition(statusList, type) != NULL;
}

StatusCondition* getStatusCondition(StatusList* statusList, StatusType type) {
    if (statusList == NULL) return NULL;
    
    if (statusList->conditions.head == NULL || statusList->conditions.size == 0) {
        return NULL;
    }
    
    ListNode* current = statusList->conditions.head;
    int iterations = statusList->conditions.size;  /* Limite iterações para evitar loop infinito em lista circular */
    
    for (int i = 0; i < iterations && current != NULL; i++) {
        StatusCondition* condition = (StatusCondition*) current->data;
        
        /* === PATCH 4: Double-check data pointer antes de usar === */
        if (condition != NULL && condition->type == type) {
            return condition;
        }
        
        current = current->next;
    }
    
    return NULL;
}

int countActiveStatus(StatusList* statusList) {
    if (statusList == NULL) return 0;
    return statusList->conditions.size;
}

int isDebuff(StatusType type) {
    switch (type) {
        case STATUS_POISON:
        case STATUS_ENSOLACAO:
        case STATUS_CONFUSION:
        case STATUS_BLEED:
        case STATUS_WEAKEN:
        case STATUS_SLOW:
        case STATUS_DEFENSE_DOWN:
        case STATUS_ENCHARCADO:
            return 1;
        default:
            return 0;
    }
}

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

int processStatusEffects(StatusList* statusList, int* currentHP, int maxHP) {
    if (statusList == NULL || currentHP == NULL) return 0;

    int totalDamage = 0;

    /* Lista é circular; itere apenas pelo tamanho inicial para evitar loops infinitos */
    List* list = &statusList->conditions;
    if (list->head == NULL || list->size == 0) return 0;

    ListNode* current = list->head;
    int iterations = list->size;

    for (int i = 0; i < iterations && current != NULL; i++) {
        StatusCondition* condition = (StatusCondition*) current->data;

        if (condition != NULL) {
            switch (condition->type) {
                case STATUS_POISON: {
                    int poisonDamage = (int)(maxHP * POISON_DAMAGE_PERCENT * condition->intensity);
                    if (poisonDamage < 1) poisonDamage = 1;
                    *currentHP -= poisonDamage;
                    totalDamage += poisonDamage;
                    break;
                }

                case STATUS_ENSOLACAO: {
                    int ensolacaoDamage = (int)(maxHP * ENSOLACAO_DAMAGE_PERCENT * condition->intensity);
                    if (ensolacaoDamage < 1) ensolacaoDamage = 1;
                    *currentHP -= ensolacaoDamage;
                    totalDamage += ensolacaoDamage;
                    break;
                }

                case STATUS_BLEED: {
                    int bleedDamage = (int)(BLEED_BASE_DAMAGE + condition->intensity);
                    if (bleedDamage < 1) bleedDamage = 1;
                    *currentHP -= bleedDamage;
                    totalDamage += bleedDamage;
                    condition->intensity += 5.0f;
                    break;
                }

                case STATUS_REGEN: {
                    int regenHeal = (int)(maxHP * REGEN_HEAL_PERCENT * condition->intensity);
                    if (regenHeal < 1) regenHeal = 1;
                    *currentHP += regenHeal;
                    if (*currentHP > maxHP) {
                        *currentHP = maxHP;
                    }
                    totalDamage -= regenHeal;
                    break;
                }

                default:
                    break;
            }
        }

        current = current->next;
    }
    
    if (*currentHP < 0) {
        *currentHP = 0;
    }
    
    return totalDamage;
}

void updateStatusDurations(StatusList* statusList) {
    if (statusList == NULL) return;

    List* list = &statusList->conditions;
    if (list->head == NULL || list->size == 0) return;

    /* Itere apenas pelo tamanho inicial da lista para evitar loops infinitos */
    ListNode* current = list->head;
    int iterations = list->size;

    for (int i = 0; i < iterations && current != NULL && list->size > 0; i++) {
        ListNode* next = current->next;
        StatusCondition* condition = (StatusCondition*) current->data;

        if (condition != NULL) {
            if (condition->turnsRemaining > 0) {
                condition->turnsRemaining--;

                if (condition->turnsRemaining == 0) {
                    free(condition);
                    removeNode(list, current);

                    if (list->size == 0) {
                        break;
                    }
                }
            }
        }

        current = next;
    }
}

int canActThisTurn(StatusList* statusList) {
    if (statusList == NULL) return 1;
    
    return 1;
}

float getStrengthModifier(StatusList* statusList) {
    float modifier = 1.0f;
    
    if (statusList == NULL) return modifier;

    if (hasStatusCondition(statusList, STATUS_WEAKEN)) {
        modifier -= 0.25f;
    }
    
    StatusCondition* strengthUp = getStatusCondition(statusList, STATUS_STRENGTH_UP);
    if (strengthUp != NULL) {
        modifier += STRENGTH_UP_BONUS * strengthUp->intensity;
    }
    
    if (modifier < 0.1f) modifier = 0.1f;
    
    return modifier;
}

float getDefenseModifier(StatusList* statusList) {
    float modifier = 1.0f;
    
    if (statusList == NULL) return modifier;
    
    StatusCondition* defenseUp = getStatusCondition(statusList, STATUS_DEFENSE_UP);
    if (defenseUp != NULL) {
        modifier += DEFENSE_UP_BONUS * defenseUp->intensity;
    }

    if (hasStatusCondition(statusList, STATUS_DEFENSE_DOWN)) {
        modifier -= 0.25f;
    }

    if (modifier < 0.1f) modifier = 0.1f;
    
    return modifier;
}

float getSpeedModifier(StatusList* statusList) {
    float modifier = 1.0f;
    
    if (statusList == NULL) return modifier;
    
    StatusCondition* speedUp = getStatusCondition(statusList, STATUS_SPEED_UP);
    if (speedUp != NULL) {
        modifier += SPEED_UP_BONUS * speedUp->intensity;
    }

    if (hasStatusCondition(statusList, STATUS_SLOW)) {
        modifier -= 0.25f;
    }

    if (modifier < 0.1f) modifier = 0.1f;
    
    return modifier;
}

float getAccuracyModifier(StatusList* statusList) {
    float modifier = 1.0f;
    
    if (statusList == NULL) return modifier;
    
    if (modifier < 0.1f) modifier = 0.1f;
    
    return modifier;
}

const char* getStatusName(StatusType type) {
    switch (type) {
        case STATUS_NONE:           return "Nenhum";
        case STATUS_POISON:         return "Envenenado";
        case STATUS_ENSOLACAO:      return "Ensolacao";
        case STATUS_CONFUSION:      return "Confuso";
        case STATUS_BLEED:          return "Sangramento";
        case STATUS_WEAKEN:         return "Fraqueza";
        case STATUS_SLOW:           return "Lentidao";
        case STATUS_DEFENSE_DOWN:   return "Defesa Reduzida";
        case STATUS_ENCHARCADO:     return "Encharcado";
        case STATUS_REGEN:          return "Regenerando";
        case STATUS_STRENGTH_UP:    return "Forca Aumentada";
        case STATUS_DEFENSE_UP:     return "Defesa Aumentada";
        case STATUS_SPEED_UP:       return "Velocidade Aumentada";
        default:                    return "Desconhecido";
    }
}

const char* getStatusDescription(StatusType type) {
    switch (type) {
        case STATUS_NONE:
            return "Sem efeitos de status ativos.";
        case STATUS_POISON:
            return "Perde 12%% do HP maximo a cada turno.";
        case STATUS_ENSOLACAO:
            return "Perde 15%% do HP maximo por turno. Nao pode ser removido por limpeza.";
        case STATUS_CONFUSION:
            return "Tem 30%% de chance de atacar a si mesmo ou aliados.";
        case STATUS_BLEED:
            return "Sangra e perde HP escalavel a cada turno. Dano aumenta +5 por turno.";
        case STATUS_WEAKEN:
            return "Forca reduzida em 25%%.";
        case STATUS_SLOW:
            return "Velocidade reduzida em 25%%.";
        case STATUS_DEFENSE_DOWN:
            return "Defesa reduzida em 25%%.";
        case STATUS_ENCHARCADO:
            return "Recebe 25%% mais dano magico.";
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
