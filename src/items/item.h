/**
 * =============================================================================
 * ITEM.H - Sistema de Itens do Jogo
 * =============================================================================
 * 
 * Este arquivo define a estrutura base de itens e suas funções de uso.
 * Os itens são organizados por tipos que determinam seu comportamento
 * quando consumidos pelo jogador.
 * 
 * TIPOS DE ITENS:
 * - ITEM_HEAL: Restaura HP do personagem
 * - ITEM_MANA: Restaura Mana do personagem
 * - ITEM_BUFF: Aplica um buff (condição positiva) ao personagem
 * - ITEM_CURE: Remove condições de status negativas (debuffs)
 * - ITEM_REVIVE: Revive um personagem derrotado (HP = 0)
 * - ITEM_STAT_BOOST: Aumenta permanentemente um atributo
 * - ITEM_INFLICT_STATUS: Aplica uma condição de status a um alvo (para combate)
 * 
 * O sistema utiliza a Lista Encadeada (List) para gerenciar o inventário,
 * permitindo adição, remoção e busca eficientes de itens.
 * 
 * =============================================================================
 */

#ifndef ITEM_H
#define ITEM_H

#include "../entities/status_condition.h"
#include "../utils/list.h"

/* Forward declaration para evitar dependência circular com player.h */
/* A estrutura completa é incluída em item.c */
struct Player;
typedef struct Player Player;

/* -----------------------------------------------------------------------------
 * ENUMERAÇÃO: ItemType
 * -----------------------------------------------------------------------------
 * Define todos os tipos de itens disponíveis no jogo.
 * Cada tipo determina como o item será processado quando usado.
 * -------------------------------------------------------------------------- */
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

/* -----------------------------------------------------------------------------
 * ENUMERAÇÃO: StatBoostType
 * -----------------------------------------------------------------------------
 * Define qual atributo será aumentado por itens do tipo ITEM_STAT_BOOST.
 * -------------------------------------------------------------------------- */
typedef enum {
    BOOST_HP,               /* Aumenta HP base */
    BOOST_MANA,             /* Aumenta Mana base */
    BOOST_FORTITUDE,        /* Aumenta Fortitude */
    BOOST_MENTE,            /* Aumenta Mente */
    BOOST_FORCA,            /* Aumenta Força */
    BOOST_DEFESA,           /* Aumenta Defesa */
    BOOST_VELOCIDADE,       /* Aumenta Velocidade */
} StatBoostType;

/* -----------------------------------------------------------------------------
 * ENUMERAÇÃO: CureType
 * -----------------------------------------------------------------------------
 * Define o escopo da cura para itens do tipo ITEM_CURE.
 * -------------------------------------------------------------------------- */
typedef enum {
    CURE_SPECIFIC,          /* Cura uma condição específica */
    CURE_ALL_DEBUFFS,       /* Cura todas as condições negativas */
    CURE_ALL,               /* Cura todas as condições (buffs e debuffs) */
} CureType;

/* -----------------------------------------------------------------------------
 * ESTRUTURA: Item
 * -----------------------------------------------------------------------------
 * Representa a definição base de um item no jogo.
 * Contém todas as informações necessárias para processar o uso do item.
 * 
 * Campos:
 *   - name: Nome do item exibido ao jogador (máx 32 caracteres)
 *   - description: Descrição do efeito do item (máx 128 caracteres)
 *   - type: Tipo do item que determina seu comportamento
 *   - hpRestore: Quantidade de HP restaurada (para ITEM_HEAL e ITEM_REVIVE)
 *   - manaRestore: Quantidade de Mana restaurada (para ITEM_MANA)
 *   - statusToApply: Condição de status aplicada (para ITEM_BUFF e ITEM_INFLICT_STATUS)
 *   - statusDuration: Duração em turnos da condição aplicada
 *   - statusIntensity: Intensidade do efeito da condição
 *   - statusToCure: Condição específica a curar (para ITEM_CURE com CURE_SPECIFIC)
 *   - cureType: Escopo da cura (para ITEM_CURE)
 *   - statBoostType: Atributo a aumentar (para ITEM_STAT_BOOST)
 *   - statBoostValue: Valor do aumento permanente
 *   - reviveHPPercent: Porcentagem do HP máximo restaurada ao reviver (0.0 a 1.0)
 * -------------------------------------------------------------------------- */
