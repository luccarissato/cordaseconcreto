#include "color_puzzle.h"
#include "../core/collision.h"
#include "../core/game.h"
#include "raylib.h"
#include <stdlib.h>

static float rectOverlapArea(Rectangle a, Rectangle b) {
    float left = (a.x > b.x) ? a.x : b.x;
    float right = ((a.x + a.width) < (b.x + b.width)) ? (a.x + a.width) : (b.x + b.width);
    float top = (a.y > b.y) ? a.y : b.y;
    float bottom = ((a.y + a.height) < (b.y + b.height)) ? (a.y + a.height) : (b.y + b.height);

    if (right <= left || bottom <= top) {
        return 0.0f;
    }

    return (right - left) * (bottom - top);
}

static ColorPuzzle* getPuzzleData(Interactable* self) {
    if (self == NULL) {
        return NULL;
    }

    return (ColorPuzzle*)self->data;
}

static void shuffleSlots(ColorPuzzle* puzzle) {
    if (puzzle == NULL || puzzle->tileCount <= 1) {
        return;
    }

    for (int i = puzzle->tileCount - 1; i > 0; i--) {
        int swapIndex = GetRandomValue(0, i);
        int temp = puzzle->slotOrder[i];
        puzzle->slotOrder[i] = puzzle->slotOrder[swapIndex];
        puzzle->slotOrder[swapIndex] = temp;
    }
}

static Rectangle getTileRect(const ColorPuzzle* puzzle, int tileIndex) {
    return (Rectangle){
        puzzle->base.position.x + (puzzle->tileWidth * tileIndex),
        puzzle->base.position.y,
        (float)puzzle->tileWidth,
        (float)puzzle->tileHeight
    };
}

static int getLeaderTileIndex(const ColorPuzzle* puzzle, Rectangle leaderRect) {
    int bestIndex = -1;
    float bestArea = 0.0f;

    for (int i = 0; i < puzzle->tileCount; i++) {
        Rectangle tileRect = getTileRect(puzzle, i);
        float area = rectOverlapArea(leaderRect, tileRect);
        if (area > bestArea) {
            bestArea = area;
            bestIndex = i;
        }
    }

    return bestIndex;
}

static void damagePartyAllMembers(int damage) {
    int partyCount = 0;
    Player* partyMembers = getPartyMembers(&partyCount);

    if (partyMembers == NULL || partyCount <= 0) {
        return;
    }

    for (int i = 0; i < partyCount; i++) {
        Player* member = &partyMembers[i];
        if (member->stats.currentHP > 0) {
            member->stats.currentHP -= damage;
            if (member->stats.currentHP < 0) {
                member->stats.currentHP = 0;
            }
            member->isAlive = (member->stats.currentHP > 0);
        }
    }
}

void colorPuzzle_init_from_assets(
    ColorPuzzle* puzzle,
    Vector2 position,
    const char* bluePath,
    const char* greenPath,
    const char* yellowPath,
    const char* redPath,
    float swapInterval,
    int damage
) {
    if (puzzle == NULL) {
        return;
    }

    puzzle->tileTextures[0] = LoadTexture(bluePath);
    puzzle->tileTextures[1] = LoadTexture(greenPath);
    puzzle->tileTextures[2] = LoadTexture(yellowPath);
    puzzle->tileTextures[3] = LoadTexture(redPath);
    puzzle->tileCount = 4;
    puzzle->slotOrder[0] = 0;
    puzzle->slotOrder[1] = 1;
    puzzle->slotOrder[2] = 2;
    puzzle->slotOrder[3] = 3;
    puzzle->redColorIndex = 3;
    puzzle->swapInterval = (swapInterval > 0.0f) ? swapInterval : 2.0f;
    puzzle->swapTimer = 0.0f;
    puzzle->damage = damage;
    puzzle->onlyLeaderTriggers = 1;
    puzzle->leaderTileIndex = -1;
    puzzle->lastRedTileIndex = -1;
    puzzle->tileWidth = (int)puzzle->tileTextures[0].width;
    puzzle->tileHeight = (int)puzzle->tileTextures[0].height;
    shuffleSlots(puzzle);

    puzzle->base.type = INTERACTABLE_COLOR_PUZZLE;
    puzzle->base.position = position;
    puzzle->base.collider.offset = (Vector2){0.0f, 0.0f};
    puzzle->base.collider.size = (Vector2){(float)puzzle->tileWidth, (float)puzzle->tileHeight};
    puzzle->base.hasInteracted = 0;
    puzzle->base.interactionDistance = 0.0f;
    puzzle->base.sprite = puzzle->tileTextures[puzzle->slotOrder[0]];
    puzzle->base.on_interact = NULL;
    puzzle->base.on_update = colorPuzzle_on_update;
    puzzle->base.on_draw = colorPuzzle_on_draw;
    puzzle->base.on_unload = colorPuzzle_on_unload;
    puzzle->base.data = puzzle;
}

