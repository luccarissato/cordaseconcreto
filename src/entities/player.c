#include "player.h"
#include "../ui/status_visuals.h"
#include <string.h>
#include <stdio.h>
#include <math.h>

#define SPEED 5.0f
#define RUN_SPEED_MULTIPLIER 1.3f
#define ANIM_SPEED 0.2f

static float playerWorldScale = 1.0f;

#define PLAYER2_FRONT_IDLE_PATH "assets/personagens/p2_frente_placeholder.png"
#define PLAYER2_BACK_PATH "assets/personagens/MANGUEBEAT_COSTAS.png"
#define PLAYER2_SIDE_PATH "assets/personagens/MANGUEBEAT_DIREITA.png"
#define PLAYER2_SIDE_WALK_PATH "assets/personagens/MANGUEBEAT_ESQUERDA.png"
#define PLAYER2_FRONT_WALK_PATH "assets/personagens/MANGUEBEAT_FRENTE.png"
#define PLAYER2_FRONT_WALK_FRAME_COUNT 5
#define PLAYER2_FRONT_WALK_FRAME_WIDTH 184
#define PLAYER2_FRONT_WALK_FRAME_HEIGHT 184
#define PLAYER2_WALK_MOVEMENT_FRAME_COUNT 4
#define PLAYER2_WALK_IDLE_FRAME_INDEX 4

static void initDirectionalPlayerAnimations(
    Player* p,
    const char* frontIdlePath,
    const char* backPath,
    const char* sidePath,
    const char* sideWalkPath,
    const char* walkDownPath,
    const char* walkUpPath,
    const char* walkRightPath,
    const char* walkLeftPath
) {
    p->front = LoadTexture(frontIdlePath);
    p->back = LoadTexture(backPath);
    p->side = LoadTexture(sidePath);
    p->side_walk = LoadTexture(sideWalkPath);

    initSpriteSheetAnimation(&p->walkDown, walkDownPath, PLAYER2_FRONT_WALK_FRAME_COUNT, PLAYER2_FRONT_WALK_FRAME_WIDTH, PLAYER2_FRONT_WALK_FRAME_HEIGHT, 0, PLAYER2_WALK_MOVEMENT_FRAME_COUNT, PLAYER2_WALK_IDLE_FRAME_INDEX, 0.15f);
    initSpriteSheetAnimation(&p->walkUp, walkUpPath, PLAYER2_FRONT_WALK_FRAME_COUNT, PLAYER2_FRONT_WALK_FRAME_WIDTH, PLAYER2_FRONT_WALK_FRAME_HEIGHT, 0, PLAYER2_WALK_MOVEMENT_FRAME_COUNT, PLAYER2_WALK_IDLE_FRAME_INDEX, 0.15f);
    initSpriteSheetAnimation(&p->walkRight, walkRightPath, PLAYER2_FRONT_WALK_FRAME_COUNT, PLAYER2_FRONT_WALK_FRAME_WIDTH, PLAYER2_FRONT_WALK_FRAME_HEIGHT, 0, PLAYER2_WALK_MOVEMENT_FRAME_COUNT, PLAYER2_WALK_IDLE_FRAME_INDEX, 0.15f);
    initSpriteSheetAnimation(&p->walkLeft, walkLeftPath, PLAYER2_FRONT_WALK_FRAME_COUNT, PLAYER2_FRONT_WALK_FRAME_WIDTH, PLAYER2_FRONT_WALK_FRAME_HEIGHT, 1, PLAYER2_WALK_MOVEMENT_FRAME_COUNT, 0, 0.15f);

    resetSpriteSheetAnimation(&p->walkDown);
    resetSpriteSheetAnimation(&p->walkUp);
    resetSpriteSheetAnimation(&p->walkRight);
    resetSpriteSheetAnimation(&p->walkLeft);
}

static void initSpriteSheetAnimationFromPath(SpriteSheetAnimation* animation, const char* texturePath, int frameCount, int frameWidth, int frameHeight, int movementFrameCount, int idleFrameIndex, float frameDuration) {
    if (animation == NULL) return;

    animation->texture = LoadTexture(texturePath);
    animation->frameCount = frameCount;
    animation->frameWidth = frameWidth;
    animation->frameHeight = frameHeight;
    animation->movementStartIndex = 0;
    animation->movementFrameCount = movementFrameCount;
    animation->idleFrameIndex = idleFrameIndex;
    animation->currentFrame = 0;
    animation->frameTimer = 0.0f;
    animation->frameDuration = frameDuration;
}

