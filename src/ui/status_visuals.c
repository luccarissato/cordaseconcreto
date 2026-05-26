#include "status_visuals.h"
#include "../entities/status_condition.h"
#include "raylib.h"
#include <math.h>
#include <stdio.h>

/* Interpola entre duas cores */
static Color colorLerp(Color a, Color b, float t) {
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;
    
    return (Color){
        (unsigned char)(a.r + (b.r - a.r) * t),
        (unsigned char)(a.g + (b.g - a.g) * t),
        (unsigned char)(a.b + (b.b - a.b) * t),
        (unsigned char)(a.a + (b.a - a.a) * t)
    };
}

 // OFFSETS PRÉ-CALCULADOS PARA AURA (CÍRCULO)
static const Vector2 AURA_OFFSETS[STATUS_VISUAL_PASS_COUNT] = {
    {3.0f, 0.0f},      // 0°
    {2.1f, 2.1f},      // 45°
    {0.0f, 3.0f},      // 90°
    {-2.1f, 2.1f},     // 135°
    {-3.0f, 0.0f},     // 180°
    {-2.1f, -2.1f},    // 225°
    {0.0f, -3.0f},     // 270°
    {2.1f, -2.1f}      // 315°
};

 // POSIÇÕES PRÉ-DETERMINADAS PARA BUFFS (SEM SOBREPOSIÇÃO)
static const Vector2 BUFF_POSITIONS[STATUS_VISUAL_BUFF_COUNT_MAX] = {
    {0.0f, -8.0f},     // Topo
    {8.0f, -4.0f},     // Topo-direita
    {8.0f, 4.0f},      // Baixo-direita
    {0.0f, 8.0f}       // Baixo
};

// MAPEMAMENTO STATUSTYPE → COLOR
static Color getStatusAuraColor(StatusType type) {
    switch (type) {
        case STATUS_POISON:         return (Color){0, 255, 0, 255};        // Verde vibrante
        case STATUS_ENSOLACAO:      return (Color){255, 165, 0, 255};      // Laranja queimado
        case STATUS_BLEED:          return (Color){200, 0, 0, 255};        // Vermelho escuro
        case STATUS_CONFUSION:      return (Color){200, 0, 255, 255};      // Roxo magenta
        case STATUS_SLOW:           return (Color){0, 200, 255, 255};      // Azul gelo
        case STATUS_WEAKEN:         return (Color){100, 100, 100, 255};    // Cinza escuro
        case STATUS_DEFENSE_DOWN:   return (Color){255, 100, 0, 255};      // Laranja intenso
        case STATUS_ENCHARCADO:     return (Color){100, 150, 255, 255};    // Azul claro
        case STATUS_ARCANE_MARK:    return (Color){186, 85, 211, 255};     // Medium Orchid
        default:                    return (Color){255, 255, 255, 0};      // Transparente
    }
}

static Color getStatusBuffColor(StatusType type) {
    switch (type) {
        case STATUS_REGEN:          return (Color){0, 255, 100, 255};      // Verde brilhante
        case STATUS_STRENGTH_UP:    return (Color){255, 200, 0, 255};      // Ouro forte
        case STATUS_DEFENSE_UP:     return (Color){65, 105, 225, 255};     // Azul royal
        case STATUS_SPEED_UP:       return (Color){200, 200, 255, 255};    // Branco-prata
        case STATUS_PROTECT:        return (Color){255, 215, 0, 255};      // Gold
        case STATUS_COUNTER:        return (Color){255, 69, 0, 255};       // Orange-Red
        case STATUS_REFLECT:        return (Color){135, 206, 250, 255};    // Light Sky Blue
        case STATUS_ASCENSAO_MAGICA: return (Color){255, 105, 180, 255};   // Hot Pink
        case STATUS_HASTE:          return (Color){255, 255, 0, 255};      // Amarelo
        default:                    return (Color){255, 255, 255, 0};      // Transparente
    }
}

