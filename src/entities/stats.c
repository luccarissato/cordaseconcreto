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

void applyLevelGrowth(Stats* stats) {
    stats->fortitude += 5;
    stats->mente += 5;
    stats->forca += 5;
    stats->defesa += 5;
    stats->velocidade += 5;
    
    /* Defesas elementais NÃO escalem com nível */
    /* baseHP e baseMana também NÃO escalem (crescem indiretamente) */
}