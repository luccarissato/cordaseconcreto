#include "player.h"
#include <string.h>
#include <stdio.h>

#define SPEED 5.0f
#define ANIM_SPEED 0.2f

void initPlayer(Player* p, const char* prefix, Vector2 startPos, char* name) {
    strcpy(p->name, name);
    p->position = startPos;
    p->direction = DIR_DOWN;

    char path[128];

    /* Carrega as texturas do personagem */
    sprintf(path, "assets/personagens/%s_frente_placeholder.png", prefix);
    p->front = LoadTexture(path);

    sprintf(path, "assets/personagens/%s_costas_placeholder.png", prefix);
    p->back = LoadTexture(path);

    sprintf(path, "assets/personagens/%s_lado_placeholder.png", prefix);
    p->side = LoadTexture(path);

    sprintf(path, "assets/personagens/%s_lado_andando_placeholder.png", prefix);
    p->side_walk = LoadTexture(path);

    /* Configura o colisor */
    p->collider.offset = (Vector2){ 75.0f, 0.0f };
    p->collider.size = (Vector2){ 150.0f, 300.0f };

    /* Inicializa animação */
    p->animFrame = 0;
    p->animTimer = 0.0f;

    /* Calcula os stats derivados (maxHP, maxMana) */
    calculateStats(&p->stats);
    
    /* === NOVO: Inicializa o sistema de condições de status === */
    initStatusList(&p->statusList);
    
    /* Personagem começa vivo */
    p->isAlive = 1;
    
    /* Inicializa sistema de habilidades === */
    p->level = 1;
    p->characterID = 0;  /* Será alterado ao carregar o personagem específico */
}

void updatePlayerAnimation(Player* p, int isMoving) {
    // só anima se estiver andando E na horizontal
    if (isMoving && (p->direction == DIR_LEFT || p->direction == DIR_RIGHT)) {
        p->animTimer += GetFrameTime();

        if (p->animTimer >= ANIM_SPEED) {
            p->animFrame = !p->animFrame;
            p->animTimer = 0;
        }
    } else {
        p->animFrame = 0;
        p->animTimer = 0;
    }
}

void updatePlayer(Player* p, const Rectangle* blockers, int blockerCount) {
    Vector2 move = {0};

    if (IsKeyDown(KEY_UP)) {
        move.y -= SPEED;
        p->direction = DIR_UP;
    }
    if (IsKeyDown(KEY_DOWN)) {
        move.y += SPEED;
        p->direction = DIR_DOWN;
    }
    if (IsKeyDown(KEY_RIGHT)) {
        move.x += SPEED;
        p->direction = DIR_RIGHT;
    }
    if (IsKeyDown(KEY_LEFT)) {
        move.x -= SPEED;
        p->direction = DIR_LEFT;
    }

    Rectangle playerCollider = getColliderRect(p->position, p->collider);
    Vector2 resolvedMove = resolveMovement(playerCollider, move, blockers, blockerCount);

    p->position.x += resolvedMove.x;
    p->position.y += resolvedMove.y;

    int isMoving = (resolvedMove.x != 0 || resolvedMove.y != 0);

    updatePlayerAnimation(p, isMoving);
}

void drawPlayer(Player* p) {
    Texture2D tex;

    switch (p->direction) {
        case DIR_UP: 
            tex = p->back; 
            break;

        case DIR_DOWN: 
            tex = p->front; 
            break;

        case DIR_LEFT:
        case DIR_RIGHT:
            tex = (p->animFrame == 0) ? p->side : p->side_walk;
            break;
    }

    Rectangle src = {0, 0, tex.width, tex.height};

    // espelhamento horizontal
    if (p->direction == DIR_LEFT) {
        src.width = -tex.width;
    }

    DrawTextureRec(tex, src, p->position, WHITE);

    // colision debug
    Rectangle colliderRect = getColliderRect(p->position, p->collider);
    DrawRectangleLinesEx(colliderRect, 2.0f, GREEN);
}

/**
 * unloadPlayer - Libera os recursos do personagem
 * 
 * Descarrega texturas e libera memória da lista de status.
 */
void unloadPlayer(Player* p) {
    /* Descarrega as texturas */
    UnloadTexture(p->front);
    UnloadTexture(p->back);
    UnloadTexture(p->side);
    UnloadTexture(p->side_walk);
    
    /* === NOVO: Libera a lista de condições de status === */
    freeStatusList(&p->statusList);
}

void playerLevelUp(Player* player) {
    if (player == NULL) return;
    
    /* Verifica se já atingiu o nível máximo */
    if (player->level >= MAX_LEVEL) return;
    
    /* 1. Incrementa nível */
    player->level++;
    
    /* 2. Aplica crescimento de stats base */
    applyLevelGrowth(&player->stats);
    
    /* 3. Recalcula derivados (maxHP, maxMana) */
    calculateStats(&player->stats);
    
    /* 4. Restaura HP/Mana (vitória justifica recuperação total) */
    player->stats.currentHP = player->stats.maxHP;
    player->stats.currentMana = player->stats.maxMana;
}

