#include "combat_ui.h"

#include "raylib.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "../core/game.h"
#include "../core/state.h"
#include "../combat/combat.h"
#include "../combat/ability.h"
#include "../combat/boss_ai.h"
#include "../items/inventory.h"
#include "../items/item.h"
#include "../entities/status_condition.h"
/* Status visuals (auras / buffs) */
#include "../ui/status_visuals.h"

extern Inventory playerInventory;

typedef enum {
    COMBAT_UI_MAIN = 0,
    COMBAT_UI_ATTACK_TARGET,
    COMBAT_UI_ABILITY_LIST,
    COMBAT_UI_ABILITY_TARGET,
    COMBAT_UI_ITEM_LIST,
    COMBAT_UI_ITEM_TARGET
} CombatUiState;

static Texture2D combatUiTexture;
static Texture2D turnArrowTexture;
static Texture2D targetArrowTexture;
static Texture2D trapIconTexture;

static CombatUiState combatUiState = COMBAT_UI_MAIN;
static int mainSelection = 0;
static int abilitySelection = 0;
static int itemSelection = 0;
static int targetSelection = 0;

static int availableAbilityIndices[4];
static int availableAbilityCount = 0;

static ListNode* availableItemNodes[64];
static int availableItemCount = 0;

static int targetIndices[MAX_ENEMIES];
static int targetCount = 0;
static int targetIsEnemy = 1;
static ListNode* selectedItemNode = NULL;

static int combatTurnEpoch = 0;
static int processedTurnEpoch = -1;
static int combatWasActive = 0;

static Texture2D weaknessIcons[4];

static void drawPlayerWeaknessIcon(int playerIndex, int x, int y, float spriteScale) {
    if (playerIndex < 0 || playerIndex >= PARTY_SIZE) return;

    int weaknessElement = bossAiGetPlayerWeaknessElement(playerIndex);
    if (weaknessElement < 0 || weaknessElement >= 4) return;
    if (weaknessIcons[weaknessElement].id == 0) return;

    float iconScale = 0.65f;
    Vector2 iconSize = {
        weaknessIcons[weaknessElement].width * iconScale,
        weaknessIcons[weaknessElement].height * iconScale
    };

    float spriteWidth = party[playerIndex].front.width * spriteScale;
    Vector2 iconPos = {
        (float)x + 40.0f + (spriteWidth * 0.5f) - (iconSize.x * 0.5f),
        (float)y - iconSize.y - 12.0f
    };

    DrawTextureEx(weaknessIcons[weaknessElement], iconPos, 0.0f, iconScale, WHITE);
}

static void drawPlayerTrapIcon(int playerIndex, int x, int y, float spriteScale) {
    if (playerIndex < 0 || playerIndex >= PARTY_SIZE) return;
    if (!bossAiHasTrapMark(playerIndex)) return;
    if (trapIconTexture.id == 0) return;

    float iconScale = 0.65f;
    Vector2 iconSize = {
        trapIconTexture.width * iconScale,
        trapIconTexture.height * iconScale
    };

    float spriteWidth = party[playerIndex].front.width * spriteScale;
    Vector2 iconPos = {
        (float)x + 40.0f + (spriteWidth * 0.5f) - (iconSize.x * 0.5f),
        (float)y - iconSize.y - 12.0f
    };

    DrawTextureEx(trapIconTexture, iconPos, 0.0f, iconScale, WHITE);
}

static void executeEnemyAction(Enemy* enemy, int worldEnemyIndex) {
    if (enemy == NULL || !enemy->isAlive) return;

    if (bossAiHandleEnemyTurn(worldEnemyIndex, enemy)) {
        return;
    }

    if (!canEnemyAct(enemy)) {
        bossAiQueueMessage("%s falhou em agir.", enemy->name);
        return;
    }

    for (int i = 0; i < PARTY_SIZE; i++) {
        if (!party[i].isAlive) continue;
        applyCombatDamageToPlayer(&party[i], 30);
        bossAiQueueMessage("%s atacou %s.", enemy->name, party[i].name);
        break;
    }
}

static int isCombatItemVisible(InventoryItem* item) {
    if (item == NULL || item->baseItem == NULL) return 0;

    switch (item->baseItem->type) {
        case ITEM_HEAL:
        case ITEM_MANA:
        case ITEM_BUFF:
        case ITEM_CURE:
        case ITEM_REVIVE:
        case ITEM_STAT_BOOST:
        case ITEM_INFLICT_STATUS:
            return 1;

        case ITEM_KEY:
        default:
            return 0;
    }
}

