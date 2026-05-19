#include "enemy.h"
#include "../core/game.h"
#include "../core/collision.h"
#include "../core/state.h"
#include <string.h>
#include <stdlib.h>

/* Variável global de game.c que é necessária */
extern EnemyManager enemyManager;

// initEnemy - Inicializa um inimigo com valores padrão
void initEnemy(Enemy* enemy, const char* name) {
    if (enemy == NULL) return;
    
    strncpy(enemy->name, name, 31);
    enemy->name[31] = '\0';
    
    enemy->stats.baseHP = 50;
    enemy->stats.maxHP = 50;
    enemy->stats.currentHP = 50;
    enemy->stats.baseMana = 20;
    enemy->stats.maxMana = 20;
    enemy->stats.currentMana = 20;
    enemy->stats.fortitude = 5;
    enemy->stats.mente = 5;
    enemy->stats.forca = 5;
    enemy->stats.defesa = 5;
    enemy->stats.velocidade = 5;
    enemy->stats.defCalor = 0;
    enemy->stats.defVento = 0;
    enemy->stats.defMare = 0;
    enemy->stats.defTerra = 0;
    
    initStatusList(&enemy->statusList);
    
    enemy->isAlive = 1;
}

// freeEnemy - Libera recursos do inimigo
void freeEnemy(Enemy* enemy) {
    if (enemy == NULL) return;
    
    freeStatusList(&enemy->statusList);
}

// setEnemyStats - Configura os stats de um inimigo
void setEnemyStats(Enemy* enemy, int hp, int forca, int defesa, int velocidade) {
    if (enemy == NULL) return;
    
    enemy->stats.baseHP = hp;
    enemy->stats.maxHP = hp;
    enemy->stats.currentHP = hp;
    enemy->stats.forca = forca;
    enemy->stats.defesa = defesa;
    enemy->stats.velocidade = velocidade;
}

// damageEnemy - Aplica dano a um inimigo
int damageEnemy(Enemy* enemy, int damage) {
    if (enemy == NULL || !enemy->isAlive) return 0;
    
    float defenseModifier = getDefenseModifier(&enemy->statusList);
    
    int effectiveDefense = (int)(enemy->stats.defesa * defenseModifier);
    int effectiveDamage = damage - effectiveDefense;
    
    if (effectiveDamage < 1) effectiveDamage = 1;
    
    enemy->stats.currentHP -= effectiveDamage;
    
    if (enemy->stats.currentHP <= 0) {
        enemy->stats.currentHP = 0;
        enemy->isAlive = 0;
    }
    
    return effectiveDamage;
}

// healEnemy - Cura HP de um inimigo
void healEnemy(Enemy* enemy, int amount) {
    if (enemy == NULL || !enemy->isAlive) return;
    
    enemy->stats.currentHP += amount;
    
    if (enemy->stats.currentHP > enemy->stats.maxHP) {
        enemy->stats.currentHP = enemy->stats.maxHP;
    }
}

// applyStatusToEnemy - Aplica uma condição de status ao inimigo
void applyStatusToEnemy(Enemy* enemy, StatusType type, int turns, float intensity) {
    if (enemy == NULL || !enemy->isAlive) return;
    
    addStatusCondition(&enemy->statusList, type, turns, intensity);
}

// processEnemyStatusEffects - Processa efeitos de status no turno
int processEnemyStatusEffects(Enemy* enemy) {
    if (enemy == NULL || !enemy->isAlive) return 0;
    
    int damage = processStatusEffects(
        &enemy->statusList,
        &enemy->stats.currentHP,
        enemy->stats.maxHP
    );
    
    if (enemy->stats.currentHP <= 0) {
        enemy->stats.currentHP = 0;
        enemy->isAlive = 0;
    }
    
    updateStatusDurations(&enemy->statusList);
    
    return damage;
}

// canEnemyAct - Verifica se o inimigo pode agir neste turno
int canEnemyAct(Enemy* enemy) {
    if (enemy == NULL || !enemy->isAlive) return 0;
    
    return canActThisTurn(&enemy->statusList);
}

// getEnemyDamageModifier - Obtém o modificador de dano do inimigo
float getEnemyDamageModifier(Enemy* enemy) {
    if (enemy == NULL) return 1.0f;
    
    return getStrengthModifier(&enemy->statusList);
}

