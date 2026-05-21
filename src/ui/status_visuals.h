#ifndef STATUS_VISUALS_H
#define STATUS_VISUALS_H

#include "raylib.h"
#include "../entities/status_condition.h"

/* Debug mode: renderiza retângulos e contadores de status */
// Ative para diagnosticar chamadas de render e flags de status (temporário)
// Debug desativado por padrão; habilitar apenas ao depurar visuais.
#define STATUS_VIS_DEBUG 0

/* Constantes de renderização - AJUSTÁVEIS */
#define STATUS_VISUAL_PASS_COUNT        8           /* Passes para aura */
#define STATUS_VISUAL_AURA_ALPHA        160         /* Alpha base (~0.63) */
#define STATUS_VISUAL_CYCLE_PERIOD      0.8f        /* Segundos para ciclar cores */
#define STATUS_VISUAL_BUFF_SPARK_RADIUS 2           /* Raio dos brilhos */
#define STATUS_VISUAL_BUFF_PULSE_SPEED  3.0f        /* Velocidade da pulsação */
#define STATUS_VISUAL_BUFF_COUNT_MAX    4           /* Max posições de buff */


void renderStatusAura(
    Texture2D sprite,
    Rectangle srcRect,
    Vector2 spritePos,
    Vector2 origin,
    float scale,
    StatusList* statusList
);

void renderStatusBuffs(
    StatusList* statusList,
    Vector2 spritePos,
    Vector2 spriteSize
);

/* Wrapper que chama ambas */
void renderStatusVisuals(
    Texture2D sprite,
    Rectangle srcRect,
    Vector2 spritePos,
    Vector2 origin,
    float scale,
    StatusList* statusList
);

#endif