void initSpriteSheetAnimation(SpriteSheetAnimation* animation, const char* texturePath, int frameCount, int frameWidth, int frameHeight, int movementStartIndex, int movementFrameCount, int idleFrameIndex, float frameDuration) {
    initSpriteSheetAnimationFromPath(animation, texturePath, frameCount, frameWidth, frameHeight, movementFrameCount, idleFrameIndex, frameDuration);
    animation->movementStartIndex = movementStartIndex;
}

void setPlayerWorldScale(float scale) {
    playerWorldScale = (scale > 0.0f) ? scale : 1.0f;
}

float getPlayerWorldScale(void) {
    return playerWorldScale;
}

static Rectangle getScaledPlayerColliderRect(Player* p) {
    if (playerWorldScale != 1.0f) {
        float scaledSpriteSize = 184.0f * playerWorldScale;
        return (Rectangle){
            p->position.x + (50.0f * playerWorldScale),
            p->position.y,
            scaledSpriteSize,
            scaledSpriteSize
        };
    }

    return (Rectangle){
        p->position.x + (p->collider.offset.x * playerWorldScale),
        p->position.y + (p->collider.offset.y * playerWorldScale),
        p->collider.size.x * playerWorldScale,
        p->collider.size.y * playerWorldScale
    };
}

void unloadSpriteSheetAnimation(SpriteSheetAnimation* animation) {
    if (animation == NULL) return;

    if (animation->texture.id != 0) {
        UnloadTexture(animation->texture);
    }

    animation->texture = (Texture2D){0};
    animation->frameCount = 0;
    animation->frameWidth = 0;
    animation->frameHeight = 0;
    animation->movementStartIndex = 0;
    animation->movementFrameCount = 0;
    animation->idleFrameIndex = 0;
    animation->currentFrame = 0;
    animation->frameTimer = 0.0f;
    animation->frameDuration = 0.0f;
}

void resetSpriteSheetAnimation(SpriteSheetAnimation* animation) {
    if (animation == NULL) return;

    if (animation->idleFrameIndex >= 0 && animation->idleFrameIndex < animation->frameCount) {
        animation->currentFrame = animation->idleFrameIndex;
    } else {
        animation->currentFrame = 0;
    }
    animation->frameTimer = 0.0f;
}

void updateSpriteSheetAnimation(SpriteSheetAnimation* animation, int isMoving) {
    if (animation == NULL) return;

    if (animation->texture.id == 0 || animation->frameCount <= 0 || animation->frameDuration <= 0.0f) {
        return;
    }

    if (!isMoving) {
        if (animation->idleFrameIndex >= 0 && animation->idleFrameIndex < animation->frameCount) {
            animation->currentFrame = animation->idleFrameIndex;
        } else {
            animation->currentFrame = 0;
        }
        animation->frameTimer = 0.0f;
        return;
    }

    int movementStartIndex = animation->movementStartIndex;
    int movementFrameCount = animation->movementFrameCount;
    if (movementStartIndex < 0 || movementStartIndex >= animation->frameCount) {
        movementStartIndex = 0;
    }
    if (movementFrameCount <= 0 || movementStartIndex + movementFrameCount > animation->frameCount) {
        movementFrameCount = animation->frameCount - movementStartIndex;
    }

    if (movementFrameCount <= 0) {
        animation->currentFrame = animation->idleFrameIndex;
        animation->frameTimer = 0.0f;
        return;
    }

    if (animation->currentFrame < movementStartIndex || animation->currentFrame >= movementStartIndex + movementFrameCount) {
        animation->currentFrame = movementStartIndex;
    }

    animation->frameTimer += GetFrameTime();

    while (animation->frameTimer >= animation->frameDuration) {
        animation->currentFrame++;
        if (animation->currentFrame >= movementStartIndex + movementFrameCount) {
            animation->currentFrame = movementStartIndex;
        }
        animation->frameTimer -= animation->frameDuration;
    }
}

Rectangle getSpriteSheetFrameRect(const SpriteSheetAnimation* animation) {
    if (animation == NULL || animation->texture.id == 0 || animation->frameCount <= 0 || animation->frameWidth <= 0 || animation->frameHeight <= 0) {
        return (Rectangle){0, 0, 0, 0};
    }

    int safeFrameIndex = animation->currentFrame;
    if (safeFrameIndex < 0 || safeFrameIndex >= animation->frameCount) {
        safeFrameIndex = animation->idleFrameIndex;
        if (safeFrameIndex < 0 || safeFrameIndex >= animation->frameCount) {
            safeFrameIndex = 0;
        }
    }

    return (Rectangle){
        (float)(safeFrameIndex * animation->frameWidth),
        0.0f,
        (float)animation->frameWidth,
        (float)animation->frameHeight
    };
}

