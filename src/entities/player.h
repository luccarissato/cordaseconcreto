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

/* -----------------------------------------------------------------------------
 * ENUMERAÇÃO: Direction
 * -----------------------------------------------------------------------------
 * Define as direções possíveis de movimento do jogador.
 * -------------------------------------------------------------------------- */
typedef enum {
    DIR_UP,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT
} Direction;

/* -----------------------------------------------------------------------------
 * ESTRUTURA: Player
 * -----------------------------------------------------------------------------
 * Representa um personagem jogável no jogo.
 * Contém informações de posição, sprites, colisão, animação e status.
 * 
 * Campos:
 *   - name: Nome do personagem
 *   - position: Posição atual no mundo
 *   - direction: Direção que o personagem está olhando
 *   - front/back/side/side_walk: Texturas para cada direção/estado
 *   - collider: Área de colisão do personagem
 *   - animFrame/animTimer: Controle de animação
 *   - stats: Atributos do personagem (HP, Mana, etc.)
 *   - statusList: Lista de condições de status ativas
 *   - isAlive: Flag indicando se o personagem está vivo (HP > 0)
 * -------------------------------------------------------------------------- */
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

#endif
