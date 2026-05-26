#include "ability.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include "../core/game.h"
#include "../entities/status_condition.h"
#include "boss_ai.h"
#include "combat.h"

extern Player party[PARTY_SIZE];


static Ability abilities[4][4];
static int abilitiesInitialized = 0;

// initCharacter1Abilities - Inicializa habilidades do Tanque/Suporte
static void initCharacter1Abilities() {
    /* Skill 1: Baque - Dano físico baixo com cura baseada no dano causado */
    abilities[0][0].level_unlocked = 1;
    abilities[0][0].mana_cost = 20;
    abilities[0][0].target_type = TARGET_SINGLE_ENEMY;
    abilities[0][0].ability_type = ABILITY_TYPE_DAMAGE;
    abilities[0][0].damage_base = 35;
    abilities[0][0].scaling_type = SCALING_FORCA;
    abilities[0][0].scaling_mult = 0.3f;
    abilities[0][0].characterID = CHARACTER_1_TANK;
    abilities[0][0].ability_index = 0;
    strcpy(abilities[0][0].name, "Baque de cura");
    strcpy(abilities[0][0].description, "Ataque fisico leve que cura voce com base no dano causado");
    
    /* Skill 2: Protect - Absorve dano de aliado */
    abilities[0][1].level_unlocked = 2;
    abilities[0][1].mana_cost = 30;
    abilities[0][1].target_type = TARGET_SINGLE_ALLY;
    abilities[0][1].ability_type = ABILITY_TYPE_BUFF;
    abilities[0][1].damage_base = 0;
    abilities[0][1].scaling_type = SCALING_NONE;
    strcpy(abilities[0][1].name, "Proteçao Ritmica");
    strcpy(abilities[0][1].description, "Absorve dano de um aliado por 3 turnos");
    abilities[0][1].data.buff_action.turns_extra = 3;
    
    /* Skill 3: Debilitating Strike - Dano físico + debuffs */
    abilities[0][2].level_unlocked = 3;
    abilities[0][2].mana_cost = 25;
    abilities[0][2].target_type = TARGET_AREA_ENEMIES;
    abilities[0][2].ability_type = ABILITY_TYPE_DAMAGE;
    abilities[0][2].damage_base = 25;
    abilities[0][2].scaling_type = SCALING_FORCA;
    abilities[0][2].scaling_mult = 0.8f;
    strcpy(abilities[0][2].name, "Loa de Respeito");
    strcpy(abilities[0][2].description, "Dano em área + reduz força e velocidade por 3 turnos");
    
    /* Skill 4: Reduce Damage - Reduz dano e reflete */
    abilities[0][3].level_unlocked = 4;
    abilities[0][3].mana_cost = 35;
    abilities[0][3].target_type = TARGET_SELF;
    abilities[0][3].ability_type = ABILITY_TYPE_BUFF;
    abilities[0][3].damage_base = 0;
    abilities[0][3].scaling_type = SCALING_NONE;
    strcpy(abilities[0][3].name, "A Marcha da Folia");
    strcpy(abilities[0][3].description, "Reduz dano e reflete parte como true damage por 3 turnos");
    abilities[0][3].data.reflect.reflect_duration = 3;
    abilities[0][3].data.reflect.reflect_mult = 1.0f;
}


 // initCharacter2Abilities - Inicializa habilidades do DPS Físico
