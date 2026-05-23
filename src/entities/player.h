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

typedef struct SpriteSheetAnimation {
    Texture2D texture;
    int frameCount;
    int frameWidth;
    int frameHeight;
    int movementStartIndex;
    int movementFrameCount;
    int idleFrameIndex;
    int currentFrame;
    float frameTimer;
    float frameDuration;
} SpriteSheetAnimation;

typedef struct Player {
    char name[32];              /* Nome do personagem */
    Vector2 position;           /* Posição no mundo */
    Direction direction;        /* Direção atual */

    Texture2D front;            /* Sprite olhando para frente */
    Texture2D back;             /* Sprite olhando para trás */
    Texture2D side;             /* Sprite olhando para o lado (parado) */
    Texture2D side_walk;        /* Sprite olhando para o lado (andando) */
    SpriteSheetAnimation walkDown; /* Animação de caminhada para baixo */
    SpriteSheetAnimation walkUp;    /* Animação de caminhada para cima */
    SpriteSheetAnimation walkRight;  /* Animação de caminhada para direita */
    SpriteSheetAnimation walkLeft;   /* Animação de caminhada para esquerda */

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

    /* Estado temporário de defesa (combate) */
    int defenseGuardActive;             /* Imune a NOVOS status até o próximo turno */
    int defenseDamageReductionPending;  /* Reduz 30% do próximo ataque recebido */
    int extraTurnsPending;              /* Turnos extras pendentes de Aceleração */
} Player;

void initPlayer(Player* p, const char* prefix, Vector2 startPos, char* name);
void updatePlayer(Player* p, const Rectangle* blockers, int blockerCount);
void drawPlayer(Player* p);
void unloadPlayer(Player* p);
void updatePlayerAnimation(Player* p, int isMoving);
void initSpriteSheetAnimation(SpriteSheetAnimation* animation, const char* texturePath, int frameCount, int frameWidth, int frameHeight, int movementStartIndex, int movementFrameCount, int idleFrameIndex, float frameDuration);
void unloadSpriteSheetAnimation(SpriteSheetAnimation* animation);
void updateSpriteSheetAnimation(SpriteSheetAnimation* animation, int isMoving);
void resetSpriteSheetAnimation(SpriteSheetAnimation* animation);
Rectangle getSpriteSheetFrameRect(const SpriteSheetAnimation* animation);
void playerLevelUp(Player* player);

#endif