static void clampItemSelection(void) {
    if (availableItemCount <= 0) {
        itemSelection = 0;
        return;
    }

    if (itemSelection < 0) {
        itemSelection = availableItemCount - 1;
    } else if (itemSelection >= availableItemCount) {
        itemSelection = 0;
    }
}

static void resetCombatUiState() {
    combatUiState = COMBAT_UI_MAIN;
    mainSelection = 0;
    abilitySelection = 0;
    itemSelection = 0;
    targetSelection = 0;
    targetCount = 0;
    targetIsEnemy = 1;
    selectedItemNode = NULL;
    combatTurnEpoch = 0;
    processedTurnEpoch = -1;
}

static void advanceCombatTurn() {
    int wrapped = (combatState.currentTurn + 1 >= combatState.combatantCount);
    nextTurn();
    combatTurnEpoch++;
    processedTurnEpoch = -1;

    if (wrapped) {
        bossAiOnRoundWrap();
    }
}

static Combatant* getCurrentCombatantSafe() {
    return isCombatActive() ? getCurrentCombatant() : NULL;
}

static void refreshAvailableAbilities(Player* player) {
    availableAbilityCount = 0;

    if (player == NULL) return;

    for (int i = 0; i < 4; i++) {
        Ability* ability = getAbilityByIndex(player->characterID, i);
        if (ability == NULL) continue;

        if (player->level >= ability->level_unlocked) {
            availableAbilityIndices[availableAbilityCount++] = i;
        }
    }

    if (availableAbilityCount == 0) {
        abilitySelection = 0;
    } else if (abilitySelection >= availableAbilityCount) {
        abilitySelection = 0;
    }
}

static void refreshAvailableItems() {
    availableItemCount = 0;

    if (playerInventory.items.head != NULL && playerInventory.items.size > 0) {
        ListNode* node = playerInventory.items.head;

        for (int i = 0; i < playerInventory.items.size && availableItemCount < 64; i++) {
            InventoryItem* item = (InventoryItem*)node->data;
            if (isCombatItemVisible(item)) {
                availableItemNodes[availableItemCount++] = node;
            }

            node = node->next;
        }
    }

    if (availableItemCount == 0) {
        itemSelection = 0;
    } else if (itemSelection >= availableItemCount) {
        itemSelection = 0;
    }
}

static int currentPlayerIndex() {
    Combatant* combatant = getCurrentCombatantSafe();
    if (combatant == NULL || combatant->type != COMBATANT_PLAYER) return -1;
    return combatant->playerIndex;
}

static int currentEnemyCombatIndex() {
    Combatant* combatant = getCurrentCombatantSafe();
    if (combatant == NULL || combatant->type != COMBATANT_ENEMY) return -1;
    return combatant->enemyIndex;
}

static void processPlayerTurnStart(Player* player) {
    if (player == NULL || !player->isAlive) return;

    /* Defender dura até o próximo turno próprio */
    player->defenseGuardActive = 0;
    player->defenseDamageReductionPending = 0;

    processStatusEffects(&player->statusList, &player->stats.currentHP, player->stats.maxHP);
    updateStatusDurations(&player->statusList);

    if (player->stats.currentHP <= 0) {
        player->stats.currentHP = 0;
        player->isAlive = 0;
    }
}

static void processEnemyTurnStart(Enemy* enemy) {
    if (enemy == NULL || !enemy->isAlive) return;

    processEnemyStatusEffects(enemy);
}

static void finishCombatIfNeeded() {
    int alivePlayers = 0;
    int aliveEnemies = 0;

    for (int i = 0; i < PARTY_SIZE; i++) {
        if (party[i].isAlive) alivePlayers++;
    }

    for (int i = 0; i < combat.enemyCount; i++) {
        int worldEnemyIndex = combat.enemyIndices[i];
        if (worldEnemyIndex >= 0 && worldEnemyIndex < enemyManager.count && enemyManager.enemies[worldEnemyIndex].isAlive) {
            aliveEnemies++;
        }
    }

    if (alivePlayers <= 0 || aliveEnemies <= 0) {
        endCombat();
    }
}