static void initCharacter2Abilities() {
    /* Skill 1: Triple Strike - 3 hits com chance de debuff */
    abilities[1][0].level_unlocked = 1;
    abilities[1][0].mana_cost = 15;
    abilities[1][0].target_type = TARGET_SINGLE_ENEMY;
    abilities[1][0].ability_type = ABILITY_TYPE_DAMAGE;
    abilities[1][0].damage_base = 20;
    abilities[1][0].scaling_type = SCALING_FORCA;
    abilities[1][0].scaling_mult = 0.5f;
    strcpy(abilities[1][0].name, "Triplice Parabolica");
    strcpy(abilities[1][0].description, "3 ataques rápidos. 50% de chance de reduzir defesa");
    abilities[1][0].data.area.max_targets = 3;
    
    /* Skill 2: Bleed - Aplica sangramento */
    abilities[1][1].level_unlocked = 2;
    abilities[1][1].mana_cost = 20;
    abilities[1][1].target_type = TARGET_SINGLE_ENEMY;
    abilities[1][1].ability_type = ABILITY_TYPE_DAMAGE;
    abilities[1][1].damage_base = 35;
    abilities[1][1].scaling_type = SCALING_FORCA;
    abilities[1][1].scaling_mult = 0.7f;
    strcpy(abilities[1][1].name, "Corte de Carangueijo");
    strcpy(abilities[1][1].description, "Ataque físico que aplica sangramento escalável");
    abilities[1][1].data.status.status_type = STATUS_BLEED;
    abilities[1][1].data.status.duration = -1;
    abilities[1][1].data.status.intensity = 0;
    
    /* Skill 3: Counter - Devolve dano recebido */
    abilities[1][2].level_unlocked = 3;
    abilities[1][2].mana_cost = 30;
    abilities[1][2].target_type = TARGET_SELF;
    abilities[1][2].ability_type = ABILITY_TYPE_BUFF;
    abilities[1][2].damage_base = 0;
    abilities[1][2].scaling_type = SCALING_NONE;
    strcpy(abilities[1][2].name, "Banditismo por Necessidade");
    strcpy(abilities[1][2].description, "Devolve 50% do dano recebido. Aumenta sangramento em 1");
    
    /* Skill 4: Haste - Extra turno + buff velocidade */
    abilities[1][3].level_unlocked = 4;
    abilities[1][3].mana_cost = 30;
    abilities[1][3].target_type = TARGET_SELF;
    abilities[1][3].ability_type = ABILITY_TYPE_BUFF;
    abilities[1][3].damage_base = 0;
    abilities[1][3].scaling_type = SCALING_NONE;
    strcpy(abilities[1][3].name, "A Praieira");
    strcpy(abilities[1][3].description, "Ganha turno extra + 5 de velocidade por 3 turnos");
    abilities[1][3].data.buff_action.turns_extra = 1;
    abilities[1][3].data.buff_action.speed_bonus = 5;
}

//initCharacter3Abilities - Inicializa habilidades da Healer/Suporte
static void initCharacter3Abilities() {
    /* Skill 1: Cleanse - Remove debuffs */
    abilities[2][0].level_unlocked = 1;
    abilities[2][0].mana_cost = 20;
    abilities[2][0].target_type = TARGET_SINGLE_ALLY;
    abilities[2][0].ability_type = ABILITY_TYPE_CLEANSE;
    abilities[2][0].damage_base = 0;
    abilities[2][0].scaling_type = SCALING_NONE;
    strcpy(abilities[2][0].name, "Vento de Itamaracá");
    strcpy(abilities[2][0].description, "Remove todos os debuffs de um aliado");
    
    /* Skill 2: Group Heal - Cura em grupo */
    abilities[2][1].level_unlocked = 2;
    abilities[2][1].mana_cost = 25;
    abilities[2][1].target_type = TARGET_AREA_ALLIES;
    abilities[2][1].ability_type = ABILITY_TYPE_HEAL;
    abilities[2][1].damage_base = 0;
    abilities[2][1].scaling_type = SCALING_MENTE;
    abilities[2][1].scaling_mult = 0.8f;
    strcpy(abilities[2][1].name, "Passo da Ciranda");
    strcpy(abilities[2][1].description, "Cura pequena em todos os aliados");
    abilities[2][1].data.heal.heal_amount = 30;
    abilities[2][1].data.heal.heal_scaling = 0.8f;
    
    /* Skill 3: Target Buff - Buff alternável */
    abilities[2][2].level_unlocked = 3;
    abilities[2][2].mana_cost = 30;
    abilities[2][2].target_type = TARGET_SINGLE_ALLY;
    abilities[2][2].ability_type = ABILITY_TYPE_BUFF;
    abilities[2][2].damage_base = 0;
    abilities[2][2].scaling_type = SCALING_NONE;
    strcpy(abilities[2][2].name, "O Balanço das Ondas >");
    strcpy(abilities[2][2].description, "Buff alternável: +10% dano por 3 turnos.\n\nPressione setas para trocar o efeito.");
    abilities[2][2].is_alternatable = 1;
    abilities[2][2].current_buff_mode = BUFF_MODE_DAMAGE;  /* Começa em dano */
    
    /* Skill 4: Revive - Ressuscita aliado */
    abilities[2][3].level_unlocked = 4;
    abilities[2][3].mana_cost = 50;
    abilities[2][3].target_type = TARGET_SINGLE_ALLY;
    abilities[2][3].ability_type = ABILITY_TYPE_REVIVE;
    abilities[2][3].damage_base = 0;
    abilities[2][3].scaling_type = SCALING_NONE;
    strcpy(abilities[2][3].name, "Quem me deu foi Lia");
    strcpy(abilities[2][3].description, "Ressuscita aliado com 33% de HP");
    abilities[2][3].data.revive.revive_hp_percent = 0.33f;
}


