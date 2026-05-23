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
    BOSS_KIND_2,
    BOSS_KIND_3
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
    int boss3OfferTurnUsedThisRound;
    int boss3SpecialUsedThisRound;
    int boss3Phase2SkipPending;
    int boss3Debt;
    int boss3OfferTargetIndex[2];
    int boss3OfferType[2];
    Boss2TrapMark boss3TemptationMarks[PARTY_SIZE];
} BossEncounterState;

static BossEncounterState bossState = {0};
static char bossMessageQueue[BOSS_AI_MAX_MESSAGES][BOSS_AI_MESSAGE_LEN];
static int bossMessageCount = 0;
static int bossMessageIndex = 0;
static float bossMessageTimer = 0.0f;

typedef struct {
    int active;
    int playerIndex;
    int offerType;
    char text[BOSS_AI_MESSAGE_LEN];
} Boss3PromptEntry;

static BossAiPrompt currentPromptView = {0};
static Boss3PromptEntry boss3PromptQueue[2];
static int boss3PromptCount = 0;
static int boss3PromptIndex = 0;
static int boss3PromptActive = 0;

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

static int boss3HasSpecialTarget(void) {
    for (int i = 0; i < PARTY_SIZE; i++) {
        if (!party[i].isAlive) continue;

        if (hasStatusCondition(&party[i].statusList, STATUS_ENSOLACAO) ||
            hasStatusCondition(&party[i].statusList, STATUS_DEFENSE_DOWN)) {
            return 1;
        }
    }

    return 0;
}

