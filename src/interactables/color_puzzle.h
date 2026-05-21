#ifndef COLOR_PUZZLE_H
#define COLOR_PUZZLE_H

#include <stdbool.h>
#include "interactable.h"
#include "../entities/player.h"

#define COLOR_PUZZLE_MAX_TILES 8

typedef struct ColorPuzzle {
    Interactable base;
    Texture2D tileTextures[COLOR_PUZZLE_MAX_TILES];
    int tileCount;
    int slotOrder[COLOR_PUZZLE_MAX_TILES];
    int redColorIndex;
    float swapInterval;
    float swapTimer;
    int damage;
    int onlyLeaderTriggers;
    int leaderTileIndex;
    int lastRedTileIndex;
    int tileWidth;
    int tileHeight;
} ColorPuzzle;

Interactable createColorPuzzle(
    Vector2 position,
    const char* bluePath,
    const char* greenPath,
    const char* yellowPath,
    const char* redPath,
    float swapInterval,
    int damage
);

void colorPuzzle_init_from_assets(
    ColorPuzzle* puzzle,
    Vector2 position,
    const char* bluePath,
    const char* greenPath,
    const char* yellowPath,
    const char* redPath,
    float swapInterval,
    int damage
);

void colorPuzzle_on_update(Interactable* self, Vector2 playerPos);
void colorPuzzle_on_draw(Interactable* self);
void colorPuzzle_on_unload(Interactable* self);

#endif