// initCharacter4Abilities - Inicializa habilidades do Mago Elemental
static void initCharacter4Abilities() {
    /* Skill 1: Single Element Spell - Alvo único com troca de elemento */
    abilities[3][0].level_unlocked = 1;
    abilities[3][0].mana_cost = 18;
    abilities[3][0].target_type = TARGET_SINGLE_ENEMY;
    abilities[3][0].ability_type = ABILITY_TYPE_DAMAGE;
    abilities[3][0].damage_base = 50;
    abilities[3][0].scaling_type = SCALING_MENTE;
    abilities[3][0].scaling_mult = 0.0f;
    strcpy(abilities[3][0].name, "Rima dos Quatro Cantos >");
    strcpy(abilities[3][0].description, "Ataque elemental em alvo único. Elemento: Calor\n\nPressione setas para trocar elemento");
    abilities[3][0].is_alternatable = 1;
    abilities[3][0].current_element = ELEMENT_HEAT;  /* Começa em Calor */
    
    /* Skill 2: Arcane Mark - Marca para próximo dano 2x */
    abilities[3][1].level_unlocked = 2;
    abilities[3][1].mana_cost = 15;
    abilities[3][1].target_type = TARGET_SINGLE_ENEMY;
    abilities[3][1].ability_type = ABILITY_TYPE_DEBUFF;
    abilities[3][1].damage_base = 0;
    abilities[3][1].scaling_type = SCALING_NONE;
    strcpy(abilities[3][1].name, "Ponto de Marração");
    strcpy(abilities[3][1].description, "Marca alvo. Próximo dano elemental recebe 2x multiplicador");
    abilities[3][1].data.status.status_type = STATUS_ARCANE_MARK;
    abilities[3][1].data.status.duration = 3;
    abilities[3][1].data.status.intensity = 1.0f;
    
    /* Skill 3: Area Element Spell - Área com troca de elemento */
    abilities[3][2].level_unlocked = 3;
    abilities[3][2].mana_cost = 30;
    abilities[3][2].target_type = TARGET_AREA_ENEMIES;
    abilities[3][2].ability_type = ABILITY_TYPE_DAMAGE;
    abilities[3][2].damage_base = 50;
    abilities[3][2].scaling_type = SCALING_MENTE;
    abilities[3][2].scaling_mult = 0.0f;
    strcpy(abilities[3][2].name, "Ritmo intercostal >");
    strcpy(abilities[3][2].description, "Ataque elemental em todos inimigos. Elemento: Calor\n\nPressione setas para trocar elemento");
    abilities[3][2].is_alternatable = 1;
    abilities[3][2].current_element = ELEMENT_HEAT;  /* Começa em Calor */
    
    /* Skill 4: Permanent Buff - Aumenta dano por turno */
    abilities[3][3].level_unlocked = 4;
    abilities[3][3].mana_cost = 20;
    abilities[3][3].target_type = TARGET_SELF;
    abilities[3][3].ability_type = ABILITY_TYPE_BUFF;
    abilities[3][3].damage_base = 0;
    abilities[3][3].scaling_type = SCALING_NONE;
    strcpy(abilities[3][3].name, "A Balada do Iludido");
    strcpy(abilities[3][3].description, "Buff permanente: +10% dano elemental por round ate morte");
    abilities[3][3].data.buff_action.turns_extra = -1;
}


//initializeAbilitySystem - Inicializa todas as habilidades do jogo
static void initializeAbilitySystem() {
    if (abilitiesInitialized) return;
    
    initCharacter1Abilities();
    initCharacter2Abilities();
    initCharacter3Abilities();
    initCharacter4Abilities();
    
    /* Pós-processamento: configura characterID e ability_index para todas as habilidades */
    for (int charID = 0; charID < CHARACTER_COUNT; charID++) {
        for (int abilityIdx = 0; abilityIdx < 4; abilityIdx++) {
            abilities[charID][abilityIdx].characterID = (CharacterID)charID;
            abilities[charID][abilityIdx].ability_index = abilityIdx;
        }
    }
    
    abilitiesInitialized = 1;
}


//initPlayerAbilities - Inicializa as habilidades de um personagem
void initPlayerAbilities(Player* player, CharacterID characterID) {
    if (player == NULL) return;
    
    initializeAbilitySystem();
    
    /* Preserve existing player level if already set (avoid overwriting during re-inits) */
    if (player->level == 0) {
        player->level = 1;
    }
    player->characterID = characterID;
}


//initAbility - Inicializa uma habilidade manualmente

void initAbility(Ability* ability, const char* name, int level_unlocked, int mana_cost) {
    if (ability == NULL) return;
    
    memset(ability, 0, sizeof(Ability));
    strncpy(ability->name, name, 31);
    ability->level_unlocked = level_unlocked;
    ability->mana_cost = mana_cost;
}


