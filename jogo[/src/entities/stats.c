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