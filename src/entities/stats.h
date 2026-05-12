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

#endif