typedef struct {
    char name[32];              /* Nome do item */
    char description[128];      /* Descrição do efeito */
    ItemType type;              /* Tipo do item */
    
    /* === Campos para ITEM_HEAL e ITEM_MANA === */
    int hpRestore;              /* HP restaurado */
    int manaRestore;            /* Mana restaurada */
    
    /* === Campos para ITEM_BUFF e ITEM_INFLICT_STATUS === */
    StatusType statusToApply;   /* Condição de status a aplicar */
    int statusDuration;         /* Duração em turnos (-1 = permanente) */
    float statusIntensity;      /* Intensidade do efeito (1.0 = normal) */
    
    /* === Campos para ITEM_CURE === */
    StatusType statusToCure;    /* Condição específica a curar (se CURE_SPECIFIC) */
    CureType cureType;          /* Escopo da cura */
    
    /* === Campos para ITEM_STAT_BOOST === */
    StatBoostType statBoostType;/* Atributo a aumentar */
    int statBoostValue;         /* Valor do aumento */
    
    /* === Campos para ITEM_REVIVE === */
    float reviveHPPercent;      /* Porcentagem do HP máximo ao reviver (0.5 = 50%) */
} Item;

/* -----------------------------------------------------------------------------
 * ESTRUTURA: InventoryItem
 * -----------------------------------------------------------------------------
 * Representa um item no inventário do jogador.
 * Associa um item base com sua quantidade em posse.
 * 
 * Campos:
 *   - baseItem: Ponteiro para a definição do item
 *   - quantity: Quantidade atual no inventário
 * -------------------------------------------------------------------------- */
typedef struct {
    Item* baseItem;             /* Ponteiro para o item base */
    int quantity;               /* Quantidade no inventário */
} InventoryItem;

/* Forward declaration para evitar dependência circular */
struct Inventory;

/* =============================================================================
 * FUNÇÕES DE USO DE ITENS
 * ============================================================================= */

/**
 * useItem - Aplica os efeitos de um item em um jogador
 * @param target: Jogador que receberá os efeitos do item
 * @param item: Item a ser usado
 * 
 * Processa o item de acordo com seu tipo:
 * - ITEM_HEAL: Restaura HP
 * - ITEM_MANA: Restaura Mana
 * - ITEM_BUFF: Aplica condição positiva
 * - ITEM_CURE: Remove condições negativas
 * - ITEM_REVIVE: Revive se HP = 0
 * - ITEM_STAT_BOOST: Aumenta atributo permanentemente
 */
void useItem(Player* target, InventoryItem* item);

/**
 * consumeItem - Usa um item e decrementa sua quantidade no inventário
 * @param inv: Inventário contendo o item
 * @param node: Nó da lista que contém o InventoryItem
 * @param target: Jogador que receberá os efeitos
 * 
 * Após usar o item, decrementa a quantidade.
 * Se a quantidade chegar a 0, remove o item do inventário.
 */
void consumeItem(struct Inventory* inv, ListNode* node, Player* target);

/**
 * canUseItem - Verifica se um item pode ser usado no alvo
 * @param target: Jogador alvo
 * @param item: Item a ser verificado
 * @return: 1 se pode usar, 0 se não pode
 * 
 * Verifica condições como:
 * - Item de revive só funciona se HP = 0
 * - Item de cura só funciona se HP < máximo
 * - Item de mana só funciona se Mana < máximo
 */
int canUseItem(Player* target, InventoryItem* item);

/**
 * useItemOnStatusList - Aplica efeitos de item em uma StatusList
 * @param statusList: Lista de status do alvo
 * @param item: Item a ser usado
 * 
 * Usado para aplicar buffs ou curar status sem precisar
 * de referência completa ao Player.
 */
void useItemOnStatusList(StatusList* statusList, Item* item);

/**
 * applyStatBoost - Aplica aumento permanente de atributo
 * @param target: Jogador que receberá o boost
 * @param item: Item de stat boost
 * 
 * Aumenta permanentemente o atributo especificado e recalcula stats.
 */
void applyStatBoost(Player* target, Item* item);

/**
 * revivePlayer - Revive um jogador com HP = 0
 * @param target: Jogador a ser revivido
 * @param hpPercent: Porcentagem do HP máximo a restaurar (0.0 a 1.0)
 * @return: 1 se reviveu com sucesso, 0 se não precisava reviver
 * 
 * Só funciona se o jogador estiver com HP = 0.
 */
int revivePlayer(Player* target, float hpPercent);

#endif /* ITEM_H */
