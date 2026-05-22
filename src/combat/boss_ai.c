#include "boss_ai.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

#include "ability.h"
#include "../core/game.h"
#include "../entities/status_condition.h"

typedef enum {
    BOSS_KIND_NONE = 0,
    BOSS_KIND_1,
    BOSS_KIND_2
} BossKind;

typedef enum {
    BOSS2_TRAP_HIGH_DAMAGE = 0,
    BOSS2_TRAP_LOW_BLEED,
    BOSS2_TRAP_LOW_DEFENSE_DOWN,
    BOSS2_TRAP_FALSE
} Boss2TrapEffect;

typedef struct {
    int active;
    Boss2TrapEffect effect;
} Boss2TrapMark;

typedef enum {
    BOSS_ELEMENT_HEAT = 0,
    BOSS_ELEMENT_WIND,
    BOSS_ELEMENT_TIDE,
    BOSS_ELEMENT_EARTH,
    BOSS_ELEMENT_COUNT
} BossElement;

typedef struct {
    int active;
    BossKind kind;
    int worldEnemyIndex;
    int phase;
    int currentElementIndex;
    int openingTurnPending;
    float cumulativeDamageBonus;
    int originalResistances[PARTY_SIZE][4];
    int originalSaved;
    int displayedWeakness[PARTY_SIZE];
    Boss2TrapMark trapMarks[PARTY_SIZE];
    int boss2SpecialUsedThisRound;
} BossEncounterState;

static BossEncounterState bossState = {0};
static char bossMessageQueue[BOSS_AI_MAX_MESSAGES][BOSS_AI_MESSAGE_LEN];
static int bossMessageCount = 0;
static int bossMessageIndex = 0;
static float bossMessageTimer = 0.0f;

#define BOSS_STATUS_DURATION 3

static void clearBoss2TrapMarks(void) {
    for (int i = 0; i < PARTY_SIZE; i++) {
        bossState.trapMarks[i].active = 0;
        bossState.trapMarks[i].effect = BOSS2_TRAP_HIGH_DAMAGE;
    }
}

static int hasAnyStatusPlayer(void) {
    for (int i = 0; i < PARTY_SIZE; i++) {
        if (!party[i].isAlive) continue;
        if (countActiveStatus(&party[i].statusList) > 0) {
            return 1;
        }
    }
    return 0;
}

static int findStatusTargetIndex(void) {
    for (int i = 0; i < PARTY_SIZE; i++) {
        if (!party[i].isAlive) continue;
        if (countActiveStatus(&party[i].statusList) > 0) {
            return i;
        }
    }
    return -1;
}

static int chooseRandomAliveUnmarkedPlayer(void) {
    int candidates[PARTY_SIZE];
    int candidateCount = 0;

    for (int i = 0; i < PARTY_SIZE; i++) {
        if (!party[i].isAlive) continue;
        if (bossState.trapMarks[i].active) continue;
        candidates[candidateCount++] = i;
    }

    if (candidateCount <= 0) {
        return -1;
    }

    return candidates[rand() % candidateCount];
}

static Boss2TrapEffect rollTrapEffect(void) {
    return (Boss2TrapEffect)(rand() % 3);
}

static void boss2ApplyTrapEffect(int playerIndex, Boss2TrapEffect effect);
static void boss2UseSpecialAttack(void);

static int boss2HasPendingTraps(void) {
    for (int i = 0; i < PARTY_SIZE; i++) {
        if (bossState.trapMarks[i].active) {
            return 1;
        }
    }

    return 0;
}

static void boss2ActivateAllPendingTraps(void) {
    for (int i = 0; i < PARTY_SIZE; i++) {
        if (!bossState.trapMarks[i].active) continue;

        boss2ApplyTrapEffect(i, bossState.trapMarks[i].effect);
    }
}