// COLOR CYCLING PARA AURA (MÚLTIPLOS DEBUFFS)
static Color calculateAuraCyclingColor(StatusList* statusList) {
    if (statusList == NULL || statusList->conditions.size == 0) {
        return (Color){255, 255, 255, 0};
    }
    
    // Coleta cores de debuffs ativos
    Color debuffColors[8];
    int debuffCount = 0;
    
    ListNode* current = statusList->conditions.head;
    int iterations = statusList->conditions.size;
    
    for (int i = 0; i < iterations && current != NULL && debuffCount < 8; i++) {
        StatusCondition* cond = (StatusCondition*)current->data;
        if (cond != NULL && isDebuff(cond->type)) {
            debuffColors[debuffCount++] = getStatusAuraColor(cond->type);
        }
        current = current->next;
    }
    
    if (debuffCount == 0) return (Color){255, 255, 255, 0};
    
    // Aura única: cor fixa
    if (debuffCount == 1) {
        Color c = debuffColors[0];
        c.a = STATUS_VISUAL_AURA_ALPHA;
        return c;
    }
    
    // Múltiplos debuffs: ciclar entre cores via lerp + sin
    float t = (sinf(GetTime() / STATUS_VISUAL_CYCLE_PERIOD * 3.14159f) + 1.0f) / 2.0f;
    int idx1 = (int)(t * (debuffCount - 1));
    int idx2 = (idx1 + 1) % debuffCount;
    float blend = (t * (debuffCount - 1)) - idx1;
    
    Color result = colorLerp(debuffColors[idx1], debuffColors[idx2], blend);
    result.a = STATUS_VISUAL_AURA_ALPHA;
    return result;
}


// RENDERIZAR AURA (OUTLINE SUAVE)
void renderStatusAura(
    Texture2D sprite,
    Rectangle srcRect,
    Vector2 spritePos,
    Vector2 origin,
    float scale,
    StatusList* statusList
) {
    if (statusList == NULL || statusList->conditions.size == 0) return;
    
    int hasDebuff = 0;
    int debuffCount = 0;
    ListNode* current = statusList->conditions.head;
    int iterations = statusList->conditions.size;
    
    for (int i = 0; i < iterations && current != NULL; i++) {
        StatusCondition* cond = (StatusCondition*)current->data;
        if (cond != NULL && isDebuff(cond->type)) {
            hasDebuff = 1;
            debuffCount++;
        }
        current = current->next;
    }
    
    if (!hasDebuff) return;
    
    Color auraColor = calculateAuraCyclingColor(statusList);
    if (auraColor.a == 0) return;
    
    float spriteWidth = fabsf(srcRect.width);
    float spriteHeight = fabsf(srcRect.height);
    float maxDim = (spriteWidth > spriteHeight) ? spriteWidth : spriteHeight;
    float adaptiveOffset = maxDim * 0.035f;  // 3.5% do tamanho: visivel alem da outline preta
    if (adaptiveOffset < 5.0f) adaptiveOffset = 5.0f;    // minimo para sprites com contorno grosso
    if (adaptiveOffset > 14.0f) adaptiveOffset = 14.0f;  // evita aura excessiva em sprites grandes
    
    Vector2 adaptiveAuraOffsets[8];
    float offsetScale = adaptiveOffset / 3.0f;  // 3 era o original
    for (int i = 0; i < 8; i++) {
        adaptiveAuraOffsets[i] = (Vector2){
            AURA_OFFSETS[i].x * offsetScale,
            AURA_OFFSETS[i].y * offsetScale
        };
    }

#if STATUS_VIS_DEBUG
    /* Log discreto (a cada ~1s) para confirmar que a função é chamada */
    {
        static double lastLog = 0.0;
        double now = GetTime();
        if (now - lastLog > 1.0) {
            lastLog = now;
            fprintf(stderr, "[STATUS_VIS] aura at (%.1f,%.1f) debuffs=%d adaptiveOffset=%.1f\n",
                    spritePos.x, spritePos.y, debuffCount, adaptiveOffset);
        }
    }
#endif
    
    for (int ring = 2; ring >= 1; ring--) {
        Color ringColor = auraColor;
        ringColor.a = (unsigned char)((auraColor.a * ring) / 2);

        // Desenha N passes deslocados para criar outline suave
        for (int pass = 0; pass < STATUS_VISUAL_PASS_COUNT; pass++) {
            Vector2 offset = adaptiveAuraOffsets[pass];
            Vector2 shiftedPos = {
                spritePos.x + offset.x * scale * ring,
                spritePos.y + offset.y * scale * ring
            };
            
            Rectangle dstRect = {
                shiftedPos.x,
                shiftedPos.y,
                spriteWidth * scale,
                spriteHeight * scale
            };
            
            DrawTexturePro(sprite, srcRect, dstRect, origin, 0.0f, ringColor);
        }
    }
    
    #if STATUS_VIS_DEBUG
    // === DEBUG OVERLAY: Desenha retângulo de debug + info ===
    DrawText("AURA", (int)spritePos.x, (int)spritePos.y - 20, 16, YELLOW);
    DrawText(TextFormat("debuffs: %d", debuffCount), (int)spritePos.x, (int)spritePos.y - 5, 12, WHITE);
    DrawText(TextFormat("offset: %.1f", adaptiveOffset), (int)spritePos.x, (int)spritePos.y + 10, 10, SKYBLUE);
    #endif
}