static void advanceTurnFlow() {
    int safety = 0;

    while (isCombatActive() && safety < 64) {
        Combatant* combatant = getCurrentCombatantSafe();
        if (combatant == NULL) return;

        if (processedTurnEpoch != combatTurnEpoch) {
            if (combatant->type == COMBATANT_PLAYER) {
                processPlayerTurnStart(&party[combatant->playerIndex]);
            } else if (combatant->type == COMBATANT_ENEMY) {
                int worldEnemyIndex = combat.enemyIndices[combatant->enemyIndex];
                if (worldEnemyIndex >= 0 && worldEnemyIndex < enemyManager.count) {
                    processEnemyTurnStart(&enemyManager.enemies[worldEnemyIndex]);
                }
            }

            processedTurnEpoch = combatTurnEpoch;
        }

        if (combatant->type == COMBATANT_ENEMY) {
            int worldEnemyIndex = combat.enemyIndices[combatant->enemyIndex];
            if (worldEnemyIndex >= 0 && worldEnemyIndex < enemyManager.count) {
                executeEnemyAction(&enemyManager.enemies[worldEnemyIndex], worldEnemyIndex);
            }

            advanceCombatTurn();
            finishCombatIfNeeded();
            safety++;
            continue;
        }

        if (!party[combatant->playerIndex].isAlive) {
            advanceCombatTurn();
            safety++;
            continue;
        }

        return;
    }
}

static void buildEnemyTargetList() {
    targetCount = 0;
    targetIsEnemy = 1;

    for (int i = 0; i < combat.enemyCount; i++) {
        int worldEnemyIndex = combat.enemyIndices[i];
        if (worldEnemyIndex < 0 || worldEnemyIndex >= enemyManager.count) continue;

        if (enemyManager.enemies[worldEnemyIndex].isAlive) {
            targetIndices[targetCount++] = worldEnemyIndex;
        }
    }

    if (targetCount == 0) {
        targetSelection = 0;
    } else if (targetSelection >= targetCount) {
        targetSelection = 0;
    }
}

static void buildAllyTargetList(int includeDeadAllies) {
    targetCount = 0;
    targetIsEnemy = 0;

    for (int i = 0; i < PARTY_SIZE; i++) {
        if (!includeDeadAllies && !party[i].isAlive) continue;
        targetIndices[targetCount++] = i;
    }

    if (targetCount == 0) {
        targetSelection = 0;
    } else if (targetSelection >= targetCount) {
        targetSelection = 0;
    }
}

static void spendAndFinishTurn() {
    combatUiState = COMBAT_UI_MAIN;
    selectedItemNode = NULL;
    advanceCombatTurn();
    finishCombatIfNeeded();
}

static void executeDefendAction(Player* player) {
    if (player == NULL || !player->isAlive) return;

    player->defenseGuardActive = 1;
    player->defenseDamageReductionPending = 1;
    spendAndFinishTurn();
}

static void executeSelectedAbility(Player* player) {
    if (player == NULL || availableAbilityCount <= 0) return;

    int abilityIndex = availableAbilityIndices[abilitySelection];
    Ability* ability = getAbilityByIndex(player->characterID, abilityIndex);
    if (ability == NULL || !canUseAbility(player, abilityIndex)) return;

    if (ability->target_type == TARGET_SELF) {
        int targetIndex = currentPlayerIndex();
        if (targetIndex >= 0) {
            int targetIndicesBuffer[1] = { targetIndex };
            useAbility(player, abilityIndex, party, PARTY_SIZE, targetIndicesBuffer, 1);
            spendAndFinishTurn();
        }
        return;
    }

    if (ability->target_type == TARGET_AREA_ENEMIES) {
        int enemyIndices[MAX_ENEMIES];
        int enemyCount = 0;

        for (int i = 0; i < combat.enemyCount; i++) {
            int worldEnemyIndex = combat.enemyIndices[i];
            if (worldEnemyIndex >= 0 && worldEnemyIndex < enemyManager.count && enemyManager.enemies[worldEnemyIndex].isAlive) {
                enemyIndices[enemyCount++] = worldEnemyIndex;
            }
        }

        if (enemyCount > 0) {
            useAbility(player, abilityIndex, enemyManager.enemies, enemyManager.count, enemyIndices, enemyCount);
            spendAndFinishTurn();
        }
        return;
    }

    if (ability->target_type == TARGET_AREA_ALLIES) {
        int allyIndices[PARTY_SIZE];
        int allyCount = 0;

        for (int i = 0; i < PARTY_SIZE; i++) {
            if (party[i].isAlive) {
                allyIndices[allyCount++] = i;
            }
        }

        if (allyCount > 0) {
            useAbility(player, abilityIndex, party, PARTY_SIZE, allyIndices, allyCount);
            spendAndFinishTurn();
        }
        return;
    }

    if (ability->target_type == TARGET_SINGLE_ENEMY) {
        buildEnemyTargetList();
        combatUiState = COMBAT_UI_ABILITY_TARGET;
        return;
    }

    if (ability->target_type == TARGET_SINGLE_ALLY) {
        buildAllyTargetList(ability->ability_type == ABILITY_TYPE_REVIVE);
        combatUiState = COMBAT_UI_ABILITY_TARGET;
        return;
    }
}

