#ifndef ENEMY_H
#define ENEMY_H

#include "raylib.h"
#include "../core/collision.h"
#include "stats.h"
#include "status_condition.h"

typedef struct Enemy {
    char name[32];              /* Nome do inimigo */
    Vector2 position;           /* Posição no mundo */
    Texture2D texture;          /* Sprite do inimigo */
    Collider collider;          /* Área de colisão */
    Stats stats;                /* Atributos de combate */
    StatusList statusList;      /* Lista de condições de status ativas */
    int isAlive;                /* 1 se HP > 0, 0 se derrotado */
    int inCombat;               /* 1 se em combate, 0 se em exploração */
} Enemy;

// initEnemy - Inicializa um inimigo com valores padrão
void initEnemy(Enemy* enemy, const char* name);

// freeEnemy - Libera recursos do inimigo
void freeEnemy(Enemy* enemy);

// setEnemyStats - Configura os stats de um inimigo
void setEnemyStats(Enemy* enemy, int hp, int forca, int defesa, int velocidade);

// setEnemyRewards - Configura as recompensas do inimigo
void setEnemyRewards(Enemy* enemy, int exp, int gold);

// damageEnemy - Aplica dano a um inimigo
int damageEnemy(Enemy* enemy, int damage);

// healEnemy - Cura HP de um inimigo
void healEnemy(Enemy* enemy, int amount);

// applyStatusToEnemy - Aplica uma condição de status ao inimigo
void applyStatusToEnemy(Enemy* enemy, StatusType type, int turns, float intensity);

// processEnemyStatusEffects - Processa efeitos de status no turno
int processEnemyStatusEffects(Enemy* enemy);

// canEnemyAct - Verifica se o inimigo pode agir neste turno
int canEnemyAct(Enemy* enemy);

// getEnemyDamageModifier - Obtém o modificador de dano do inimigo
float getEnemyDamageModifier(Enemy* enemy);

// initEnemyWithTexture - Inicializa um inimigo com posição e textura
void initEnemyWithTexture(Enemy* enemy, const char* name, Vector2 position, const char* texturePath);

// drawEnemy - Renderiza um inimigo na tela
void drawEnemy(Enemy* enemy);

// unloadEnemy - Descarrega recursos do inimigo
void unloadEnemy(Enemy* enemy);

// getEnemyCollider - Obtém o retângulo de colisão do inimigo
Rectangle getEnemyCollider(Enemy* enemy);

#endif
