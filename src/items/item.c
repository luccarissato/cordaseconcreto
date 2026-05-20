#include "item.h"
#include "inventory.h"
#include <stdio.h>
#include <stdlib.h>

#include "../entities/player.h"
#include "../entities/enemy.h"
#include "../entities/stats.h"
#include "../entities/status_condition.h"
#include "../utils/list.h"

/* =============================================================================
 * FUNÇÕES AUXILIARES (ESTÁTICAS)
 * ============================================================================= */
static void clampHP(Stats* stats) {
    if (stats->currentHP > stats->maxHP) {
        stats->currentHP = stats->maxHP;
    }
    if (stats->currentHP < 0) {
        stats->currentHP = 0;
    }
}

static void clampMana(Stats* stats) {
    if (stats->currentMana > stats->maxMana) {
        stats->currentMana = stats->maxMana;
    }
    if (stats->currentMana < 0) {
        stats->currentMana = 0;
    }
}

static void applyItemEffectGeneric(Stats* stats, StatusList* statusList, Item* base) {
    switch (base->type) {
        case ITEM_HEAL:
        case ITEM_MANA: {
            stats->currentHP += base->hpRestore;
            stats->currentMana += base->manaRestore;
            clampHP(stats);
            clampMana(stats);
            break;
        }
        case ITEM_BUFF:
        case ITEM_INFLICT_STATUS: {
            addStatusCondition(statusList, base->statusToApply, base->statusDuration, base->statusIntensity);
            break;
        }
        case ITEM_CURE: {
            switch (base->cureType) {
                case CURE_SPECIFIC:
                    removeStatusCondition(statusList, base->statusToCure);
                    break;
                case CURE_ALL_DEBUFFS:
                    removeAllDebuffs(statusList);
                    break;
                case CURE_ALL:
                    clearAllStatus(statusList);
                    break;
            }
            break;
        }
        case ITEM_REVIVE:
        case ITEM_STAT_BOOST:
        case ITEM_KEY:
        default:
            break;
    }
}

void useItem(Player* target, InventoryItem* item) {
    if (target == NULL || item == NULL || item->baseItem == NULL) {
        return;
    }
    
    Item* base = item->baseItem;
    
    applyItemEffectGeneric(&target->stats, &target->statusList, base);
    
    if (base->type == ITEM_STAT_BOOST) {
        applyStatBoost(target, base);
    }
    
    if (base->type == ITEM_REVIVE) {
        if (target->stats.currentHP <= 0) {
            int hpToRestore = (int)(target->stats.maxHP * base->reviveHPPercent);
            if (hpToRestore < 1) hpToRestore = 1;
            target->stats.currentHP = hpToRestore;
            clampHP(&target->stats);
            clampMana(&target->stats);
            clearAllStatus(&target->statusList);
        }
    }
}

void consumeItem(Inventory* inv, ListNode* node, Player* target) {
    if (node == NULL || inv == NULL || target == NULL) {
        return;
    }
    
    InventoryItem* invItem = (InventoryItem*) node->data;
    
    if (invItem == NULL) {
        return;
    }
    
    if (invItem->baseItem->type == ITEM_KEY) {
        return;
    }
    
    useItem(target, invItem);
    
    invItem->quantity--;
    
    if (invItem->quantity <= 0) {
        removeNode(&inv->items, node);
        
        free(invItem);
    }
}

int canUseItem(Player* target, InventoryItem* item) {
    if (target == NULL || item == NULL || item->baseItem == NULL) {
        return 0;
    }
    
    Item* base = item->baseItem;
    
    switch (base->type) {
        case ITEM_REVIVE:
            return (target->stats.currentHP <= 0);
            
        case ITEM_HEAL:
            /* Item de cura só faz sentido se HP não está cheio */
            /* Mas permitimos usar mesmo cheio (alguns itens curam status também) */
            return 1;
            
        case ITEM_MANA:
            /* Item de mana só faz sentido se mana não está cheia */
            return 1;
            
        case ITEM_CURE:
            if (base->cureType == CURE_SPECIFIC) {
                return hasStatusCondition(&target->statusList, base->statusToCure);
            }
            return 1;
            
        default:
            return 1;
    }
}

void useItemOnStatusList(StatusList* statusList, Item* item) {
    if (statusList == NULL || item == NULL) {
        return;
    }
    
    switch (item->type) {
        case ITEM_BUFF:
        case ITEM_INFLICT_STATUS:
            addStatusCondition(
                statusList,
                item->statusToApply,
                item->statusDuration,
                item->statusIntensity
            );
            break;
            
        case ITEM_CURE:
            switch (item->cureType) {
                case CURE_SPECIFIC:
                    removeStatusCondition(statusList, item->statusToCure);
                    break;
                case CURE_ALL_DEBUFFS:
                    removeAllDebuffs(statusList);
                    break;
                case CURE_ALL:
                    clearAllStatus(statusList);
                    break;
            }
            break;
            
        default:
            break;
    }
}

void applyStatBoost(Player* target, Item* item) {
    if (target == NULL || item == NULL) {
        return;
    }
    
    /* Aplica o boost no atributo especificado */
    switch (item->statBoostType) {
        case BOOST_HP:
            /* Aumenta HP base, que afeta maxHP */
            target->stats.baseHP += item->statBoostValue;
            break;
            
        case BOOST_MANA:
            /* Aumenta Mana base, que afeta maxMana */
            target->stats.baseMana += item->statBoostValue;
            break;
            
        case BOOST_FORTITUDE:
            /* Aumenta Fortitude (afeta maxHP indiretamente) */
            target->stats.fortitude += item->statBoostValue;
            break;
            
        case BOOST_MENTE:
            /* Aumenta Mente (afeta maxMana indiretamente) */
            target->stats.mente += item->statBoostValue;
            break;
            
        case BOOST_FORCA:
            /* Aumenta Força (dano físico) */
            target->stats.forca += item->statBoostValue;
            break;
            
        case BOOST_DEFESA:
            /* Aumenta Defesa (redução de dano) */
            target->stats.defesa += item->statBoostValue;
            break;
            
        case BOOST_VELOCIDADE:
            /* Aumenta Velocidade (ordem de turno) */
            target->stats.velocidade += item->statBoostValue;
            break;
    }
    
    int currentHP = target->stats.currentHP;
    int currentMana = target->stats.currentMana;
    
    calculateStats(&target->stats);
    
    target->stats.currentHP = currentHP;
    target->stats.currentMana = currentMana;

    clampHP(&target->stats);
    clampMana(&target->stats);
}

int revivePlayer(Player* target, float hpPercent) {
    if (target == NULL) {
        return 0;
    }
    
    if (target->stats.currentHP > 0) {
        return 0;
    }
    
    if (hpPercent < 0.1f) hpPercent = 0.1f;
    if (hpPercent > 1.0f) hpPercent = 1.0f;
    
    int hpToRestore = (int)(target->stats.maxHP * hpPercent);
    if (hpToRestore < 1) hpToRestore = 1;
    
    target->stats.currentHP = hpToRestore;
    
    clearAllStatus(&target->statusList);
    
    return 1;
}

void useItemOnEnemy(Enemy* target, InventoryItem* item) {
    if (target == NULL || item == NULL || item->baseItem == NULL) {
        return;
    }
    
    Item* base = item->baseItem;
    
    if (base->type != ITEM_REVIVE && base->type != ITEM_STAT_BOOST) {
        applyItemEffectGeneric(&target->stats, &target->statusList, base);
    }
}