static void boss2LaunchNewTraps(void) {
    int trapCount = (bossState.phase >= 2) ? 3 : 2;
    int falseTrapIndex = (bossState.phase >= 2) ? (rand() % trapCount) : -1;
    int marksAdded = 0;

    while (marksAdded < trapCount) {
        int playerIndex = chooseRandomAliveUnmarkedPlayer();
        if (playerIndex < 0) break;

        Boss2TrapEffect effect = (marksAdded == falseTrapIndex) ? BOSS2_TRAP_FALSE : rollTrapEffect();
        bossState.trapMarks[playerIndex].active = 1;
        bossState.trapMarks[playerIndex].effect = effect;
        bossAiQueueMessage("Boss 2 armou uma armadilha em Personagem %d!", playerIndex + 1);
        marksAdded++;
    }
}

static void boss2ResolveTurnAction(void) {
    if (boss2HasPendingTraps()) {
        boss2ActivateAllPendingTraps();
        return;
    }

    if (!bossState.boss2SpecialUsedThisRound && hasAnyStatusPlayer()) {
        boss2UseSpecialAttack();
        return;
    }

    boss2LaunchNewTraps();
}

static void boss2ApplyTrapEffect(int playerIndex, Boss2TrapEffect effect) {
    if (playerIndex < 0 || playerIndex >= PARTY_SIZE) return;

    Player* target = &party[playerIndex];
    if (!target->isAlive) return;

    float damageMultiplier = 1.0f + bossState.cumulativeDamageBonus;
    int trapDamageHigh = (bossState.phase >= 2) ? 150 : 120;
    int trapDamageLow = (bossState.phase >= 2) ? 60 : 45;

    switch (effect) {
        case BOSS2_TRAP_HIGH_DAMAGE:
            bossAiQueueMessage("A armadilha explodiu em Personagem %d!", playerIndex + 1);
            applyCombatDamageToPlayer(target, (int)(trapDamageHigh * damageMultiplier));
            break;

        case BOSS2_TRAP_LOW_BLEED:
            bossAiQueueMessage("A armadilha feriu Personagem %d!", playerIndex + 1);
            applyCombatDamageToPlayer(target, (int)(trapDamageLow * damageMultiplier));
            if (!target->defenseGuardActive) {
                addStatusCondition(&target->statusList, STATUS_BLEED, -1, 5.0f);
                bossAiQueuePlayerAfflictedMessage(playerIndex, STATUS_BLEED);
            }
            break;

        case BOSS2_TRAP_LOW_DEFENSE_DOWN:
            applyCombatDamageToPlayer(target, (int)(trapDamageLow * damageMultiplier));
            if (!target->defenseGuardActive) {
                bossAiQueueMessage("A armadilha abalou a defesa de Personagem %d!", playerIndex + 1);
                addStatusCondition(&target->statusList, STATUS_DEFENSE_DOWN, 3, 1.0f);
                bossAiQueuePlayerAfflictedMessage(playerIndex, STATUS_DEFENSE_DOWN);
            }
            break;

        case BOSS2_TRAP_FALSE:
            bossAiQueueMessage("A armadilha era falsa em Personagem %d!", playerIndex + 1);
            break;
    }

    bossState.cumulativeDamageBonus += 0.10f;
    bossState.trapMarks[playerIndex].active = 0;
    bossState.trapMarks[playerIndex].effect = BOSS2_TRAP_HIGH_DAMAGE;
}

static void boss2UseSpecialAttack(void) {
    int targetIndex = findStatusTargetIndex();
    if (targetIndex < 0) return;

    Player* target = &party[targetIndex];
    bossAiQueueMessage("Boss 2 esmagou %s com um ataque especial!", target->name);
    applyCombatDamageToPlayer(target, (int)(150 * (1.0f + bossState.cumulativeDamageBonus)));
    bossState.boss2SpecialUsedThisRound = 1;
}

static void boss2UseRoundWrapAction(void) {
    boss2ResolveTurnAction();
}

static void clearMessageQueue(void) {
    bossMessageCount = 0;
    bossMessageIndex = 0;
    bossMessageTimer = 0.0f;
}

