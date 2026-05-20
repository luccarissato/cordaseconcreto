#ifndef STATS_H
#define STATS_H

typedef struct {
    int baseHP;
    int maxHP;
    int currentHP;

    int baseMana;
    int maxMana;
    int currentMana;

    int fortitude;
    int mente;
    int forca;
    int defesa;
    int velocidade;

    int defCalor;
    int defVento;
    int defMare;
    int defTerra;

} Stats;

void calculateStats(Stats* stats);

/**
 * applyLevelGrowth - Aplica o crescimento de atributos por level up
 * 
 * Incrementa os atributos base em +5:
 * - fortitude
 * - mente
 * - forca
 * - defesa
 * - velocidade
 * 
 * NÃO afeta:
 * - baseHP/baseMana (crescem indiretamente via fortitude/mente)
 * - Defesas elementais
 * 
 * IMPORTANTE: Chamar calculateStats() após este função para recalcular
 * derivados (maxHP, maxMana).
 */
void applyLevelGrowth(Stats* stats);

#endif