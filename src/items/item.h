#ifndef ITEM_H
#define ITEM_H

#include "../entities/player.h"
#include "../utils/list.h"

typedef enum {
    ITEM_HEAL,
    ITEM_MANA,
    ITEM_BUFF,
    ITEM_CURE,
} ItemType;

typedef struct {
    char name[32];
    ItemType type;
    int hpRestore;
    int manaRestore;
} Item;

typedef struct {
    Item* baseItem;
    int quantity;
} InventoryItem;

struct Inventory;

void useItem(Player* target, InventoryItem* item);
void consumeItem(struct Inventory* inv, ListNode* node, Player* target);

#endif