static void executeSelectedItem() {
    if (availableItemCount <= 0) return;

    selectedItemNode = availableItemNodes[itemSelection];
    InventoryItem* item = (InventoryItem*)selectedItemNode->data;
    if (item == NULL || item->baseItem == NULL) return;

    buildAllyTargetList(item->baseItem->type == ITEM_REVIVE);

    combatUiState = COMBAT_UI_ITEM_TARGET;
}

static void removeItemNodeAndFree(InventoryItem* item) {
    if (selectedItemNode == NULL || item == NULL) return;

    removeNode(&playerInventory.items, selectedItemNode);
    free(item);
    selectedItemNode = NULL;
}

static void confirmTargetAction() {
    Player* player = currentPlayerIndex() >= 0 ? &party[currentPlayerIndex()] : NULL;
    if (player == NULL || targetCount <= 0 || targetSelection < 0 || targetSelection >= targetCount) return;

    int targetIndex = targetIndices[targetSelection];

    if (combatUiState == COMBAT_UI_ATTACK_TARGET) {
        if (targetIsEnemy && targetIndex >= 0 && targetIndex < enemyManager.count) {
            int damage = 10 + player->stats.forca * 2;
            damageEnemy(&enemyManager.enemies[targetIndex], damage);
            bossAiNotifyPlayerPhysicalAction(currentPlayerIndex(), 1);
            spendAndFinishTurn();
        }
        return;
    }

    if (combatUiState == COMBAT_UI_ABILITY_TARGET) {
        int abilityIndex = availableAbilityIndices[abilitySelection];
        Ability* ability = getAbilityByIndex(player->characterID, abilityIndex);
        if (ability == NULL) return;

        if (targetIsEnemy) {
            int targetIndicesBuffer[1] = { targetIndex };
            useAbility(player, abilityIndex, enemyManager.enemies, enemyManager.count, targetIndicesBuffer, 1);
        } else {
            int targetIndicesBuffer[1] = { targetIndex };
            useAbility(player, abilityIndex, party, PARTY_SIZE, targetIndicesBuffer, 1);
        }

        spendAndFinishTurn();
        return;
    }

    if (combatUiState == COMBAT_UI_ITEM_TARGET) {
        InventoryItem* item = (InventoryItem*)selectedItemNode->data;
        if (item == NULL || item->baseItem == NULL) return;
        Item* baseItem = item->baseItem;

        if (targetIsEnemy && targetIndex >= 0 && targetIndex < enemyManager.count) {
            useItemOnEnemy(&enemyManager.enemies[targetIndex], item);
            item->quantity--;
            if (item->quantity <= 0) {
                removeItemNodeAndFree(item);
            }
            spendAndFinishTurn();
            return;
        }

        if (!targetIsEnemy && targetIndex >= 0 && targetIndex < PARTY_SIZE) {
            consumeItem(&playerInventory, selectedItemNode, &party[targetIndex]);
            if (baseItem->type == ITEM_INFLICT_STATUS && baseItem->statusToApply != STATUS_NONE && !party[targetIndex].defenseGuardActive) {
                bossAiQueuePlayerAfflictedMessage(targetIndex, baseItem->statusToApply);
            }
            selectedItemNode = NULL;
            spendAndFinishTurn();
        }
    }
}

