#ifndef ABILITY_H
#define ABILITY_H

#include "../entities/player.h"
#include "../entities/enemy.h"
#include "../entities/status_condition.h"

typedef enum {
    TARGET_SINGLE_ENEMY,        /* Ataque em inimigo único */
    TARGET_SINGLE_ALLY,         /* Efeito em aliado único */
    TARGET_AREA_ENEMIES,        /* Ataque em todos inimigos */
    TARGET_AREA_ALLIES,         /* Efeito em todos aliados */
    TARGET_SELF                 /* Efeito no próprio personagem */
} TargetType;

typedef enum {
    ABILITY_TYPE_DAMAGE,        /* Dano direto */
    ABILITY_TYPE_HEAL,          /* Cura */
    ABILITY_TYPE_BUFF,          /* Buff positivo */
    ABILITY_TYPE_DEBUFF,        /* Debuff negativo */
    ABILITY_TYPE_CLEANSE,       /* Remove debuffs */
    ABILITY_TYPE_REVIVE,        /* Ressuscita */
    ABILITY_TYPE_SPECIAL        /* Habilidades especiais */
} AbilityType;

typedef enum {
    SCALING_NONE,
    SCALING_FORCA,
    SCALING_MENTE,
    SCALING_DEFESA,
    SCALING_VELOCIDADE
} ScalingType;

typedef struct {
    char name[32];
    char description[256];
    int level_unlocked;
    int mana_cost;
    TargetType target_type;
    AbilityType ability_type;
    float damage_base;
    ScalingType scaling_type;
    float scaling_mult;
    
    /* Dados específicos da habilidade (uso genérico) */
    union {
        struct {
            StatusType status_type;
            int duration;
            float intensity;
        } status;
        struct {
            float heal_amount;
            float heal_scaling;
        } heal;
        struct {
            int max_targets;
            float aoe_radius;
        } area;
        struct {
            float aggro_duration;
        } taunt;
        struct {
            int reflect_duration;
            float reflect_mult;
        } reflect;
        struct {
            int turns_extra;
            int speed_bonus;
        } buff_action;
        struct {
            float revive_hp_percent;
        } revive;
    } data;
} Ability;

typedef struct {
    Ability* ability;
    int cooldown;
    int charges;
} AbilityState;

typedef enum {
    CHARACTER_1_TANK,
    CHARACTER_2_DPS,
    CHARACTER_3_HEALER,
    CHARACTER_4_MAGE,
    CHARACTER_COUNT
} CharacterID;

//initPlayerAbilities - Inicializa as habilidades de um personagem
void initPlayerAbilities(Player* player, CharacterID characterID);

// initAbility - Inicializa uma habilidade manualmente
void initAbility(Ability* ability, const char* name, int level_unlocked, int mana_cost);

// canUseAbility - Verifica se a habilidade pode ser usada
int canUseAbility(Player* player, int ability_index);

// useAbility - Executa uma habilidade
void useAbility(
    Player* caster,
    int ability_index,
    void* targets,
    int target_count,
    int* target_indices,
    int selected_count
);

// getAbilityDamage - Calcula o dano da habilidade
float getAbilityDamage(Ability* ability, Player* caster);

// getAbilityHeal - Calcula a cura da habilidade
float getAbilityHeal(Ability* ability, Player* caster);

// getAbilityByIndex - Obtém uma habilidade por personagem e índice
Ability* getAbilityByIndex(int characterID, int abilityIndex);

#endif 