// initEnemyWithTexture - Inicializa um inimigo com posição e textura
void initEnemyWithTexture(Enemy* enemy, const char* name, Vector2 position, const char* texturePath) {
    if (enemy == NULL) return;
    
    strncpy(enemy->name, name, 31);
    enemy->name[31] = '\0';
    
    enemy->position = position;
    
    enemy->texture = LoadTexture(texturePath);
    
    enemy->collider.offset = (Vector2){ 75.0f, 0.0f };
    enemy->collider.size = (Vector2){ 150.0f, 300.0f };
    
    enemy->stats.baseHP = 50;
    enemy->stats.maxHP = 50;
    enemy->stats.currentHP = 50;
    enemy->stats.baseMana = 20;
    enemy->stats.maxMana = 20;
    enemy->stats.currentMana = 20;
    enemy->stats.fortitude = 5;
    enemy->stats.mente = 5;
    enemy->stats.forca = 5;
    enemy->stats.defesa = 5;
    enemy->stats.velocidade = 5;
    enemy->stats.defCalor = 0;
    enemy->stats.defVento = 0;
    enemy->stats.defMare = 0;
    enemy->stats.defTerra = 0;
    
    initStatusList(&enemy->statusList);
    
    enemy->isAlive = 1;
    enemy->inCombat = 0;
}

// drawEnemy - Renderiza um inimigo na tela
void drawEnemy(Enemy* enemy) {
    if (enemy == NULL) return;

    DrawTexture(enemy->texture, (int)enemy->position.x, (int)enemy->position.y, WHITE);
    
    Rectangle colliderRect = getColliderRect(enemy->position, enemy->collider);
    DrawRectangleLinesEx(colliderRect, 2.0f, RED);
}

// unloadEnemy - Descarrega recursos do inimigo
void unloadEnemy(Enemy* enemy) {
    if (enemy == NULL) return;
    
    UnloadTexture(enemy->texture);
    
    freeStatusList(&enemy->statusList);
}

// getEnemyCollider - Obtém o retângulo de colisão do inimigo
Rectangle getEnemyCollider(Enemy* enemy) {
    if (enemy == NULL) {
        return (Rectangle){0, 0, 0, 0};
    }
    
    return getColliderRect(enemy->position, enemy->collider);
}

Enemy* getEnemyManagerArray(int* outCount) {
    if (outCount != NULL) {
        *outCount = enemyManager.count;
    }

    return enemyManager.enemies;
}

void initEnemyManager() {
    enemyManager.count = 0;
    memset(enemyManager.enemies, 0, sizeof(enemyManager.enemies));
}

void spawnEnemy(const char* name, Vector2 position, const char* texturePath, int maxEnemies) {
    if (enemyManager.count >= maxEnemies) {
        return;
    }
    
    initEnemyWithTexture(&enemyManager.enemies[enemyManager.count], name, position, texturePath);
    enemyManager.count++;
}

void updateEnemies(const Rectangle* blockers, int blockerCount) {
    for (int i = 0; i < enemyManager.count; i++) {
        Enemy* enemy = &enemyManager.enemies[i];
        
        if (!enemy->isAlive) continue;
        
        /* Processa status effects no inimigo */
        processEnemyStatusEffects(enemy);
        
        /* Aqui será adicionada lógica de IA no futuro */
    }
}

void updateEnemiesInteraction(Vector2 playerPos) {
    if (currentGameState != STATE_EXPLORATION) return;
    
    Collider playerCollider = {
        .offset = {75.0f, 0.0f},
        .size = {150.0f, 300.0f}
    };
    
    float interactionDistance = 80.0f;
    
    for (int i = 0; i < enemyManager.count; i++) {
        Enemy* enemy = &enemyManager.enemies[i];
        if (!enemy->isAlive) continue;
        
        int isNear = areCollidersNearEdgeBased(
            playerPos, playerCollider,
            enemy->position, enemy->collider,
            interactionDistance
        );
        
        if (isNear) {
            if (IsKeyPressed(KEY_Z)) {
                startCombat(playerPos, COMBAT_DETECTION_DISTANCE);
                return;
            }
        }
    }
}

void drawEnemies() {
    for (int i = 0; i < enemyManager.count; i++) {
        if (enemyManager.enemies[i].isAlive) {
            drawEnemy(&enemyManager.enemies[i]);
        }
    }
}

void getEnemyBlockers(Rectangle* outBlockers, int* outCount) {
    if (outBlockers == NULL || outCount == NULL) return;
    
    *outCount = 0;
    
    for (int i = 0; i < enemyManager.count; i++) {
        if (enemyManager.enemies[i].isAlive) {
            outBlockers[*outCount] = getEnemyCollider(&enemyManager.enemies[i]);
            (*outCount)++;
        }
    }
}

void unloadEnemyManager() {
    for (int i = 0; i < enemyManager.count; i++) {
        unloadEnemy(&enemyManager.enemies[i]);
    }
    enemyManager.count = 0;
}