// canUseAbility - Verifica se a habilidade pode ser usada
int canUseAbility(Player* player, int ability_index) {
    if (player == NULL || ability_index < 0 || ability_index >= 4) return 0;
    if (!player->isAlive) return 0;
    
    /* Verifica se está desbloqueada */
    Ability* ability = &abilities[player->characterID][ability_index];
    if (ability->level_unlocked > player->level) return 0;
    
    /* Verifica se tem mana */
    if (player->stats.currentMana < ability->mana_cost) return 0;
    
    return 1;
}


// getAbilityDamage - Calcula o dano da habilidade com scaling
float getAbilityDamage(Ability* ability, Player* caster) {
    if (ability == NULL || caster == NULL) return 0;
    
    float damage = ability->damage_base;
    
    switch (ability->scaling_type) {
        case SCALING_FORCA:
            damage += caster->stats.forca * ability->scaling_mult;
            break;
        case SCALING_MENTE:
            damage += caster->stats.mente * ability->scaling_mult;
            break;
        case SCALING_DEFESA:
            damage += caster->stats.defesa * ability->scaling_mult;
            break;
        case SCALING_VELOCIDADE:
            damage += caster->stats.velocidade * ability->scaling_mult;
            break;
        case SCALING_NONE:
        default:
            break;
    }
    
    return damage;
}


// getAbilityHeal - Calcula a cura da habilidade com scaling
float getAbilityHeal(Ability* ability, Player* caster) {
    if (ability == NULL || caster == NULL) return 0;
    
    float heal = ability->data.heal.heal_amount;
    
    heal += caster->stats.mente * ability->data.heal.heal_scaling;
    
    return heal;
}


//getAbilityByIndex - Obtém habilidade por personagem e índice
Ability* getAbilityByIndex(int characterID, int abilityIndex) {
    if (characterID < 0 || characterID >= 4 || abilityIndex < 0 || abilityIndex >= 4) {
        return NULL;
    }
    
    initializeAbilitySystem();
    return &abilities[characterID][abilityIndex];
}

static void clampPlayerCombatStats(Player* target) {
    if (target == NULL) return;

    if (target->stats.currentHP < 0) {
        target->stats.currentHP = 0;
    }

    if (target->stats.currentHP > target->stats.maxHP) {
        target->stats.currentHP = target->stats.maxHP;
    }

    if (target->stats.currentMana < 0) {
        target->stats.currentMana = 0;
    }

    if (target->stats.currentMana > target->stats.maxMana) {
        target->stats.currentMana = target->stats.maxMana;
    }

    target->isAlive = (target->stats.currentHP > 0);
}

void applyCombatDamageToPlayer(Player* target, int damage) {
    if (target == NULL || !target->isAlive) return;

    // Proteção redirection
    if (target != &party[0] && hasStatusCondition(&target->statusList, STATUS_PROTECT)) {
        if (party[0].isAlive) {
            bossAiQueueMessage("Protecao! Dano em %s redirecionado para %s.", target->name, party[0].name);
            target = &party[0];
        }
    }

    float defenseModifier = getDefenseModifier(&target->statusList);
    int effectiveDefense = (int)(target->stats.defesa * defenseModifier);
    int effectiveDamage = damage - effectiveDefense;

    if (effectiveDamage < 1) effectiveDamage = 1;

    if (target->defenseDamageReductionPending) {
        effectiveDamage = (int)(effectiveDamage * 0.70f);
        if (effectiveDamage < 1) effectiveDamage = 1;
        target->defenseDamageReductionPending = 0;
    }

    // Retaliation (Contra-ataque) & Bastião logic (applied before subtracting HP so reduction is effective)
    Combatant* active = getCurrentCombatant();
    Enemy* attacker = NULL;
    if (active != NULL && active->type == COMBATANT_ENEMY) {
        int idx = combat.enemyIndices[active->enemyIndex];
        if (idx >= 0 && idx < enemyManager.count) {
            attacker = &enemyManager.enemies[idx];
        }
    }

    if (attacker != NULL && attacker->isAlive) {
        if (hasStatusCondition(&target->statusList, STATUS_COUNTER)) {
            int counterDamage = (int)(effectiveDamage * 0.50f);
            if (counterDamage < 1) counterDamage = 1;
            
            // Redirect 50% of the damage: reduce damage taken by 50%, and deal 50% to attacker
            effectiveDamage -= counterDamage;
            if (effectiveDamage < 1) effectiveDamage = 1;
            
            damageEnemy(attacker, counterDamage);
            
            // Consume the counter-attack status (only works on next valid damage instance)
            removeStatusCondition(&target->statusList, STATUS_COUNTER);
            
            bossAiQueueMessage("Contra-ataque! %s redirecionou %d de dano a %s.", target->name, counterDamage, attacker->name);
        }

        if (hasStatusCondition(&target->statusList, STATUS_REFLECT)) {
            // Bastião: reduces damage received by 50%
            effectiveDamage = (int)(effectiveDamage * 0.50f);
            if (effectiveDamage < 1) effectiveDamage = 1;
            
            // Returns the received damage to attacker, ignoring resistances (true damage)
            int reflectDamage = effectiveDamage;
            
            attacker->stats.currentHP -= reflectDamage;
            if (attacker->stats.currentHP <= 0) {
                attacker->stats.currentHP = 0;
                attacker->isAlive = 0;
            }
            bossAiQueueMessage("Bastiao! %s reduziu o dano e refletiu %d de dano real a %s.", target->name, reflectDamage, attacker->name);
        }
    }

    target->stats.currentHP -= effectiveDamage;
    clampPlayerCombatStats(target);
}

