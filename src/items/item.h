#ifndef ITEM_H
#define ITEM_H

#include "../entities/status_condition.h"
#include "../utils/list.h"

/* Forward declaration para evitar dependência circular com player.h */
struct Player;
typedef struct Player Player;

typedef enum {
    ITEM_HEAL,              /* Item de cura de HP */
    ITEM_MANA,              /* Item de recuperação de Mana */
    ITEM_BUFF,              /* Item que aplica buff (condição positiva) */
    ITEM_CURE,              /* Item que cura condições negativas (debuffs) */
    ITEM_REVIVE,            /* Item que revive personagem com HP = 0 */
    ITEM_STAT_BOOST,        /* Item que aumenta atributos permanentemente */
    ITEM_INFLICT_STATUS,    /* Item que aplica condição de status em combate */
    ITEM_KEY,               /* Item-chave não consumível */
} ItemType;

typedef enum {
    BOOST_HP,               /* Aumenta HP base */
    BOOST_MANA,             /* Aumenta Mana base */
    BOOST_FORTITUDE,        /* Aumenta Fortitude */
    BOOST_MENTE,            /* Aumenta Mente */
    BOOST_FORCA,            /* Aumenta Força */
    BOOST_DEFESA,           /* Aumenta Defesa */
    BOOST_VELOCIDADE,       /* Aumenta Velocidade */
} StatBoostType;

typedef enum {
    CURE_SPECIFIC,          /* Cura uma condição específica */
    CURE_ALL_DEBUFFS,       /* Cura todas as condições negativas */
    CURE_ALL,               /* Cura todas as condições (buffs e debuffs) */
} CureType;

typedef struct {
    char name[32];              /* Nome do item */
    char description[128];      /* Descrição do efeito */
    ItemType type;              /* Tipo do item */
    

    int hpRestore;              /* HP restaurado */
    int manaRestore;            /* Mana restaurada */
    
 
    StatusType statusToApply;   /* Condição de status a aplicar */
    int statusDuration;         /* Duração em turnos (-1 = permanente) */
    float statusIntensity;      /* Intensidade do efeito (1.0 = normal) */
    

    StatusType statusToCure;    /* Condição específica a curar (se CURE_SPECIFIC) */
    CureType cureType;          /* Escopo da cura */
    

    StatBoostType statBoostType;/* Atributo a aumentar */
    int statBoostValue;         /* Valor do aumento */
    

    float reviveHPPercent;      /* Porcentagem do HP máximo ao reviver (0.5 = 50%) */
} Item;

typedef struct {
    Item* baseItem;             /* Ponteiro para o item base */
    int quantity;               /* Quantidade no inventário */
} InventoryItem;

/* Forward declaration para evitar dependência circular */
struct Inventory;

// * useItem - Aplica os efeitos de um item em um jogador
void useItem(Player* target, InventoryItem* item);

// * consumeItem - Usa um item e decrementa sua quantidade no inventário
void consumeItem(struct Inventory* inv, ListNode* node, Player* target);

// * canUseItem - Verifica se um item pode ser usado no alvo
int canUseItem(Player* target, InventoryItem* item);

// * useItemOnStatusList - Aplica efeitos de item em uma StatusList
void useItemOnStatusList(StatusList* statusList, Item* item);

// * applyStatBoost - Aplica aumento permanente de atributo
void applyStatBoost(Player* target, Item* item);

// * revivePlayer - Revive um jogador com HP = 0
int revivePlayer(Player* target, float hpPercent);

#endif