void bossAiQueueMessage(const char* fmt, ...) {
    if (fmt == NULL) return;
    if (bossMessageCount >= BOSS_AI_MAX_MESSAGES) return;

    va_list args;
    va_start(args, fmt);
    vsnprintf(bossMessageQueue[bossMessageCount], BOSS_AI_MESSAGE_LEN, fmt, args);
    va_end(args);

    bossMessageCount++;

    if (bossMessageCount == 1) {
        bossMessageIndex = 0;
        bossMessageTimer = 1.4f;
    }
}

void bossAiQueuePlayerAfflictedMessage(int playerIndex, StatusType statusType) {
    if (playerIndex < 0 || statusType == STATUS_NONE || !isDebuff(statusType)) return;
    bossAiQueueMessage("Personagem %d foi aflito com %s!", playerIndex + 1, getStatusName(statusType));
}

int bossAiHasActiveMessage(void) {
    return (bossMessageCount > 0 && bossMessageIndex < bossMessageCount);
}

const char* bossAiGetCurrentMessage(void) {
    if (!bossAiHasActiveMessage()) return NULL;
    return bossMessageQueue[bossMessageIndex];
}

void bossAiUpdateMessages(float deltaTime) {
    if (!bossAiHasActiveMessage()) return;

    bossMessageTimer -= deltaTime;
    if (bossMessageTimer > 0.0f) return;

    bossMessageIndex++;
    if (bossMessageIndex >= bossMessageCount) {
        clearMessageQueue();
        return;
    }

    bossMessageTimer = 1.4f;
}

static const char* getBossElementName(int elementIndex) {
    switch (elementIndex) {
        case BOSS_ELEMENT_HEAT:  return "Calor";
        case BOSS_ELEMENT_WIND:  return "Vento";
        case BOSS_ELEMENT_TIDE:  return "Mare";
        case BOSS_ELEMENT_EARTH: return "Terra";
        default:                 return "Desconhecido";
    }
}

static int getPlayerElementDefense(const Player* player, int elementIndex) {
    if (player == NULL) return 0;

    switch (elementIndex) {
        case BOSS_ELEMENT_HEAT:  return player->stats.defCalor;
        case BOSS_ELEMENT_WIND:  return player->stats.defVento;
        case BOSS_ELEMENT_TIDE:  return player->stats.defMare;
        case BOSS_ELEMENT_EARTH: return player->stats.defTerra;
        default:                 return 0;
    }
}

static int getPlayerWeakestElement(const Player* player) {
    int minValue = getPlayerElementDefense(player, 0);
    int weakest = 0;

    for (int i = 1; i < BOSS_ELEMENT_COUNT; i++) {
        int value = getPlayerElementDefense(player, i);
        if (value < minValue) {
            minValue = value;
            weakest = i;
        }
    }

    return weakest;
}

static StatusType getElementStatusType(int elementIndex) {
    switch (elementIndex) {
        case BOSS_ELEMENT_HEAT:  return STATUS_ENSOLACAO;
        case BOSS_ELEMENT_WIND:  return STATUS_CONFUSION;
        case BOSS_ELEMENT_TIDE:  return STATUS_ENCHARCADO;
        case BOSS_ELEMENT_EARTH: return STATUS_SLOW;
        default:                 return STATUS_NONE;
    }
}

static void saveOriginalPartyResistances(void) {
    if (bossState.originalSaved) return;

    for (int i = 0; i < PARTY_SIZE; i++) {
        bossState.originalResistances[i][0] = party[i].stats.defCalor;
        bossState.originalResistances[i][1] = party[i].stats.defVento;
        bossState.originalResistances[i][2] = party[i].stats.defMare;
        bossState.originalResistances[i][3] = party[i].stats.defTerra;
    }

    bossState.originalSaved = 1;
}

static void restoreOriginalPartyResistances(void) {
    if (!bossState.originalSaved) return;

    for (int i = 0; i < PARTY_SIZE; i++) {
        party[i].stats.defCalor = bossState.originalResistances[i][0];
        party[i].stats.defVento = bossState.originalResistances[i][1];
        party[i].stats.defMare = bossState.originalResistances[i][2];
        party[i].stats.defTerra = bossState.originalResistances[i][3];
        bossState.displayedWeakness[i] = -1;
    }

    bossState.originalSaved = 0;
}