static void healPlayer(Player* target, int amount) {
    if (target == NULL || !target->isAlive) return;

    target->stats.currentHP += amount;
    clampPlayerCombatStats(target);
}

static void revivePlayerCombat(Player* target, float hpPercent) {
    if (target == NULL) return;

    if (hpPercent < 0.1f) hpPercent = 0.1f;
    if (hpPercent > 1.0f) hpPercent = 1.0f;

    if (target->stats.currentHP > 0) return;

    int hpToRestore = (int)(target->stats.maxHP * hpPercent);
    if (hpToRestore < 1) hpToRestore = 1;

    target->stats.currentHP = hpToRestore;
    clearAllStatus(&target->statusList);
    clampPlayerCombatStats(target);
}

static void applyStatusToPlayerTarget(Player* target, int targetIndex, StatusType statusType, int turns, float intensity) {
    if (target == NULL || statusType == STATUS_NONE) return;

    /* Defender: bloqueia apenas NOVAS condições sem limpar as existentes */
    if (target->defenseGuardActive) return;

    addStatusCondition(&target->statusList, statusType, turns, intensity);
    bossAiQueuePlayerAfflictedMessage(targetIndex, statusType);
}

static void applyStatusToEnemyTarget(Enemy* target, StatusType statusType, int turns, float intensity) {
    if (target == NULL || statusType == STATUS_NONE) return;
    applyStatusToEnemy(target, statusType, turns, intensity);
}

static int applyDamageToEnemyTarget(Enemy* target, Player* caster, Ability* ability) {
    if (target == NULL || caster == NULL || ability == NULL) return 0;

    int damage = (int)getAbilityDamage(ability, caster);
    
    /* Aplica modificadores de buff de força (STATUS_STRENGTH_UP) se aplicável */
    if (ability->scaling_type == SCALING_FORCA) {
        float strengthMod = getStrengthModifier(&caster->statusList);
        damage = (int)(damage * strengthMod);
    }
    
    return damageEnemy(target, damage);
}

    static int isPhysicalDamageAbility(Player* caster, int ability_index, Ability* ability) {
        if (caster == NULL || ability == NULL) return 0;

        if (ability->ability_type != ABILITY_TYPE_DAMAGE) {
            return 0;
        }

        if (caster->characterID == CHARACTER_4_MAGE && (ability_index == 0 || ability_index == 2)) {
            return (ability->current_element == ELEMENT_NONE) ? 1 : 0;
        }

        return 1;
    }

    static int getCasterPartyIndex(Player* caster) {
        if (caster == NULL) return -1;
        if (caster < party || caster >= party + PARTY_SIZE) return -1;
        return (int)(caster - party);
    }

static void applyDamageToPlayerTarget(Player* target, Player* caster, Ability* ability) {
    if (target == NULL || caster == NULL || ability == NULL) return;

    int damage = (int)getAbilityDamage(ability, caster);
    applyCombatDamageToPlayer(target, damage);
}


