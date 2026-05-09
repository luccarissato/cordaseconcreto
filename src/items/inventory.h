#ifndef INVENTORY_H
#define INVENTORY_H

#include "../utils/list.h"
#include "item.h"

typedef struct {
    List items;
} Inventory;

void initInventory(Inventory* inv);

void addItemInventory(Inventory* inv, InventoryItem* item);

void removeItemInventory(Inventory* inv, ListNode* node);

#endif