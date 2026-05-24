#ifndef GAME_H
#define GAME_H

#include "raylib.h"
#include "../entities/player.h"
#include "../entities/enemy.h"

#define MAX_ENEMIES 32

#define PARTY_SIZE 4

#define COMBAT_DETECTION_DISTANCE 200.0f

typedef struct {
    Enemy enemies[MAX_ENEMIES];
    int count;
} EnemyManager;

typedef struct {
    int inCombat;
    int enemyIndices[MAX_ENEMIES];
    int enemyCount;
} Combat;

extern Player party[PARTY_SIZE];
extern EnemyManager enemyManager;
extern Combat combat;
extern Texture2D mapTexture;
extern Camera2D camera;

void configureCameraForCurrentWorld(void);
void updateCameraTarget(Vector2 target);

void initParty(int applyGrowth, Vector2 spawnPosition);
void unloadParty(void);
void applyPartyDamage(int damage);

// initGame - Inicializa o jogo
void initGame();

// updateGame - Atualiza a lógica do jogo em cada frame
void updateGame();

// drawGame - Renderiza o jogo na tela
void drawGame();

// closeGame - Finaliza e libera recursos do jogo
void closeGame();

// resetGameState - Reinicia o estado de jogo sem fechar a janela
void resetGameState();

// initEnemyManager - Inicializa o gerenciador de inimigos
void initEnemyManager();

// spawnEnemy - Spawna um novo inimigo no mapa
void spawnEnemy(const char* name, Vector2 position, const char* texturePath, int maxEnemies);

// updateEnemies - Atualiza todos os inimigos
void updateEnemies(const Rectangle* blockers, int blockerCount);

// updateEnemiesInteraction - Verifica proximidade e início de combate (NOVO)
void updateEnemiesInteraction(Vector2 playerPos);

// drawEnemies - Renderiza todos os inimigos
void drawEnemies();

// getEnemyBlockers - Coleta retângulos de colisão de todos os inimigos
void getEnemyBlockers(Rectangle* outBlockers, int* outCount);

// unloadEnemyManager - Libera recursos do gerenciador de inimigos
void unloadEnemyManager();

Player* getPartyMembers(int* outCount);
Enemy* getEnemyManagerArray(int* outCount);

// startCombat - Inicia combate com inimigos próximos
void startCombat(Vector2 playerPos, float combatDistance);

// getCombatState - Verifica se há combate ativo
int getCombatState();

// getEnemiesInCombat - Obtém os inimigos participando do combate
Enemy** getEnemiesInCombat();

// getEnemyCombatCount - Obtém quantidade de inimigos em combate
int getEnemyCombatCount();

// endCombat - Finaliza o combate e volta ao estado de exploração
void endCombat();

#endif