static void drawTopInitiativeBar() {
    DrawRectangle(0, 0, 1920, 90, ColorAlpha(BLACK, 0.72f));
    DrawRectangleLines(0, 0, 1920, 90, DARKGRAY);

    Combatant* combatant = getCurrentCombatantSafe();
    const char* nextTurnName = "-";

    if (combatant != NULL) {
        int nextTurnIndex = combatState.currentTurn + 1;
        if (nextTurnIndex >= combatState.combatantCount) {
            nextTurnIndex = 0;
        }

        Combatant* nextCombatant = &combatState.combatants[nextTurnIndex];

        if (nextCombatant->type == COMBATANT_PLAYER) {
            nextTurnName = party[nextCombatant->playerIndex].name;
        } else {
            int worldEnemyIndex = combat.enemyIndices[nextCombatant->enemyIndex];
            if (worldEnemyIndex >= 0 && worldEnemyIndex < enemyManager.count) {
                nextTurnName = enemyManager.enemies[worldEnemyIndex].name;
            }
        }
    }

    DrawText(TextFormat("PRÓXIMO TURNO: %s", nextTurnName), 790, 30, 28, RAYWHITE);
}

static void drawPlayers() {
    for (int i = 0; i < PARTY_SIZE; i++) {
        int x = 90;
        int y = 125 + (i * 165);

        if (party[i].front.id != 0) {
            Vector2 spritePos = {(float)x + 40, (float)y};
            Rectangle srcRect = {0, 0, (float)party[i].front.width, (float)party[i].front.height};
            
            renderStatusAura(party[i].front, srcRect, spritePos, (Vector2){0, 0}, 0.25f, &party[i].statusList);
            
            DrawTextureEx(party[i].front, spritePos, 0.0f, 0.25f, party[i].isAlive ? WHITE : GRAY);
            
            Vector2 spriteSize = {party[i].front.width * 0.25f, party[i].front.height * 0.25f};
            renderStatusBuffs(&party[i].statusList, spritePos, spriteSize);
        } else {
            DrawRectangle(x, y, 120, 120, party[i].isAlive ? DARKBLUE : DARKGRAY);
        }

        if (targetIsEnemy == 0 && (combatUiState == COMBAT_UI_ATTACK_TARGET || combatUiState == COMBAT_UI_ABILITY_TARGET || combatUiState == COMBAT_UI_ITEM_TARGET)) {
            if (targetSelection < targetCount && targetIndices[targetSelection] == i) {
                DrawTexture(targetArrowTexture, x - 35, y + 45, WHITE);
            }
        }

        if (currentPlayerIndex() == i) {
            DrawTexture(turnArrowTexture, x - 35, y + 10, WHITE);
        }

        DrawText(party[i].name, x + 145, y + - 10, 24, RAYWHITE);
        DrawText(TextFormat("HP %d/%d", party[i].stats.currentHP, party[i].stats.maxHP), x + 145, y + 24, 20, GREEN);
        DrawText(TextFormat("MP %d/%d", party[i].stats.currentMana, party[i].stats.maxMana), x + 145, y + 52, 20, SKYBLUE);

        drawPlayerWeaknessIcon(i, x, y, 0.25f);
        drawPlayerTrapIcon(i, x, y, 0.25f);
    }
}

static void drawCombatMessageBox(void) {
    const char* message = bossAiGetCurrentMessage();
    if (message == NULL) return;

    DrawRectangle(220, 740, 1480, 180, ColorAlpha(BLACK, 0.85f));
    DrawRectangleLines(220, 740, 1480, 180, GRAY);
    DrawText(message, 260, 810, 30, RAYWHITE);
}

static void drawEnemyColumn() {
    for (int i = 0; i < combat.enemyCount; i++) {
        int worldEnemyIndex = combat.enemyIndices[i];
        if (worldEnemyIndex < 0 || worldEnemyIndex >= enemyManager.count) continue;

        Enemy* enemy = &enemyManager.enemies[worldEnemyIndex];
        int x = 1200;
        int y = 345 + (i * 165);

        if (enemy->texture.id != 0) {
            Vector2 spritePos = {(float)x, (float)y};
            Rectangle srcRect = {0, 0, (float)enemy->texture.width, (float)enemy->texture.height};
            
            renderStatusAura(enemy->texture, srcRect, spritePos, (Vector2){0, 0}, 0.55f, &enemy->statusList);
            
            DrawTextureEx(enemy->texture, spritePos, 0.0f, 0.55f, enemy->isAlive ? WHITE : GRAY);
            
            Vector2 spriteSize = {enemy->texture.width * 0.55f, enemy->texture.height * 0.55f};
            renderStatusBuffs(&enemy->statusList, spritePos, spriteSize);
        } else {
            DrawRectangle(x, y, 120, 120, enemy->isAlive ? MAROON : DARKGRAY);
        }

        if (targetIsEnemy == 1 && (combatUiState == COMBAT_UI_ATTACK_TARGET || combatUiState == COMBAT_UI_ABILITY_TARGET || combatUiState == COMBAT_UI_ITEM_TARGET)) {
            if (targetSelection < targetCount && targetIndices[targetSelection] == worldEnemyIndex) {
                DrawTexture(targetArrowTexture, x - 35, y + 45, WHITE);
            }
        }

        if (currentEnemyCombatIndex() == i) {
            DrawTexture(turnArrowTexture, x - 35, y + 10, WHITE);
        }

        DrawText(enemy->name, x + 40, y - 50, 24, RAYWHITE);
        DrawText(TextFormat("HP %d/%d", enemy->stats.currentHP, enemy->stats.maxHP), x + 25, y - 20, 20, PINK);
    }
}