static void clearBoss3TemptationMarks(void) {
    for (int i = 0; i < PARTY_SIZE; i++) {
        bossState.boss3TemptationMarks[i].active = 0;
        bossState.boss3TemptationMarks[i].effect = BOSS2_TRAP_HIGH_DAMAGE;
    }
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
static void boss3PrepareOfferQueue(void);
static int boss3StartOfferTurn(void);
static void boss3ApplyCurrentOffer(int accepted);
static void boss3UseSpecialAttack(void);
static void boss3UsePhase1AreaAttack(void);
static void boss3EnterPhase2(void);
static int boss3GetAbilityDebtValue(int abilityIndex);
static void boss3UsePhase2ZeroDebtAttack(void);
static void boss3UsePhase2DebtAttack(void);
static int boss3ChooseRandomAliveTarget(void);
static void boss3MarkTemptationTargets(int firstTargetIndex, int secondTargetIndex);

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
    bossAiClearPromptQueue();
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

        if (strcmp(enemyManager.enemies[worldEnemyIndex].name, "Boss 3") == 0) {
            bossState.active = 1;
            bossState.kind = BOSS_KIND_3;
            bossState.worldEnemyIndex = worldEnemyIndex;
            bossState.phase = 1;
            bossState.currentElementIndex = 0;
            bossState.openingTurnPending = 0;
            bossState.cumulativeDamageBonus = 0.0f;
            bossState.boss3OfferTurnUsedThisRound = 0;
            bossState.boss3SpecialUsedThisRound = 0;
            bossState.boss3Phase2SkipPending = 0;
            bossState.boss3Debt = 0;
            bossState.boss3OfferTargetIndex[0] = -1;
            bossState.boss3OfferTargetIndex[1] = -1;
            bossState.boss3OfferType[0] = 0;
            bossState.boss3OfferType[1] = 0;
            clearBoss3TemptationMarks();
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
    bossAiClearPromptQueue();
    restoreOriginalPartyResistances();
    clearBoss2TrapMarks();
    clearBoss3TemptationMarks();
    memset(&bossState, 0, sizeof(bossState));
}

void bossAiOnRoundWrap(void) {
    if (!bossState.active) return;

    if (bossState.kind == BOSS_KIND_2) {
        boss2UseRoundWrapAction();
        bossState.boss2SpecialUsedThisRound = 0;
        return;
    }

    if (bossState.kind == BOSS_KIND_3) {
        if (bossState.phase >= 2 && bossState.boss3Phase2SkipPending) {
            bossState.boss3Phase2SkipPending = 0;
            bossAiQueueMessage("Boss 3 perdeu um turno para reorganizar a divida.");
            return;
        }

        if (bossState.phase < 2) {
            if (!bossState.boss3SpecialUsedThisRound && boss3HasSpecialTarget()) {
                boss3UseSpecialAttack();
            } else {
                boss3UsePhase1AreaAttack();
            }
        } else {
            if (bossState.boss3Debt <= 0) {
                boss3UsePhase2ZeroDebtAttack();
            } else {
                boss3UsePhase2DebtAttack();
            }
        }

        bossState.boss3OfferTurnUsedThisRound = 0;
        bossState.boss3SpecialUsedThisRound = 0;
    }
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

    if (bossState.kind == BOSS_KIND_3) {
        if (bossState.phase == 1 && enemy->stats.currentHP <= (enemy->stats.maxHP / 2)) {
            boss3EnterPhase2();
        }

        if (bossState.phase >= 2) {
            if (!canEnemyAct(enemy)) {
                bossAiQueueMessage("%s falhou em agir.", enemy->name);
            }

            return 1;
        }

        if (boss3PromptActive) {
            return 2;
        }

        if (!canEnemyAct(enemy)) {
            bossAiQueueMessage("%s falhou em agir.", enemy->name);
            return 1;
        }

        return boss3StartOfferTurn();
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

static int boss3CountAlivePlayers(void) {
    int count = 0;
    for (int i = 0; i < PARTY_SIZE; i++) {
        if (party[i].isAlive) {
            count++;
        }
    }

    return count;
}

static int boss3ChooseRandomAlivePlayer(int excludedIndex) {
    int candidates[PARTY_SIZE];
    int candidateCount = 0;

    for (int i = 0; i < PARTY_SIZE; i++) {
        if (!party[i].isAlive) continue;
        if (i == excludedIndex) continue;
        candidates[candidateCount++] = i;
    }

    if (candidateCount <= 0) return -1;
    return candidates[rand() % candidateCount];
}

static int boss3ChooseRandomAliveTarget(void) {
    return boss3ChooseRandomAlivePlayer(-1);
}

static const char* boss3GetOfferName(int offerType) {
    switch (offerType) {
        case 0: return "Poder";
        case 1: return "Folego";
        case 2: return "Protecao";
        default: return "Desconhecido";
    }
}

static StatusType boss3GetPositiveStatus(int offerType) {
    switch (offerType) {
        case 0: return STATUS_STRENGTH_UP;
        case 1: return STATUS_SPEED_UP;
        case 2: return STATUS_DEFENSE_UP;
        default: return STATUS_NONE;
    }
}

static StatusType boss3GetNegativeStatus(int offerType) {
    switch (offerType) {
        case 0: return STATUS_ENSOLACAO;
        case 1: return STATUS_DEFENSE_DOWN;
        case 2: return STATUS_DEFENSE_DOWN;
        default: return STATUS_NONE;
    }
}

void bossAiClearPromptQueue(void) {
    memset(boss3PromptQueue, 0, sizeof(boss3PromptQueue));
    boss3PromptCount = 0;
    boss3PromptIndex = 0;
    boss3PromptActive = 0;
    memset(&currentPromptView, 0, sizeof(currentPromptView));
}

int bossAiHasPendingPrompt(void) {
    return boss3PromptActive && boss3PromptIndex < boss3PromptCount;
}

const BossAiPrompt* bossAiGetCurrentPrompt(void) {
    if (!bossAiHasPendingPrompt()) return NULL;

    currentPromptView.text = boss3PromptQueue[boss3PromptIndex].text;
    currentPromptView.optionLeft = "SIM";
    currentPromptView.optionRight = "NAO";
    return &currentPromptView;
}

int bossAiResolveCurrentPrompt(int accepted) {
    if (!bossAiHasPendingPrompt()) return 1;

    if (accepted) {
        Boss3PromptEntry* prompt = &boss3PromptQueue[boss3PromptIndex];
        int playerIndex = prompt->playerIndex;

        if (playerIndex >= 0 && playerIndex < PARTY_SIZE) {
            Player* target = &party[playerIndex];
            if (target->isAlive && !target->defenseGuardActive) {
                StatusType positiveStatus = boss3GetPositiveStatus(prompt->offerType);
                StatusType negativeStatus = boss3GetNegativeStatus(prompt->offerType);

                if (positiveStatus != STATUS_NONE) {
                    addStatusCondition(&target->statusList, positiveStatus, BOSS_STATUS_DURATION, 1.0f);
                    bossAiQueuePlayerAfflictedMessage(playerIndex, positiveStatus);
                }

                if (negativeStatus != STATUS_NONE) {
                    addStatusCondition(&target->statusList, negativeStatus, BOSS_STATUS_DURATION, 1.0f);
                    bossAiQueuePlayerAfflictedMessage(playerIndex, negativeStatus);
                }
            }
        }
    }

    boss3PromptIndex++;
    if (boss3PromptIndex >= boss3PromptCount) {
        bossAiClearPromptQueue();
        return 1;
    }

    return 0;
}

void bossAiAdjustBoss3Debt(int delta) {
    if (!bossState.active || bossState.kind != BOSS_KIND_3 || bossState.phase < 2) return;

    bossState.boss3Debt += delta;
    if (bossState.boss3Debt < 0) {
        bossState.boss3Debt = 0;
    }
}

int bossAiGetBoss3Debt(void) {
    if (!bossState.active || bossState.kind != BOSS_KIND_3 || bossState.phase < 2) return 0;
    return bossState.boss3Debt;
}

int bossAiIsBoss3Phase2(void) {
    return bossState.active && bossState.kind == BOSS_KIND_3 && bossState.phase >= 2;
}

int bossAiHasTemptationMark(int playerIndex) {
    if (!bossState.active || bossState.kind != BOSS_KIND_3 || bossState.phase < 2) return 0;
    if (playerIndex < 0 || playerIndex >= PARTY_SIZE) return 0;
    return bossState.boss3TemptationMarks[playerIndex].active;
}

void bossAiNotifyPlayerAbilityUsed(int playerIndex, int abilityIndex) {
    if (!bossState.active || bossState.kind != BOSS_KIND_3 || bossState.phase < 2) return;
    if (playerIndex < 0 || playerIndex >= PARTY_SIZE) return;
    if (abilityIndex < 0 || abilityIndex >= 4) return;

    int debtValue = boss3GetAbilityDebtValue(abilityIndex);
    if (debtValue <= 0) return;

    if (bossState.boss3TemptationMarks[playerIndex].active) {
        debtValue += debtValue / 2;
        bossState.boss3TemptationMarks[playerIndex].active = 0;
    }

    bossAiAdjustBoss3Debt(debtValue);
}

static int boss3GetAbilityDebtValue(int abilityIndex) {
    switch (abilityIndex) {
        case 0: return 40;
        case 1: return 60;
        case 2: return 80;
        case 3: return 100;
        default: return 0;
    }
}

static void boss3MarkTemptationTargets(int firstTargetIndex, int secondTargetIndex) {
    if (firstTargetIndex >= 0 && firstTargetIndex < PARTY_SIZE) {
        bossState.boss3TemptationMarks[firstTargetIndex].active = 1;
    }

    if (secondTargetIndex >= 0 && secondTargetIndex < PARTY_SIZE) {
        bossState.boss3TemptationMarks[secondTargetIndex].active = 1;
    }
}

static void boss3PrepareOfferQueue(void) {
    int aliveCount = boss3CountAlivePlayers();
    int offerCount = (aliveCount >= 2) ? 2 : aliveCount;

    bossAiClearPromptQueue();

    if (offerCount <= 0) {
        return;
    }

    boss3PromptQueue[0].active = 1;
    boss3PromptQueue[0].playerIndex = boss3ChooseRandomAlivePlayer(-1);
    boss3PromptQueue[0].offerType = rand() % 3;
    snprintf(
        boss3PromptQueue[0].text,
        sizeof(boss3PromptQueue[0].text),
        "Boss 3 oferece %s para Personagem %d. Aceita?",
        boss3GetOfferName(boss3PromptQueue[0].offerType),
        boss3PromptQueue[0].playerIndex + 1
    );

    if (offerCount >= 2) {
        boss3PromptQueue[1].active = 1;
        boss3PromptQueue[1].playerIndex = boss3ChooseRandomAlivePlayer(boss3PromptQueue[0].playerIndex);
        boss3PromptQueue[1].offerType = rand() % 3;
        snprintf(
            boss3PromptQueue[1].text,
            sizeof(boss3PromptQueue[1].text),
            "Boss 3 oferece %s para Personagem %d. Aceita?",
            boss3GetOfferName(boss3PromptQueue[1].offerType),
            boss3PromptQueue[1].playerIndex + 1
        );
        boss3PromptCount = 2;
    } else {
        boss3PromptCount = 1;
    }

    boss3PromptIndex = 0;
    boss3PromptActive = 1;
}

static int boss3StartOfferTurn(void) {
    if (bossState.phase >= 2) {
        bossAiClearPromptQueue();
        bossState.boss3OfferTurnUsedThisRound = 1;
        return 1;
    }

    boss3PrepareOfferQueue();
    bossState.boss3OfferTurnUsedThisRound = 1;
    return bossAiHasPendingPrompt() ? 2 : 1;
}

static void boss3EnterPhase2(void) {
    if (bossState.phase >= 2) return;

    bossState.phase = 2;
    bossState.boss3Phase2SkipPending = 1;
    bossState.boss3Debt = 0;
    bossState.boss3OfferTurnUsedThisRound = 0;
    bossAiClearPromptQueue();
    bossAiQueueMessage("Boss 3 entrou na fase 2!");
}

static void boss3UseSpecialAttack(void) {
    int targetIndex = -1;

    for (int i = 0; i < PARTY_SIZE; i++) {
        if (!party[i].isAlive) continue;

        if (hasStatusCondition(&party[i].statusList, STATUS_ENSOLACAO) ||
            hasStatusCondition(&party[i].statusList, STATUS_DEFENSE_DOWN)) {
            targetIndex = i;
            break;
        }
    }

    if (targetIndex < 0) return;

    Player* target = &party[targetIndex];
    bossAiQueueMessage("Boss 3 castigou %s com um ataque especial!", target->name);
    applyCombatDamageToPlayer(target, 150);
    healEnemy(&enemyManager.enemies[bossState.worldEnemyIndex], 50);
    bossState.boss3SpecialUsedThisRound = 1;
}

static void boss3UsePhase1AreaAttack(void) {
    int afflictedCount = 0;

    for (int i = 0; i < PARTY_SIZE; i++) {
        Player* target = &party[i];
        if (!target->isAlive) continue;

        applyCombatDamageToPlayer(target, 90);

        if (!target->defenseGuardActive && (rand() % 100) < 50) {
            addStatusCondition(&target->statusList, STATUS_DEFENSE_DOWN, BOSS_STATUS_DURATION, 1.0f);
            bossAiQueuePlayerAfflictedMessage(i, STATUS_DEFENSE_DOWN);
            afflictedCount++;
        }
    }

    bossAiQueueMessage("Boss 3 atingiu a party com um golpe de area.");
    if (afflictedCount > 0) {
        bossAiQueueMessage("A pressao abalou a defesa de alguns alvos.");
    }
}

static void boss3UsePhase2ZeroDebtAttack(void) {
    int firstTargetIndex = boss3ChooseRandomAliveTarget();
    int secondTargetIndex = boss3ChooseRandomAliveTarget();

    if (firstTargetIndex < 0) return;
    if (secondTargetIndex == firstTargetIndex) {
        secondTargetIndex = -1;
    }

    if (firstTargetIndex >= 0) {
        Player* firstTarget = &party[firstTargetIndex];
        bossAiQueueMessage("Boss 3 amarra %s em tentacao!", firstTarget->name);
        applyCombatDamageToPlayer(firstTarget, 110);
    }

    if (secondTargetIndex < 0) {
        secondTargetIndex = boss3ChooseRandomAlivePlayer(firstTargetIndex);
    }

    if (secondTargetIndex >= 0) {
        Player* secondTarget = &party[secondTargetIndex];
        bossAiQueueMessage("Boss 3 amarra %s em tentacao!", secondTarget->name);
        applyCombatDamageToPlayer(secondTarget, 110);
    }

    boss3MarkTemptationTargets(firstTargetIndex, secondTargetIndex);
    bossAiQueueMessage("Boss 3 deixou dois alvos tentados. A proxima habilidade deles vai custar mais.");
}

static void boss3UsePhase2DebtAttack(void) {
    int debtValue = bossState.boss3Debt;
    int damage = 45 + (debtValue / 2);
    int afflictedCount = 0;

    for (int i = 0; i < PARTY_SIZE; i++) {
        Player* target = &party[i];
        if (!target->isAlive) continue;

        applyCombatDamageToPlayer(target, damage);

        if (!target->defenseGuardActive && (rand() % 100) < 50) {
            addStatusCondition(&target->statusList, STATUS_DEFENSE_DOWN, BOSS_STATUS_DURATION, 1.0f);
            bossAiQueuePlayerAfflictedMessage(i, STATUS_DEFENSE_DOWN);
            afflictedCount++;
        }
    }

    bossState.boss3Debt = 0;
    bossAiQueueMessage("Boss 3 consumiu a divida e atingiu a party.");
    if (afflictedCount > 0) {
        bossAiQueueMessage("A pressao abalou a defesa de alguns alvos.");
    }
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

int bossAiHasTemptationMark(int playerIndex);

void bossAiNotifyPlayerPhysicalAction(int playerIndex, int isPhysicalAction) {
    if (!bossState.active || bossState.kind != BOSS_KIND_2) return;
    if (!isPhysicalAction) return;
    if (playerIndex < 0 || playerIndex >= PARTY_SIZE) return;

    if (!bossState.trapMarks[playerIndex].active) return;

    boss2ApplyTrapEffect(playerIndex, bossState.trapMarks[playerIndex].effect);
}
