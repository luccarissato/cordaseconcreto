
#include "status_condition.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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

#define BURN_STRENGTH_REDUCTION 0.25f    /* -25% força quando queimando */
#define STRENGTH_UP_BONUS 0.50f          /* +50% força com buff */
#define DEFENSE_UP_BONUS 0.50f           /* +50% defesa com buff */
#define SPEED_UP_BONUS 0.50f             /* +50% velocidade com buff */
#define BLIND_ACCURACY_REDUCTION 0.50f   /* -50% precisão quando cego */

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
        
    } else {  
        StatusCondition* newCondition = (StatusCondition*) malloc(sizeof(StatusCondition));
        if (newCondition == NULL) return;
        
        newCondition->type = type;
        newCondition->turnsRemaining = turns;
        newCondition->intensity = intensity;
        
        pushBack(&statusList->conditions, newCondition);
    }
}

void removeStatusCondition(StatusList* statusList, StatusType type) {
    if (statusList == NULL) return;
    
    ListNode* current = statusList->conditions.head;
    
    while (current != NULL) {
        StatusCondition* condition = (StatusCondition*) current->data;
        
        if (condition != NULL && condition->type == type) {
            ListNode* toRemove = current;
            
            free(condition);
            
            removeNode(&statusList->conditions, toRemove);
            
            return; 
        }
        
        current = current->next;
    }
}

void removeAllDebuffs(StatusList* statusList) {
    if (statusList == NULL) return;
    
    ListNode* current = statusList->conditions.head;
    
    while (current != NULL) {
        ListNode* next = current->next;
        StatusCondition* condition = (StatusCondition*) current->data;
        
        if (condition != NULL && isDebuff(condition->type) && condition->type != STATUS_BURN) {
            free(condition);
            removeNode(&statusList->conditions, current);
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

int countActiveStatus(StatusList* statusList) {
    if (statusList == NULL) return 0;
    return statusList->conditions.size;
}

int isDebuff(StatusType type) {
    switch (type) {
        case STATUS_POISON:
        case STATUS_BURN:
        case STATUS_PARALYSIS:
        case STATUS_SLEEP:
        case STATUS_CONFUSION:
        case STATUS_BLIND:
        case STATUS_BLEED:
        case STATUS_WEAKEN:
        case STATUS_SLOW:
        case STATUS_DEFENSE_DOWN:
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

                case STATUS_BURN: {
                    int burnDamage = (int)(maxHP * BURN_DAMAGE_PERCENT * condition->intensity);
                    if (burnDamage < 1) burnDamage = 1;

                    *currentHP -= burnDamage;
                    totalDamage += burnDamage;
                    break;
                }

                case STATUS_BLEED: {
                    int bleedDamage = (int)condition->intensity;
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

    for (int i = 0; i < iterations && current != NULL; i++) {
        ListNode* next = current->next;
        StatusCondition* condition = (StatusCondition*) current->data;

        if (condition != NULL) {
            if (condition->turnsRemaining > 0) {
                condition->turnsRemaining--;

                if (condition->turnsRemaining == 0) {
                    free(condition);
                    removeNode(list, current);

                    if (list->head == NULL) {
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
    
    if (hasStatusCondition(statusList, STATUS_SLEEP)) {
        return 0;
    }

    if (hasStatusCondition(statusList, STATUS_PARALYSIS)) {
        int roll = rand() % 100;
        if (roll < PARALYSIS_SKIP_CHANCE) {
            return 0;
        }
    }
    
    return 1;
}

float getStrengthModifier(StatusList* statusList) {
    float modifier = 1.0f;
    
    if (statusList == NULL) return modifier;
    
    if (hasStatusCondition(statusList, STATUS_BURN)) {
        modifier -= BURN_STRENGTH_REDUCTION;
    }

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
    
    if (hasStatusCondition(statusList, STATUS_BLIND)) {
        modifier -= BLIND_ACCURACY_REDUCTION;
    }
    
    if (modifier < 0.1f) modifier = 0.1f;
    
    return modifier;
}

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
