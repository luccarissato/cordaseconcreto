#include "combat_ui.h"

#include "raylib.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "../core/game.h"
#include "../core/state.h"
#include "../combat/combat.h"
#include "../combat/ability.h"
#include "../items/inventory.h"
#include "../items/item.h"
#include "../entities/status_condition.h"

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
    nextTurn();
    combatTurnEpoch++;
    processedTurnEpoch = -1;
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
            advanceCombatTurn();
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

    if (item->baseItem->type == ITEM_INFLICT_STATUS) {
        buildEnemyTargetList();
    } else {
        buildAllyTargetList(item->baseItem->type == ITEM_REVIVE);
    }

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

        if (item->baseItem->type == ITEM_INFLICT_STATUS) {
            if (targetIsEnemy && targetIndex >= 0 && targetIndex < enemyManager.count) {
                useItemOnStatusList(&enemyManager.enemies[targetIndex].statusList, item->baseItem);
                item->quantity--;
                if (item->quantity <= 0) {
                    removeItemNodeAndFree(item);
                }
                spendAndFinishTurn();
            }
            return;
        }

        if (!targetIsEnemy && targetIndex >= 0 && targetIndex < PARTY_SIZE) {
            consumeItem(&playerInventory, selectedItemNode, &party[targetIndex]);
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
            DrawTextureEx(party[i].front, (Vector2){ (float)x + 40, (float)y }, 0.0f, 0.25f, party[i].isAlive ? WHITE : GRAY);
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
    }
}

static void drawEnemyColumn() {
    for (int i = 0; i < combat.enemyCount; i++) {
        int worldEnemyIndex = combat.enemyIndices[i];
        if (worldEnemyIndex < 0 || worldEnemyIndex >= enemyManager.count) continue;

        Enemy* enemy = &enemyManager.enemies[worldEnemyIndex];
        int x = 1200;
        int y = 345 + (i * 165);

        if (enemy->texture.id != 0) {
            DrawTextureEx(enemy->texture, (Vector2){ (float)x, (float)y }, 0.0f, 0.55f, enemy->isAlive ? WHITE : GRAY);
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
        DrawText("HABILIDADES", 120, 850, 28, mainSelection == 1 ? YELLOW : RAYWHITE);
        DrawText("ITENS", 120, 910, 28, mainSelection == 2 ? YELLOW : RAYWHITE);
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
    resetCombatUiState();
}

void unloadCombatUI() {
    UnloadTexture(combatUiTexture);
    UnloadTexture(turnArrowTexture);
    UnloadTexture(targetArrowTexture);
}

void updateCombatUI() {
    if (!isCombatActive()) {
        combatWasActive = 0;
        currentGameState = STATE_EXPLORATION;
        return;
    }

    if (!combatWasActive) {
        resetCombatUiState();
        combatWasActive = 1;
    }

    advanceTurnFlow();

    if (!isCombatActive()) {
        combatWasActive = 0;
        currentGameState = STATE_EXPLORATION;
        return;
    }

    Combatant* combatant = getCurrentCombatantSafe();
    if (combatant == NULL || combatant->type != COMBATANT_PLAYER) {
        return;
    }

    if (combatUiState == COMBAT_UI_MAIN) {
        if (IsKeyPressed(KEY_DOWN)) {
            mainSelection++;
            if (mainSelection > 2) mainSelection = 0;
        }

        if (IsKeyPressed(KEY_UP)) {
            mainSelection--;
            if (mainSelection < 0) mainSelection = 2;
        }

        if (IsKeyPressed(KEY_Z)) {
            if (mainSelection == 0) {
                buildEnemyTargetList();
                combatUiState = COMBAT_UI_ATTACK_TARGET;
            } else if (mainSelection == 1) {
                refreshAvailableAbilities(&party[combatant->playerIndex]);
                combatUiState = COMBAT_UI_ABILITY_LIST;
            } else if (mainSelection == 2) {
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
    drawBottomPanel();
}