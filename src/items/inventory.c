#include "inventory.h"

void initInventory(Inventory* inv) {
    initList(&inv->items);
}

void addItemInventory(Inventory* inv, InventoryItem* item) {
    pushBack(&inv->items, item);
}

void removeItemInventory(Inventory* inv, ListNode* node) {
    removeNode(&inv->items, node);
}

