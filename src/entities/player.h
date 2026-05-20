/**
 * =============================================================================
 * PLAYER.H - Estrutura e Funções do Jogador
 * =============================================================================
 * 
 * Define a estrutura do jogador e suas funções de movimentação e renderização.
 * Inclui o sistema de condições de status para efeitos de combate.
 * 
 * =============================================================================
 */

#ifndef PLAYER_H
#define PLAYER_H

#include "raylib.h"
#include "../core/collision.h"
#include "stats.h"
#include "status_condition.h"

/* Limite máximo de nível */
#define MAX_LEVEL 4

typedef enum {
    DIR_UP,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT
} Direction;

typedef struct Player {
    char name[32];              /* Nome do personagem */
    Vector2 position;           /* Posição no mundo */
    Direction direction;        /* Direção atual */

    Texture2D front;            /* Sprite olhando para frente */
    Texture2D back;             /* Sprite olhando para trás */
    Texture2D side;             /* Sprite olhando para o lado (parado) */
    Texture2D side_walk;        /* Sprite olhando para o lado (andando) */

    Collider collider;          /* Área de colisão */

    int animFrame;              /* Frame atual da animação */
    float animTimer;            /* Timer para troca de frames */

    Stats stats;                /* Atributos do personagem */
    
    /* === NOVO: Sistema de Condições de Status === */
    StatusList statusList;      /* Lista de condições de status ativas */
    int isAlive;                /* 1 se HP > 0, 0 se derrotado */
    
    /* === NOVO: Sistema de Habilidades === */
    int level;                  /* Nível do personagem (1-MAX) */
    int characterID;            /* ID do tipo de personagem (0-3) */
} Player;

void initPlayer(Player* p, const char* prefix, Vector2 startPos, char* name);
void updatePlayer(Player* p, const Rectangle* blockers, int blockerCount);
void drawPlayer(Player* p);
void unloadPlayer(Player* p);
void updatePlayerAnimation(Player* p, int isMoving);

/**
 * playerLevelUp - Aplica level up ao jogador
 * 
 * Fluxo:
 * 1. Incrementa player->level
 * 2. Aplica crescimento de stats (fortitude +5, mente +5, etc)
 * 3. Recalcula HP/Mana derivados automaticamente
 * 4. Restaura HP e Mana para os novos máximos
 * 
 * IMPORTANTE: Função modular que também servirá para inimigos
 * (após adaptação para a estrutura Enemy).
 * 
 * ABORDAGEM ADOTADA PARA currentHP/currentMana:
 * - Restauração completa (currentHP/currentMana = máximos)
 * - Justificativa: Vitória em combate merece recuperação total
 */
void playerLevelUp(Player* player);

#endif