// useAbility - Executa uma habilidade
void useAbility(Player* caster, int ability_index, void* targets, int target_count, int* target_indices, int selected_count) {
    if (caster == NULL || !canUseAbility(caster, ability_index)) return;
    
    Ability* ability = getAbilityByIndex(caster->characterID, ability_index);
    if (ability == NULL) return;
    int casterPartyIndex = getCasterPartyIndex(caster);
    int abilityWasUsed = 0;
    
    /* Deduz mana */
    caster->stats.currentMana -= ability->mana_cost;
    if (caster->stats.currentMana < 0) {
        caster->stats.currentMana = 0;
    }
    
    if (ability->ability_type == ABILITY_TYPE_DAMAGE) {
        if (target_indices == NULL || selected_count <= 0) return;

        abilityWasUsed = 1;

        if (ability->target_type == TARGET_AREA_ENEMIES || ability->target_type == TARGET_SINGLE_ENEMY) {
            Enemy* enemyTargets = (Enemy*)targets;
            for (int i = 0; i < selected_count; i++) {
                int targetIndex = target_indices[i];
                if (targetIndex < 0 || targetIndex >= target_count) continue;

                Enemy* target = &enemyTargets[targetIndex];

                if (caster->characterID == CHARACTER_2_DPS && ability_index == 0) {
                    applyDamageToEnemyTarget(target, caster, ability);
                    applyDamageToEnemyTarget(target, caster, ability);
                    applyDamageToEnemyTarget(target, caster, ability);

                    if ((rand() % 100) < 50) {
                        applyStatusToEnemyTarget(target, STATUS_DEFENSE_DOWN, 2, 1.0f);
                    }
                } else if (caster->characterID == CHARACTER_4_MAGE && (ability_index == 0 || ability_index == 2)) {
                    /* Dano elemental que considera resistência */
                    applyElementalDamage(target, caster, ability);
                } else {
                    int damageDealt = applyDamageToEnemyTarget(target, caster, ability);

                    if (caster->characterID == CHARACTER_1_TANK && ability_index == 0 && damageDealt > 0) {
                        healPlayer(caster, damageDealt);
                    }
                }

                if (isPhysicalDamageAbility(caster, ability_index, ability)) {
                    bossAiNotifyPlayerPhysicalAction(getCasterPartyIndex(caster), 1);
                }

                if (ability->data.status.status_type != STATUS_NONE) {
                    applyStatusToEnemyTarget(
                        target,
                        ability->data.status.status_type,
                        ability->data.status.duration,
                        ability->data.status.intensity
                    );
                }

                if (caster->characterID == CHARACTER_1_TANK && ability_index == 2) {
                    applyStatusToEnemyTarget(target, STATUS_WEAKEN, 3, 1.0f);
                    applyStatusToEnemyTarget(target, STATUS_SLOW, 3, 1.0f);
                }
            }
        } else if (ability->target_type == TARGET_AREA_ALLIES || ability->target_type == TARGET_SINGLE_ALLY) {
            Player* playerTargets = (Player*)targets;
            for (int i = 0; i < selected_count; i++) {
                int targetIndex = target_indices[i];
                if (targetIndex < 0 || targetIndex >= target_count) continue;
                applyDamageToPlayerTarget(&playerTargets[targetIndex], caster, ability);
            }
        }
    }

    if (ability->ability_type == ABILITY_TYPE_HEAL) {
        if (target_indices == NULL || selected_count <= 0) return;

        abilityWasUsed = 1;
        Player* playerTargets = (Player*)targets;
        int healAmount = (int)getAbilityHeal(ability, caster);

        for (int i = 0; i < selected_count; i++) {
            int targetIndex = target_indices[i];
            if (targetIndex < 0 || targetIndex >= target_count) continue;
            healPlayer(&playerTargets[targetIndex], healAmount);
        }
    }

    if (ability->ability_type == ABILITY_TYPE_CLEANSE) {
        if (target_indices == NULL || selected_count <= 0) return;

        abilityWasUsed = 1;
        Player* playerTargets = (Player*)targets;
        for (int i = 0; i < selected_count; i++) {
            int targetIndex = target_indices[i];
            if (targetIndex < 0 || targetIndex >= target_count) continue;
            removeAllDebuffs(&playerTargets[targetIndex].statusList);
        }
    }

    if (ability->ability_type == ABILITY_TYPE_REVIVE) {
        if (target_indices == NULL || selected_count <= 0) return;

        abilityWasUsed = 1;
        Player* playerTargets = (Player*)targets;
        for (int i = 0; i < selected_count; i++) {
            int targetIndex = target_indices[i];
            if (targetIndex < 0 || targetIndex >= target_count) continue;
            revivePlayerCombat(&playerTargets[targetIndex], ability->data.revive.revive_hp_percent);
        }
    }

    if (ability->ability_type == ABILITY_TYPE_BUFF) {
        if (target_indices == NULL || selected_count <= 0) return;

        abilityWasUsed = 1;
        Player* playerTargets = (Player*)targets;
        StatusType blessingType = STATUS_NONE;

        if (caster->characterID == CHARACTER_3_HEALER && ability_index == 2) {
            /* Usa o modo armazenado na habilidade, não variável global */
            blessingType = (ability->current_buff_mode == BUFF_MODE_DAMAGE) ? STATUS_STRENGTH_UP : STATUS_DEFENSE_UP;
        }

        for (int i = 0; i < selected_count; i++) {
            int targetIndex = target_indices[i];
            if (targetIndex < 0 || targetIndex >= target_count) continue;

            Player* target = &playerTargets[targetIndex];

            if (caster->characterID == CHARACTER_1_TANK && ability_index == 1) {
                applyStatusToPlayerTarget(target, targetIndex, STATUS_PROTECT, ability->data.buff_action.turns_extra, 1.0f);
            } else if (caster->characterID == CHARACTER_1_TANK && ability_index == 3) {
                applyStatusToPlayerTarget(caster, targetIndex, STATUS_DEFENSE_UP, ability->data.reflect.reflect_duration, 1.0f);
                applyStatusToPlayerTarget(caster, targetIndex, STATUS_SPEED_UP, ability->data.reflect.reflect_duration, 1.0f);
                applyStatusToPlayerTarget(caster, targetIndex, STATUS_REFLECT, ability->data.reflect.reflect_duration, 1.0f);
            } else if (caster->characterID == CHARACTER_2_DPS && ability_index == 2) {
                applyStatusToPlayerTarget(caster, targetIndex, STATUS_COUNTER, 3, 1.0f);
            } else if (caster->characterID == CHARACTER_2_DPS && ability_index == 3) {
                applyStatusToPlayerTarget(caster, targetIndex, STATUS_SPEED_UP, 3, 1.0f);
                applyStatusToPlayerTarget(caster, targetIndex, STATUS_HASTE, 3, 1.0f);
                caster->extraTurnsPending = 1;
            } else if (caster->characterID == CHARACTER_3_HEALER && ability_index == 2) {
                applyStatusToPlayerTarget(target, targetIndex, blessingType, 3, 1.0f);
            } else if (caster->characterID == CHARACTER_4_MAGE && ability_index == 3) {
                applyStatusToPlayerTarget(caster, targetIndex, STATUS_ASCENSAO_MAGICA, -1, 0.10f);
            } else {
                applyStatusToPlayerTarget(target, targetIndex, STATUS_DEFENSE_UP, 2, 1.0f);
            }
        }
    }

    if (ability->ability_type == ABILITY_TYPE_DEBUFF) {
        if (target_indices == NULL || selected_count <= 0) return;

        abilityWasUsed = 1;
        Enemy* enemyTargets = (Enemy*)targets;
        for (int i = 0; i < selected_count; i++) {
            int targetIndex = target_indices[i];
            if (targetIndex < 0 || targetIndex >= target_count) continue;

            Enemy* target = &enemyTargets[targetIndex];

            if (caster->characterID == CHARACTER_4_MAGE && ability_index == 1) {
                applyStatusToEnemyTarget(target, STATUS_ARCANE_MARK, ability->data.status.duration, 1.0f);
            } else {
                applyStatusToEnemyTarget(target, ability->data.status.status_type, ability->data.status.duration, ability->data.status.intensity);
            }
        }
    }

    if (ability->ability_type == ABILITY_TYPE_SPECIAL) {
        if (target_indices == NULL || selected_count <= 0) return;

        abilityWasUsed = 1;
        Enemy* enemyTargets = (Enemy*)targets;
        for (int i = 0; i < selected_count; i++) {
            int targetIndex = target_indices[i];
            if (targetIndex < 0 || targetIndex >= target_count) continue;
            applyStatusToEnemyTarget(&enemyTargets[targetIndex], STATUS_CONFUSION, 1, 1.0f);
        }
    }

    if (abilityWasUsed && casterPartyIndex >= 0) {
        bossAiNotifyPlayerAbilityUsed(casterPartyIndex, ability_index);
    }

    caster->isAlive = (caster->stats.currentHP > 0);
}

