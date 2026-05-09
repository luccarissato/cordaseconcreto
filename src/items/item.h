#ifndef ITEM_H
#define ITEM_H

struct Player;
struct Inventory;
struct ListNode;

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

void useItem(struct Player* target,Item* item);
void consumeItem(struct Inventory* inv, struct ListNode* node, struct Player* target);

#endif