// RENDERIZAR BUFFS (BRILHOS/SPARKS)
void renderStatusBuffs(
    StatusList* statusList,
    Vector2 spritePos,
    Vector2 spriteSize
) {
    if (statusList == NULL || statusList->conditions.size == 0) return;
    
    // Coleta buffs ativos
    Color buffColors[STATUS_VISUAL_BUFF_COUNT_MAX];
    int buffCount = 0;
    
    ListNode* current = statusList->conditions.head;
    int iterations = statusList->conditions.size;
    
    for (int i = 0; i < iterations && current != NULL && buffCount < STATUS_VISUAL_BUFF_COUNT_MAX; i++) {
        StatusCondition* cond = (StatusCondition*)current->data;
        if (cond != NULL && isBuff(cond->type)) {
            buffColors[buffCount++] = getStatusBuffColor(cond->type);
        }
        current = current->next;
    }
    
    if (buffCount == 0) return;
    
    float time = GetTime();
    
    for (int i = 0; i < buffCount; i++) {
        Vector2 buffPos = {
            spritePos.x + spriteSize.x / 2.0f + BUFF_POSITIONS[i].x,
            spritePos.y + spriteSize.y / 2.0f + BUFF_POSITIONS[i].y
        };
        
        // Pulsa via sin (0.5 a 1.0 em escala)
        float pulse = 0.5f + 0.5f * sinf(time * STATUS_VISUAL_BUFF_PULSE_SPEED);
        float radius = STATUS_VISUAL_BUFF_SPARK_RADIUS * pulse;
        
        Color sparkColor = buffColors[i];
        sparkColor.a = (unsigned char)(pulse * 200);
        
        DrawCircleV(buffPos, radius, sparkColor);
    }
    
    #if STATUS_VIS_DEBUG
    // === DEBUG OVERLAY: Contador de buffs ===
    DrawText(TextFormat("buffs: %d", buffCount), (int)spritePos.x, (int)(spritePos.y + spriteSize.y + 5), 12, GREEN);
    #endif
}

// WRAPPER GENÉRICO
void renderStatusVisuals(
    Texture2D sprite,
    Rectangle srcRect,
    Vector2 spritePos,
    Vector2 origin,
    float scale,
    StatusList* statusList
) {
    if (statusList == NULL) return;
    
    renderStatusAura(sprite, srcRect, spritePos, origin, scale, statusList);
    
    Vector2 spriteSize = {srcRect.width * scale, srcRect.height * scale};
    renderStatusBuffs(statusList, spritePos, spriteSize);
}