/* ===== Sistema de Alternância de Modo/Elemento ===== */

const char* getElementName(ElementType element) {
    switch (element) {
        case ELEMENT_HEAT:  return "Calor";
        case ELEMENT_WIND:  return "Vento";
        case ELEMENT_TIDE:  return "Maré";
        case ELEMENT_EARTH: return "Terra";
        default:            return "Nenhum";
    }
}

const char* getBuffModeName(BuffMode mode) {
    switch (mode) {
        case BUFF_MODE_DAMAGE:      return "dano";
        case BUFF_MODE_RESISTANCE:  return "resistência";
        default:                    return "nenhum";
    }
}

void toggleAbilityElement(Ability* ability, int direction) {
    if (ability == NULL || !ability->is_alternatable) return;
    
    /* Elementos cíclicos: Calor -> Vento -> Maré -> Terra -> Calor */
    int current = (int)ability->current_element;
    int next = (current + direction) % 4;
    if (next < 0) next += 4;
    
    ability->current_element = (ElementType)next;
}

void toggleAbilityMode(Ability* ability, int direction) {
    if (ability == NULL || !ability->is_alternatable) return;
    
    /* Modos cíclicos: DANO <-> RESISTÊNCIA */
    if (ability->current_buff_mode == BUFF_MODE_DAMAGE) {
        ability->current_buff_mode = BUFF_MODE_RESISTANCE;
    } else {
        ability->current_buff_mode = BUFF_MODE_DAMAGE;
    }
}

