#include "item.h"
#include "inventory.h"
#include <stdio.h>
#include <stdlib.h>

#include "../entities/player.h"
#include "../utils/list.h"

void useItem(Player* target, InventoryItem* item) {
    Item* base = item->baseItem;

    target->stats.currentHP += base->hpRestore;

    if (target->stats.currentHP > target->stats.maxHP) {
        target->stats.currentHP = target->stats.maxHP;
    }

    target->stats.currentMana += base->manaRestore;

    if (target->stats.currentMana > target->stats.maxMana) {
        target->stats.currentMana = target->stats.maxMana;
    }
}

void consumeItem(Inventory* inv, ListNode* node, Player* target) {
    if (node == NULL) return;

    InventoryItem* invItem = (InventoryItem*) node->data;

    useItem(target, invItem);
    invItem->quantity--;

    if (invItem->quantity <= 0) {
        removeNode(&inv->items, node);
        free(invItem);
    }
}