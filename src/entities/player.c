#include "player.h"
#include <stdio.h>

#define SPEED 5.0f
#define ANIM_SPEED 0.2f

void initPlayer(Player* p, const char* prefix, Vector2 startPos) {
    p->position = startPos;
    p->direction = DIR_DOWN;

    char path[128];

    sprintf(path, "assets/personagens/%s_frente_placeholder.png", prefix);
    p->front = LoadTexture(path);

    sprintf(path, "assets/personagens/%s_costas_placeholder.png", prefix);
    p->back = LoadTexture(path);

    sprintf(path, "assets/personagens/%s_lado_placeholder.png", prefix);
    p->side = LoadTexture(path);

    sprintf(path, "assets/personagens/%s_lado_andando_placeholder.png", prefix);
    p->side_walk = LoadTexture(path);

    p->animFrame = 0;
    p->animTimer = 0.0f;
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

void updatePlayer(Player* p) {
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

    p->position.x += move.x;
    p->position.y += move.y;

    int isMoving = (move.x != 0 || move.y != 0);

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
}

void unloadPlayer(Player* p) {
    UnloadTexture(p->front);
    UnloadTexture(p->back);
    UnloadTexture(p->side);
    UnloadTexture(p->side_walk);
}