void initPlayer(Player* p, const char* prefix, Vector2 startPos, char* name) {
    strcpy(p->name, name);
    p->position = startPos;
    p->direction = DIR_DOWN;
    p->walkDown = (SpriteSheetAnimation){0};
    p->walkUp = (SpriteSheetAnimation){0};
    p->walkRight = (SpriteSheetAnimation){0};
    p->walkLeft = (SpriteSheetAnimation){0};

    if (strcmp(prefix, "p1") == 0) {
        initDirectionalPlayerAnimations(
            p,
            "assets/personagens/MARACATU_FRENTE.png",
            "assets/personagens/MARACATU_COSTAS.png",
            "assets/personagens/MARACATU_DIREITA.png",
            "assets/personagens/MARACATU_ESQUERDA.png",
            "assets/personagens/MARACATU_FRENTE.png",
            "assets/personagens/MARACATU_COSTAS.png",
            "assets/personagens/MARACATU_DIREITA.png",
            "assets/personagens/MARACATU_ESQUERDA.png"
        );
    } else if (strcmp(prefix, "p2") == 0) {
        initDirectionalPlayerAnimations(
            p,
            PLAYER2_FRONT_IDLE_PATH,
            PLAYER2_BACK_PATH,
            PLAYER2_SIDE_PATH,
            PLAYER2_SIDE_WALK_PATH,
            PLAYER2_FRONT_WALK_PATH,
            PLAYER2_BACK_PATH,
            PLAYER2_SIDE_PATH,
            PLAYER2_SIDE_WALK_PATH
        );
    } else if (strcmp(prefix, "p3") == 0) {
        initDirectionalPlayerAnimations(
            p,
            "assets/personagens/CIRANDEIRA_FRENTE.png",
            "assets/personagens/CIRANDEIRA_COSTAS.png",
            "assets/personagens/CIRANDEIRA_DIREITA.png",
            "assets/personagens/CIRANDEIRA_ESQUERDA.png",
            "assets/personagens/CIRANDEIRA_FRENTE.png",
            "assets/personagens/CIRANDEIRA_COSTAS.png",
            "assets/personagens/CIRANDEIRA_DIREITA.png",
            "assets/personagens/CIRANDEIRA_ESQUERDA.png"
        );
    } else if (strcmp(prefix, "p4") == 0) {
        initDirectionalPlayerAnimations(
            p,
            "assets/personagens/COCODERODA_FRENTE.png",
            "assets/personagens/COCODERODA_COSTAS.png",
            "assets/personagens/COCODERODA_DIREITA.png",
            "assets/personagens/COCODERODA_ESQUERDA.png",
            "assets/personagens/COCODERODA_FRENTE.png",
            "assets/personagens/COCODERODA_COSTAS.png",
            "assets/personagens/COCODERODA_DIREITA.png",
            "assets/personagens/COCODERODA_ESQUERDA.png"
        );
    } else {
        char path[128];

        /* Carrega as texturas do personagem */
        snprintf(path, sizeof(path), "assets/personagens/%s_frente_placeholder.png", prefix);
        p->front = LoadTexture(path);

        snprintf(path, sizeof(path), "assets/personagens/%s_costas_placeholder.png", prefix);
        p->back = LoadTexture(path);

        snprintf(path, sizeof(path), "assets/personagens/%s_lado_placeholder.png", prefix);
        p->side = LoadTexture(path);

        snprintf(path, sizeof(path), "assets/personagens/%s_lado_andando_placeholder.png", prefix);
        p->side_walk = LoadTexture(path);
    }

    /* Configura o colisor */
    p->collider.offset = (Vector2){ 75.0f, 0.0f };
    p->collider.size = (Vector2){ 150.0f, 200.0f };

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
    /* Preserve existing level if already set (avoid resetting on world reloads) */
    if (p->level == 0) {
        p->level = 1;
    }
    p->characterID = 0;  /* Será alterado ao carregar o personagem específico */

    /* Defesa desativada por padrão */
    p->defenseGuardActive = 0;
    p->defenseDamageReductionPending = 0;
    p->extraTurnsPending = 0;
}

void updatePlayerAnimation(Player* p, int isMoving) {
    updateSpriteSheetAnimation(&p->walkDown, isMoving && p->direction == DIR_DOWN);
    updateSpriteSheetAnimation(&p->walkUp, isMoving && p->direction == DIR_UP);
    updateSpriteSheetAnimation(&p->walkRight, isMoving && p->direction == DIR_RIGHT);
    updateSpriteSheetAnimation(&p->walkLeft, isMoving && p->direction == DIR_LEFT);

    if (isMoving && (p->direction == DIR_LEFT || p->direction == DIR_RIGHT)) {
        p->animTimer += GetFrameTime();

        if (p->animTimer >= ANIM_SPEED) {
            p->animFrame = (p->animFrame + 1) % 2;
            p->animTimer = 0.0f;
        }
    } else {
        p->animFrame = 0;
        p->animTimer = 0.0f;
    }
}