static void drawBottomPanel() {
    if (combatUiTexture.id != 0) {
        DrawTexture(combatUiTexture, 0, 730, WHITE);
    } else {
        DrawRectangle(0, 730, 1920, 350, ColorAlpha(BLACK, 0.85f));
    }

    if (combatUiState == COMBAT_UI_MAIN) {
        //substituir por textura eventualmente ou só pela fonte com drawTextEx
        DrawText("ATACAR", 120, 790, 28, mainSelection == 0 ? YELLOW : RAYWHITE);
        DrawText("DEFENDER", 120, 850, 28, mainSelection == 1 ? YELLOW : RAYWHITE);
        DrawText("HABILIDADES", 120, 910, 28, mainSelection == 2 ? YELLOW : RAYWHITE);
        DrawText("ITENS", 120, 970, 28, mainSelection == 3 ? YELLOW : RAYWHITE);
        DrawTexture(turnArrowTexture, 58, 795 + (mainSelection * 60), WHITE);
    }

    if (combatUiState == COMBAT_UI_ABILITY_LIST) {
        Player* player = currentPlayerIndex() >= 0 ? &party[currentPlayerIndex()] : NULL;
        DrawText("HABILIDADES", 120, 780, 26, RAYWHITE);

        for (int i = 0; i < availableAbilityCount; i++) {
            Ability* ability = getAbilityByIndex(player->characterID, availableAbilityIndices[i]);
            if (ability == NULL) continue;

            Color color = (i == abilitySelection) ? YELLOW : RAYWHITE;
            DrawText(ability->name, 120, 825 + (i * 42), 24, color);
            DrawText(TextFormat("Lv.%d | MP %d", ability->level_unlocked, ability->mana_cost), 430, 825 + (i * 42), 20, LIGHTGRAY);
        }

        if (availableAbilityCount > 0) {
            Ability* ability = getAbilityByIndex(player->characterID, availableAbilityIndices[abilitySelection]);
            if (ability != NULL) {
                /* Gera descrição dinâmica baseada no modo/elemento atual */
                char dynamicDescription[512];
                getAbilityDynamicDescription(ability, dynamicDescription, sizeof(dynamicDescription));
                DrawText(dynamicDescription, 840, 820, 22, LIGHTGRAY);
                DrawText(TextFormat("Alvo: %d", ability->target_type), 840, 856, 20, GRAY);
            }
        }

        DrawTexture(turnArrowTexture, 58, 830 + (abilitySelection * 42), WHITE);
    }

    if (combatUiState == COMBAT_UI_ITEM_LIST) {
        DrawText("ITENS", 120, 780, 26, RAYWHITE);

        for (int i = 0; i < availableItemCount; i++) {
            InventoryItem* item = (InventoryItem*)availableItemNodes[i]->data;
            if (item == NULL || item->baseItem == NULL) continue;

            Color color = (i == itemSelection) ? YELLOW : RAYWHITE;
            DrawText(TextFormat("%s x%d", item->baseItem->name, item->quantity), 120, 825 + (i * 42), 24, color);
        }

        if (availableItemCount > 0) {
            InventoryItem* item = (InventoryItem*)availableItemNodes[itemSelection]->data;
            if (item != NULL && item->baseItem != NULL) {
                DrawText(item->baseItem->description, 840, 820, 22, LIGHTGRAY);
            }
        }

        DrawTexture(turnArrowTexture, 58, 830 + (itemSelection * 42), WHITE);
    }

    if (combatUiState == COMBAT_UI_ATTACK_TARGET || combatUiState == COMBAT_UI_ABILITY_TARGET || combatUiState == COMBAT_UI_ITEM_TARGET) {
        DrawText(targetIsEnemy ? "SELECIONE O INIMIGO" : "SELECIONE O ALIADO", 120, 780, 26, RAYWHITE);

        for (int i = 0; i < targetCount; i++) {
            int targetIndex = targetIndices[i];
            const char* name = "-";
            int hp = 0;
            int maxHp = 1;

            if (targetIsEnemy) {
                if (targetIndex >= 0 && targetIndex < enemyManager.count) {
                    name = enemyManager.enemies[targetIndex].name;
                    hp = enemyManager.enemies[targetIndex].stats.currentHP;
                    maxHp = enemyManager.enemies[targetIndex].stats.maxHP;
                }
            } else {
                if (targetIndex >= 0 && targetIndex < PARTY_SIZE) {
                    name = party[targetIndex].name;
                    hp = party[targetIndex].stats.currentHP;
                    maxHp = party[targetIndex].stats.maxHP;
                }
            }

            Color color = (i == targetSelection) ? YELLOW : RAYWHITE;
            DrawText(name, 120, 825 + (i * 42), 24, color);
            DrawText(TextFormat("HP %d/%d", hp, maxHp), 430, 825 + (i * 42), 20, LIGHTGRAY);
        }

        DrawTexture(targetArrowTexture, 58, 830 + (targetSelection * 42), WHITE);
    }
}