static void shufflePartyWeaknessesForPhase2(void) {
    for (int i = 0; i < PARTY_SIZE; i++) {
        int values[4] = {
            party[i].stats.defCalor,
            party[i].stats.defVento,
            party[i].stats.defMare,
            party[i].stats.defTerra
        };

        for (int j = 3; j > 0; j--) {
            int k = rand() % (j + 1);
            int tmp = values[j];
            values[j] = values[k];
            values[k] = tmp;
        }

        party[i].stats.defCalor = values[0];
        party[i].stats.defVento = values[1];
        party[i].stats.defMare = values[2];
        party[i].stats.defTerra = values[3];
        bossState.displayedWeakness[i] = getPlayerWeakestElement(&party[i]);
    }

    bossAiQueueMessage("As fraquezas elementais foram embaralhadas!");
}

void bossAiOnCombatStart(void) {
    clearMessageQueue();
    memset(&bossState, 0, sizeof(bossState));
    bossState.kind = BOSS_KIND_NONE;
    for (int i = 0; i < PARTY_SIZE; i++) {
        bossState.displayedWeakness[i] = -1;
    }

    clearBoss2TrapMarks();

    for (int i = 0; i < combat.enemyCount; i++) {
        int worldEnemyIndex = combat.enemyIndices[i];
        if (worldEnemyIndex < 0 || worldEnemyIndex >= enemyManager.count) continue;

        if (strcmp(enemyManager.enemies[worldEnemyIndex].name, "Boss 2") == 0) {
            bossState.active = 1;
            bossState.kind = BOSS_KIND_2;
            bossState.worldEnemyIndex = worldEnemyIndex;
            bossState.phase = 1;
            bossState.currentElementIndex = 0;
            bossState.openingTurnPending = 0;
            bossState.cumulativeDamageBonus = 0.0f;
            bossState.boss2SpecialUsedThisRound = 0;
            return;
        }
    }

    for (int i = 0; i < combat.enemyCount; i++) {
        int worldEnemyIndex = combat.enemyIndices[i];
        if (worldEnemyIndex < 0 || worldEnemyIndex >= enemyManager.count) continue;

        if (strcmp(enemyManager.enemies[worldEnemyIndex].name, "Boss 1") == 0) {
            bossState.active = 1;
            bossState.kind = BOSS_KIND_1;
            bossState.worldEnemyIndex = worldEnemyIndex;
            bossState.phase = 1;
            bossState.currentElementIndex = 0;
            bossState.openingTurnPending = 1;
            bossState.cumulativeDamageBonus = 0.0f;
            saveOriginalPartyResistances();

            /* Aviso antecipado do próximo elemento do boss */
            bossAiQueueMessage("Boss 1 prepara magia de %s!", getBossElementName(bossState.currentElementIndex));
            break;
        }
    }
}

void bossAiOnCombatEnd(void) {
    clearMessageQueue();
    restoreOriginalPartyResistances();
    clearBoss2TrapMarks();
    memset(&bossState, 0, sizeof(bossState));
}

void bossAiOnRoundWrap(void) {
    if (!bossState.active || bossState.kind != BOSS_KIND_2) return;

    boss2UseRoundWrapAction();
    bossState.boss2SpecialUsedThisRound = 0;
}

