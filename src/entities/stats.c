#include "stats.h"

void calculateStats(Stats* stats) {

    stats->maxHP =
        stats->baseHP +
        (stats->fortitude * 15);

    stats->maxMana =
        stats->baseMana +
        (stats->mente * 7);

    stats->currentHP = stats->maxHP;
    stats->currentMana = stats->maxMana;
}

/**
 * applyLevelGrowth - Aplica crescimento de stats ao subir de nível
 * 
 * Incrementa cada atributo base em +5:
 * - fortitude
 * - mente
 * - forca
 * - defesa
 * - velocidade
 * 
 * Consequências automáticas via calculateStats():
 * - HP aumenta automaticamente (+5 fortitude = +75 maxHP)
 * - Mana aumenta automaticamente (+5 mente = +35 maxMana)
 * 
 * REGRA CRÍTICA: Chamar calculateStats() após esta função
 * para que os derivados (maxHP, maxMana) sejam recalculados.
 */
void applyLevelGrowth(Stats* stats) {
    stats->fortitude += 5;
    stats->mente += 5;
    stats->forca += 5;
    stats->defesa += 5;
    stats->velocidade += 5;
    
    /* Defesas elementais NÃO escalem com nível */
    /* baseHP e baseMana também NÃO escalem (crescem indiretamente) */
}