void initCombatUI() {
    combatUiTexture = LoadTexture("assets/interface/ui_combate_placeholder.png");
    turnArrowTexture = LoadTexture("assets/interface/seta_placeholder.png");
    targetArrowTexture = LoadTexture("assets/interface/seta_alvo_placeholder.png");
    trapIconTexture = LoadTexture("assets/icones/trap.png");

    weaknessIcons[0] = LoadTexture("assets/icones/fogo_icon.png");
    weaknessIcons[1] = LoadTexture("assets/icones/vento_icon.png");
    weaknessIcons[2] = LoadTexture("assets/icones/mare_icon.png");
    weaknessIcons[3] = LoadTexture("assets/icones/terra_icon.png");

    resetCombatUiState();
}

void unloadCombatUI() {
    UnloadTexture(combatUiTexture);
    UnloadTexture(turnArrowTexture);
    UnloadTexture(targetArrowTexture);
    UnloadTexture(trapIconTexture);

    for (int i = 0; i < 4; i++) {
        if (weaknessIcons[i].id != 0) {
            UnloadTexture(weaknessIcons[i]);
        }
    }
}

void updateCombatUI() {
    if (!isCombatActive()) {
        bossAiOnCombatEnd();
        combatWasActive = 0;
        if (currentGameState == STATE_COMBAT) {
            currentGameState = STATE_EXPLORATION;
        }
        return;
    }

    if (!combatWasActive) {
        resetCombatUiState();
        bossAiOnCombatStart();
        combatWasActive = 1;
    }

    bossAiUpdateMessages(GetFrameTime());
    if (bossAiHasActiveMessage()) {
        return;
    }

    advanceTurnFlow();

    if (!isCombatActive()) {
        bossAiOnCombatEnd();
        combatWasActive = 0;
        if (currentGameState == STATE_COMBAT) {
            currentGameState = STATE_EXPLORATION;
        }
        return;
    }

    if (bossAiHasActiveMessage()) {
        return;
    }

    Combatant* combatant = getCurrentCombatantSafe();
    if (combatant == NULL || combatant->type != COMBATANT_PLAYER) {
        return;
    }

    if (combatUiState == COMBAT_UI_MAIN) {
        if (IsKeyPressed(KEY_DOWN)) {
            mainSelection++;
            if (mainSelection > 3) mainSelection = 0;
        }

        if (IsKeyPressed(KEY_UP)) {
            mainSelection--;
            if (mainSelection < 0) mainSelection = 3;
        }

        if (IsKeyPressed(KEY_Z)) {
            if (mainSelection == 0) {
                buildEnemyTargetList();
                combatUiState = COMBAT_UI_ATTACK_TARGET;
            } else if (mainSelection == 1) {
                executeDefendAction(&party[combatant->playerIndex]);
            } else if (mainSelection == 2) {
                refreshAvailableAbilities(&party[combatant->playerIndex]);
                combatUiState = COMBAT_UI_ABILITY_LIST;
            } else if (mainSelection == 3) {
                refreshAvailableItems();
                if (availableItemCount > 0) {
                    combatUiState = COMBAT_UI_ITEM_LIST;
                } else {
                    combatUiState = COMBAT_UI_MAIN;
                }
            }
        }
    } else if (combatUiState == COMBAT_UI_ABILITY_LIST) {
        if (IsKeyPressed(KEY_DOWN)) {
            abilitySelection++;
            if (abilitySelection >= availableAbilityCount) abilitySelection = 0;
        }

        if (IsKeyPressed(KEY_UP)) {
            abilitySelection--;
            if (abilitySelection < 0) abilitySelection = availableAbilityCount - 1;
        }

        if (IsKeyPressed(KEY_LEFT)) {
            /* Alterna modo/elemento da habilidade selecionada */
            Ability* selectedAbility = getAbilityByIndex(party[combatant->playerIndex].characterID, availableAbilityIndices[abilitySelection]);
            if (selectedAbility != NULL && selectedAbility->is_alternatable) {
                /* Verifica se é elemental (Character 4) ou de buff (Character 3) */
                if (selectedAbility->characterID == CHARACTER_4_MAGE && (selectedAbility->ability_index == 0 || selectedAbility->ability_index == 2)) {
                    toggleAbilityElement(selectedAbility, -1);
                } else if (selectedAbility->characterID == CHARACTER_3_HEALER && selectedAbility->ability_index == 2) {
                    toggleAbilityMode(selectedAbility, -1);
                }
            }
        }

        if (IsKeyPressed(KEY_RIGHT)) {
            /* Alterna modo/elemento da habilidade selecionada */
            Ability* selectedAbility = getAbilityByIndex(party[combatant->playerIndex].characterID, availableAbilityIndices[abilitySelection]);
            if (selectedAbility != NULL && selectedAbility->is_alternatable) {
                /* Verifica se é elemental (Character 4) ou de buff (Character 3) */
                if (selectedAbility->characterID == CHARACTER_4_MAGE && (selectedAbility->ability_index == 0 || selectedAbility->ability_index == 2)) {
                    toggleAbilityElement(selectedAbility, 1);
                } else if (selectedAbility->characterID == CHARACTER_3_HEALER && selectedAbility->ability_index == 2) {
                    toggleAbilityMode(selectedAbility, 1);
                }
            }
        }

        if (IsKeyPressed(KEY_Z)) {
            executeSelectedAbility(&party[combatant->playerIndex]);
        }

        if (IsKeyPressed(KEY_X)) {
            combatUiState = COMBAT_UI_MAIN;
        }
    } else if (combatUiState == COMBAT_UI_ITEM_LIST) {
        if (IsKeyPressed(KEY_DOWN)) {
            itemSelection++;
            clampItemSelection();
        }

        if (IsKeyPressed(KEY_UP)) {
            itemSelection--;
            clampItemSelection();
        }

        if (IsKeyPressed(KEY_Z)) {
            executeSelectedItem();
        }

        if (IsKeyPressed(KEY_X)) {
            combatUiState = COMBAT_UI_MAIN;
        }
    } else if (combatUiState == COMBAT_UI_ATTACK_TARGET ||
               combatUiState == COMBAT_UI_ABILITY_TARGET ||
               combatUiState == COMBAT_UI_ITEM_TARGET) {
        if (IsKeyPressed(KEY_DOWN)) {
            targetSelection++;
            if (targetSelection >= targetCount) targetSelection = 0;
        }

        if (IsKeyPressed(KEY_UP)) {
            targetSelection--;
            if (targetSelection < 0) targetSelection = targetCount - 1;
        }

        if (IsKeyPressed(KEY_RIGHT)) {
            if (combatUiState == COMBAT_UI_ITEM_TARGET && !targetIsEnemy) {
                buildEnemyTargetList();
                targetSelection = 0;
            }
        }

        if (IsKeyPressed(KEY_LEFT)) {
            if (combatUiState == COMBAT_UI_ITEM_TARGET && targetIsEnemy) {
                buildAllyTargetList(0);
                targetSelection = 0;
            }
        }

        if (IsKeyPressed(KEY_Z)) {
            confirmTargetAction();
        }

        if (IsKeyPressed(KEY_X)) {
            combatUiState = COMBAT_UI_MAIN;
        }
    }

    finishCombatIfNeeded();
}

void drawCombatUI() {
    ClearBackground((Color){ 20, 20, 26, 255 });
    drawTopInitiativeBar();
    drawPlayers();
    drawEnemyColumn();

    if (bossAiHasActiveMessage()) {
        drawCombatMessageBox();
    } else {
        drawBottomPanel();
    }
}