int bossAiHandleEnemyTurn(int worldEnemyIndex, Enemy* enemy) {
    if (!bossState.active || worldEnemyIndex != bossState.worldEnemyIndex || enemy == NULL || !enemy->isAlive) {
        return 0;
    }

    if (bossState.kind == BOSS_KIND_2) {
        if (bossState.phase == 1 && enemy->stats.currentHP <= (enemy->stats.maxHP / 2)) {
            bossState.phase = 2;
            bossAiQueueMessage("Boss 2 entrou na fase 2!");
        }

        boss2ResolveTurnAction();
        return 1;
    }

    if (bossAiHasActiveMessage()) {
        return 1;
    }

    if (bossState.openingTurnPending) {
        bossState.openingTurnPending = 0;
        bossAiQueueMessage("Boss 1 prepara magia de %s!", getBossElementName(bossState.currentElementIndex));
        return 1;
    }

    if (!canEnemyAct(enemy)) {
        bossAiQueueMessage("%s falhou em agir.", enemy->name);
        return 1;
    }

    if (bossState.phase == 1 && enemy->stats.currentHP <= (enemy->stats.maxHP / 2)) {
        bossState.phase = 2;
        bossState.cumulativeDamageBonus = 0.0f;
        bossAiQueueMessage("Boss 1 entrou na fase 2!");
        shufflePartyWeaknessesForPhase2();
    }

    if (bossState.phase == 2) {
        shufflePartyWeaknessesForPhase2();
    }

    int element = bossState.currentElementIndex;

    float totalMultiplier = 1.0f;
    if (bossState.phase >= 2) {
        totalMultiplier += bossState.cumulativeDamageBonus;
    }
    int baseDamage = (bossState.phase == 1) ? 70 : 90;
    int weaknessHits = 0;
    char weakNames[96] = "";

    for (int i = 0; i < PARTY_SIZE; i++) {
        Player* target = &party[i];
        if (!target->isAlive) continue;

        int weakestElement = getPlayerWeakestElement(target);
        int damage = (int)(baseDamage * totalMultiplier);

        if (weakestElement == element) {
            damage = (int)(damage * 1.35f);

            StatusType status = getElementStatusType(element);
            if (!target->defenseGuardActive && status != STATUS_NONE) {
                addStatusCondition(&target->statusList, status, BOSS_STATUS_DURATION, 1.0f);
                bossAiQueuePlayerAfflictedMessage(i, status);
            }

            if (weaknessHits > 0) {
                strncat(weakNames, ", ", sizeof(weakNames) - strlen(weakNames) - 1);
            }
            strncat(weakNames, target->name, sizeof(weakNames) - strlen(weakNames) - 1);

            if (bossState.phase >= 2) {
                bossState.cumulativeDamageBonus += 0.10f;
            }
            weaknessHits++;
        }

        applyCombatDamageToPlayer(target, damage);
    }

    bossAiQueueMessage("Boss 1 atingiu toda a party com %s.", getBossElementName(element));
    if (weaknessHits > 0 && bossState.phase >= 2) {
        bossAiQueueMessage("Fraquezas atingidas: %s. Bonus cumulativo +10%% por acerto.", weakNames);
    }

    bossState.currentElementIndex = (bossState.currentElementIndex + 1) % BOSS_ELEMENT_COUNT;

    /* Já anuncia o próximo elemento com antecedência para o jogador se preparar */
    bossAiQueueMessage("Boss 1 prepara magia de %s!", getBossElementName(bossState.currentElementIndex));

    return 1;
}

int bossAiGetPlayerWeaknessElement(int playerIndex) {
    if (!bossState.active || bossState.phase < 2) return -1;
    if (playerIndex < 0 || playerIndex >= PARTY_SIZE) return -1;
    return bossState.displayedWeakness[playerIndex];
}

int bossAiHasTrapMark(int playerIndex) {
    if (!bossState.active || bossState.kind != BOSS_KIND_2) return 0;
    if (playerIndex < 0 || playerIndex >= PARTY_SIZE) return 0;
    return bossState.trapMarks[playerIndex].active;
}

void bossAiNotifyPlayerPhysicalAction(int playerIndex, int isPhysicalAction) {
    if (!bossState.active || bossState.kind != BOSS_KIND_2) return;
    if (!isPhysicalAction) return;
    if (playerIndex < 0 || playerIndex >= PARTY_SIZE) return;

    if (!bossState.trapMarks[playerIndex].active) return;

    boss2ApplyTrapEffect(playerIndex, bossState.trapMarks[playerIndex].effect);
}