void updatePlayer(Player* p, const Rectangle* blockers, int blockerCount) {
    Vector2 move = {0};
    float currentSpeed = SPEED;

    if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) {
        currentSpeed *= RUN_SPEED_MULTIPLIER;
    }

    if (IsKeyDown(KEY_UP)) {
        move.y -= currentSpeed;
        p->direction = DIR_UP;
    }
    if (IsKeyDown(KEY_DOWN)) {
        move.y += currentSpeed;
        p->direction = DIR_DOWN;
    }
    if (IsKeyDown(KEY_RIGHT)) {
        move.x += currentSpeed;
        p->direction = DIR_RIGHT;
    }
    if (IsKeyDown(KEY_LEFT)) {
        move.x -= currentSpeed;
        p->direction = DIR_LEFT;
    }

    Rectangle playerCollider = getScaledPlayerColliderRect(p);
    Vector2 resolvedMove = resolveMovement(playerCollider, move, blockers, blockerCount);

    p->position.x += resolvedMove.x;
    p->position.y += resolvedMove.y;

    int isMoving = (move.x != 0 || move.y != 0);

    updatePlayerAnimation(p, isMoving);
}

void drawPlayer(Player* p) {
    Texture2D tex;
    Rectangle src = {0, 0, 0, 0};

    switch (p->direction) {
        case DIR_UP: 
            if (p->walkUp.texture.id != 0) {
                tex = p->walkUp.texture;
                src = getSpriteSheetFrameRect(&p->walkUp);
            } else {
                tex = p->back; 
                src = (Rectangle){0, 0, (float)tex.width, (float)tex.height};
            }
            break;

        case DIR_DOWN: 
            if (p->walkDown.texture.id != 0) {
                tex = p->walkDown.texture;
                src = getSpriteSheetFrameRect(&p->walkDown);
            } else {
                tex = p->front;
                src = (Rectangle){0, 0, (float)tex.width, (float)tex.height};
            }
            break;

        case DIR_RIGHT:
            if (p->walkRight.texture.id != 0) {
                tex = p->walkRight.texture;
                src = getSpriteSheetFrameRect(&p->walkRight);
            } else {
                tex = (p->animFrame % 2 == 0) ? p->side : p->side_walk;
                src = (Rectangle){0, 0, (float)tex.width, (float)tex.height};
            }
            break;

        case DIR_LEFT:
            if (p->walkLeft.texture.id != 0) {
                tex = p->walkLeft.texture;
                src = getSpriteSheetFrameRect(&p->walkLeft);
            } else {
                tex = (p->animFrame % 2 == 0) ? p->side_walk : p->side;
                src = (Rectangle){0, 0, (float)tex.width, (float)tex.height};
                src.width = -tex.width;
            }
            break;
    }

    Vector2 drawPos = { p->position.x + (50.0f * playerWorldScale), p->position.y };
    renderStatusAura(tex, src, drawPos, (Vector2){0, 0}, playerWorldScale, &p->statusList);

    Rectangle dest = {
        drawPos.x,
        drawPos.y,
        fabsf(src.width) * playerWorldScale,
        src.height * playerWorldScale
    };
    DrawTexturePro(tex, src, dest, (Vector2){0, 0}, 0.0f, WHITE);

    Vector2 spriteSize = {dest.width, dest.height};
    renderStatusBuffs(&p->statusList, drawPos, spriteSize);

}

/**
 * unloadPlayer - Libera os recursos do personagem
 * 
 * Descarrega texturas e libera memória da lista de status.
 */
void unloadPlayer(Player* p) {
    /* Descarrega as texturas */
    if (p->front.id != 0) UnloadTexture(p->front);
    if (p->back.id != 0) UnloadTexture(p->back);
    if (p->side.id != 0) UnloadTexture(p->side);
    if (p->side_walk.id != 0) UnloadTexture(p->side_walk);
    unloadSpriteSheetAnimation(&p->walkDown);
    unloadSpriteSheetAnimation(&p->walkUp);
    unloadSpriteSheetAnimation(&p->walkRight);
    unloadSpriteSheetAnimation(&p->walkLeft);
    
    /* === NOVO: Libera a lista de condições de status === */
    freeStatusList(&p->statusList);
}

void playerLevelUp(Player* player) {
    if (player == NULL) return;
    
    if (player->level >= MAX_LEVEL) return;
    
    player->level++;
    
    applyLevelGrowth(&player->stats);
    
    calculateStats(&player->stats);
    
    player->stats.currentHP = player->stats.maxHP;
    player->stats.currentMana = player->stats.maxMana;
    player->isAlive = 1;
}

