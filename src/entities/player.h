#ifndef PLAYER_H
#define PLAYER_H

#include "raylib.h"
#include "../core/collision.h"
#include "stats.h"

typedef enum {
    DIR_UP,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT
} Direction;

typedef struct Player {
    char name[32];
    Vector2 position;
    Direction direction;

    Texture2D front;
    Texture2D back;
    Texture2D side;
    Texture2D side_walk;

    Collider collider;

    int animFrame;
    float animTimer;

    Stats stats;
} Player;

void initPlayer(Player* p, const char* prefix, Vector2 startPos, char* name);
void updatePlayer(Player* p, const Rectangle* blockers, int blockerCount);
void drawPlayer(Player* p);
void unloadPlayer(Player* p);
void updatePlayerAnimation(Player* p, int isMoving);

#endif