void getAbilityDynamicDescription(Ability* ability, char* out_description, int max_length) {
    if (ability == NULL || out_description == NULL || max_length <= 0) return;
    
    /* Habilidades com elemento alternável (Character 4) */
    if (ability->characterID == CHARACTER_4_MAGE && (ability->ability_index == 0 || ability->ability_index == 2)) {
        const char* element_name = getElementName(ability->current_element);
        
        if (ability->ability_index == 0) {
            /* Magia Elemental (alvo único) */
            snprintf(out_description, max_length,
                "Ataque elemental em alvo unico. Elemento: %s\n\nPressione setas para trocar elemento",
                element_name);
        } else {
            /* Explosão Elemental (área) */
            snprintf(out_description, max_length,
                "Ataque elemental em todos inimigos. Elemento: %s\n\nPressione setas para trocar elemento",
                element_name);
        }
        return;
    }
    
    /* Habilidades com modo alternável (Character 3, Skill 3) */
    if (ability->characterID == CHARACTER_3_HEALER && ability->ability_index == 2) {
        const char* mode_name = getBuffModeName(ability->current_buff_mode);
        snprintf(out_description, max_length,
            "Buff alternável: +25%% %s por 3 turnos.\n\nPressione setas para trocar o efeito.",
            mode_name);
        return;
    }
    
    /* Padrão: usa descrição estática */
    strncpy(out_description, ability->description, max_length - 1);
    out_description[max_length - 1] = '\0';
}

int getElementalDefense(Enemy* enemy, ElementType element) {
    if (enemy == NULL) return 0;
    
    switch (element) {
        case ELEMENT_HEAT:  return enemy->stats.defCalor;
        case ELEMENT_WIND:  return enemy->stats.defVento;
        case ELEMENT_TIDE:  return enemy->stats.defMare;
        case ELEMENT_EARTH: return enemy->stats.defTerra;
        default:            return 0;
    }
}

void applyElementalDamage(Enemy* target, Player* caster, Ability* ability) {
    if (target == NULL || caster == NULL || ability == NULL) return;

    int damage = (int)getAbilityDamage(ability, caster);
    
    /* Aplica modificadores de buff de mente (STATUS_STRENGTH_UP para elemental) */
    if (ability->scaling_type == SCALING_MENTE) {
        float strengthMod = getStrengthModifier(&caster->statusList);
        damage = (int)(damage * strengthMod);
    }
    
    /* Aplica modificador de Ascensão Mágica se ativo */
    StatusCondition* ascensao = getStatusCondition(&caster->statusList, STATUS_ASCENSAO_MAGICA);
    if (ascensao != NULL) {
        damage = (int)(damage * (1.0f + ascensao->intensity));
    }
    
    /* Aplica defesa elemental se a habilidade tem elemento */
    if (ability->current_element != ELEMENT_NONE) {
        int elementalDefense = getElementalDefense(target, ability->current_element);
        /* Resistência é interpretada como percentual (0-100) */
        /* Fórmula: damage_final = damage_base * (1 - resistance_percent) */
        float resistance_percent = elementalDefense / 100.0f;
        if (resistance_percent > 1.0f) resistance_percent = 1.0f;  /* Clamp a 100% */
        int reduced_damage = (int)(damage * (1.0f - resistance_percent));
        if (reduced_damage < 1) reduced_damage = 1;  /* Mínimo 1 de dano */
        damage = reduced_damage;
    }
    
    /* Marca Arcana doubles the damage and is consumed */
    if (hasStatusCondition(&target->statusList, STATUS_ARCANE_MARK)) {
        damage *= 2;
        removeStatusCondition(&target->statusList, STATUS_ARCANE_MARK);
        bossAiQueueMessage("Marca Arcana consumida! Dano elemental dobrado.");
    }
    
    damageEnemy(target, damage);
}