void colorPuzzle_on_update(Interactable* self, Vector2 playerPos) {
    ColorPuzzle* puzzle = getPuzzleData(self);
    if (puzzle == NULL) {
        return;
    }

    int partyCount = 0;
    Player* partyMembers = getPartyMembers(&partyCount);
    if (partyMembers == NULL || partyCount <= 0) {
        return;
    }

    puzzle->swapTimer += GetFrameTime();
    while (puzzle->swapTimer >= puzzle->swapInterval) {
        puzzle->swapTimer -= puzzle->swapInterval;
        shuffleSlots(puzzle);
    }

    Player* leader = &partyMembers[0];
    if (puzzle->onlyLeaderTriggers && leader == NULL) {
        return;
    }

    Rectangle leaderRect = getColliderRect(playerPos, leader->collider);
    int currentTileIndex = getLeaderTileIndex(puzzle, leaderRect);
    int activeRedTileIndex = -1;

    for (int i = 0; i < puzzle->tileCount; i++) {
        if (puzzle->slotOrder[i] == puzzle->redColorIndex) {
            activeRedTileIndex = i;
            break;
        }
    }

    int isLeaderOnActiveRed = (currentTileIndex >= 0 && currentTileIndex == activeRedTileIndex);

    if (currentTileIndex < 0) {
        if (puzzle->leaderTileIndex >= 0) {
            puzzle->leaderTileIndex = -1;
        }

        puzzle->lastRedTileIndex = activeRedTileIndex;
    } else {
        int leaderChangedTile = (currentTileIndex != puzzle->leaderTileIndex);
        int redChangedUnderLeader = (isLeaderOnActiveRed && activeRedTileIndex != puzzle->lastRedTileIndex);

        if ((leaderChangedTile && isLeaderOnActiveRed) || redChangedUnderLeader) {
            damagePartyAllMembers(puzzle->damage);
        }

        puzzle->leaderTileIndex = currentTileIndex;
    }

    puzzle->lastRedTileIndex = activeRedTileIndex;

    self->sprite = puzzle->tileTextures[puzzle->slotOrder[0]];
}

void colorPuzzle_on_draw(Interactable* self) {
    ColorPuzzle* puzzle = getPuzzleData(self);
    if (puzzle == NULL) {
        return;
    }

    for (int i = 0; i < puzzle->tileCount; i++) {
        int colorIndex = puzzle->slotOrder[i];
        Texture2D tex = puzzle->tileTextures[colorIndex];
        if (tex.id == 0) {
            continue;
        }

        Vector2 tilePos = {
            puzzle->base.position.x + (float)(puzzle->tileWidth * i),
            puzzle->base.position.y
        };
        DrawTexture(tex, (int)tilePos.x, (int)tilePos.y, WHITE);
    }

    int activeRedTileIndex = -1;
    for (int i = 0; i < puzzle->tileCount; i++) {
        if (puzzle->slotOrder[i] == puzzle->redColorIndex) {
            activeRedTileIndex = i;
            break;
        }
    }

    if (activeRedTileIndex >= 0) {
        Rectangle redRect = getTileRect(puzzle, activeRedTileIndex);
        DrawRectangleLinesEx(redRect, 2.0f, RED);
    }
}

void colorPuzzle_on_unload(Interactable* self) {
    ColorPuzzle* puzzle = getPuzzleData(self);
    if (puzzle == NULL) {
        return;
    }

    for (int i = 0; i < puzzle->tileCount; i++) {
        if (puzzle->tileTextures[i].id != 0) {
            UnloadTexture(puzzle->tileTextures[i]);
        }
    }

    free(puzzle);
}

Interactable createColorPuzzle(
    Vector2 position,
    const char* bluePath,
    const char* greenPath,
    const char* yellowPath,
    const char* redPath,
    float swapInterval,
    int damage
) {
    ColorPuzzle* puzzle = (ColorPuzzle*)calloc(1, sizeof(ColorPuzzle));
    if (puzzle == NULL) {
        Interactable empty = {0};
        return empty;
    }

    colorPuzzle_init_from_assets(
        puzzle,
        position,
        bluePath,
        greenPath,
        yellowPath,
        redPath,
        swapInterval,
        damage
    );

    return puzzle->base;
}