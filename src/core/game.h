/**
 * =============================================================================
 * GAME.H - Gerenciador Principal do Jogo
 * =============================================================================
 * 
 * Define a estrutura e funções centrais para gerenciar o estado do jogo,
 * incluindo exploração, combate, inimigos e câmera.
 * 
 * =============================================================================
 */

#ifndef GAME_H
#define GAME_H

#include "raylib.h"
#include "../entities/player.h"
#include "../entities/enemy.h"

/* Número máximo de inimigos no mapa */
#define MAX_ENEMIES 32

/* Distância máxima para detecção de combate (proximidade) */
#define COMBAT_DETECTION_DISTANCE 200.0f

/* =============================================================================
 * ESTRUTURA: EnemyManager
 * =============================================================================
 * Gerencia todos os inimigos presentes no mapa durante exploração.
 * 
 * Campos:
 *   - enemies: Array de inimigos presentes no mapa
 *   - count: Número de inimigos atualmente no mapa
 * -------------------------------------------------------------------------- */
typedef struct {
    Enemy enemies[MAX_ENEMIES];
    int count;
} EnemyManager;

/* =============================================================================
 * ESTRUTURA: Combat
 * =============================================================================
 * Gerencia o estado de combate quando o jogador entra em batalha.
 * 
 * Campos:
 *   - inCombat: Flag indicando se há combate ativo (1) ou não (0)
 *   - enemyIndices: Índices dos inimigos participando do combate
 *   - enemyCount: Quantidade de inimigos no combate
 * -------------------------------------------------------------------------- */
typedef struct {
    int inCombat;
    int enemyIndices[MAX_ENEMIES];
    int enemyCount;
} Combat;

/* =============================================================================
 * FUNÇÕES PRINCIPAIS DO JOGO
 * ============================================================================= */

/**
 * initGame - Inicializa o jogo
 * 
 * Cria janela, carrega mapas, inicializa party, inimigos e sistemas.
 */
void initGame();

/**
 * updateGame - Atualiza a lógica do jogo em cada frame
 * 
 * Atualiza jogadores, inimigos, câmera e detecta transições de estado.
 */
void updateGame();

/**
 * drawGame - Renderiza o jogo na tela
 * 
 * Desenha mapa, entidades, UI e efeitos visuais.
 */
void drawGame();

/**
 * closeGame - Finaliza e libera recursos do jogo
 * 
 * Descarrega texturas, libera memória e fecha a janela.
 */
void closeGame();

/* =============================================================================
 * FUNÇÕES DE INIMIGOS
 * ============================================================================= */

/**
 * initEnemyManager - Inicializa o gerenciador de inimigos
 * 
 * Prepara o sistema para gerenciar inimigos no mapa.
 */
void initEnemyManager();

/**
 * spawnEnemy - Spawna um novo inimigo no mapa
 * @param name: Nome do inimigo
 * @param position: Posição inicial
 * @param texturePath: Caminho da textura
 * 
 * Adiciona um inimigo à lista de inimigos ativos.
 */
void spawnEnemy(const char* name, Vector2 position, const char* texturePath);

/**
 * updateEnemies - Atualiza todos os inimigos
 * @param blockers: Array de retângulos de colisão
 * @param blockerCount: Quantidade de blockers
 * 
 * Processa IA, status e colisão dos inimigos (futura implementação).
 */
void updateEnemies(const Rectangle* blockers, int blockerCount);

/**
 * drawEnemies - Renderiza todos os inimigos
 * 
 * Desenha cada inimigo ativo no mapa.
 */
void drawEnemies();

/**
 * getEnemyBlockers - Coleta retângulos de colisão de todos os inimigos
 * @param outBlockers: Array onde armazenar os retângulos
 * @param outCount: Ponteiro para receber a quantidade
 * 
 * Retorna array de retângulos para colisão com jugadores/objetos.
 */
void getEnemyBlockers(Rectangle* outBlockers, int* outCount);

/**
 * unloadEnemyManager - Libera recursos do gerenciador de inimigos
 * 
 * Descarrega texturas e libera memória de todos os inimigos.
 */
void unloadEnemyManager();

/* =============================================================================
 * FUNÇÕES DE COMBATE
 * ============================================================================= */

/**
 * startCombat - Inicia um combate com um inimigo
 * @param playerPos: Posição do jogador
 * @param combatDistance: Distância de detecção de combate
 * 
 * Detecta proximidade com inimigos e inicia combate se aplicável.
 */
void startCombat(Vector2 playerPos, float combatDistance);

/**
 * initCombat - Inicializa estrutura de combate
 * 
 * Prepara sistema para gerenciar batalhas.
 */
void initCombat();

/**
 * getCombatState - Obtém o estado atual de combate
 * @return: 1 se em combate, 0 caso contrário
 */
int getCombatState();

/**
 * getEnemiesInCombat - Obtém inimigos em combate
 * @return: Array de ponteiros para inimigos em combate
 * 
 * Retorna lista de inimigos participando da batalha atual.
 */
Enemy** getEnemiesInCombat();

/**
 * getEnemyCombatCount - Obtém quantidade de inimigos em combate
 * @return: Número de inimigos em combate
 */
int getEnemyCombatCount();

/**
 * endCombat - Finaliza o combate
 * 
 * Remove inimigos derrotados e volta para exploração.
 */
void endCombat();

#endif /* GAME_H */
