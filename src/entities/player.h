#ifndef PLAYER_H
#define PLAYER_H

#include "raylib.h"

typedef enum {
    DIR_UP,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT
} Direction;

typedef struct Player {
    Vector2 position;
    Direction direction;

    Texture2D front;
    Texture2D back;
    Texture2D side;
    Texture2D side_walk;

    int animFrame;
    float animTimer;
} Player;

void initPlayer(Player* p, const char* prefix, Vector2 startPos);
void updatePlayer(Player* p);
void drawPlayer(Player* p);
void unloadPlayer(Player* p);
void updatePlayerAnimation(Player* p, int isMoving);

#endif