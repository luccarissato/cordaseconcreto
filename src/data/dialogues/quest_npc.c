#include "quest_npc.h"
#include <stddef.h>

DialogueNode questIntroNodes[] = {
    { "Ah, eu reconheco voces.", 0, NULL, NULL, 1, -1 },
    { "Vocês vieram se livrar das criaturas que estão assolando esse lugar, não é?", 0, NULL, NULL, 2, -1 },
    { "Eu posso ajudar vocês, mas primeiro preciso saber se estão aptos para a tarefa.", 0, NULL, NULL, 3, -1 },
    { "Existem três itens escondidos pela região, mas a sua localização\n\nestá escondida por uma charada:", 0, NULL, NULL, 4, -1 },
    { "O primeiro descansa atrás do aço esquecido.", 0, NULL, NULL, 5, -1 },
    { "O segundo se esconde no peito de carne e memória.", 0, NULL, NULL, 6, -1 },
    { "O terceiro espera aonde a quarta raiz encosta o chão.", 0, NULL, NULL, 7, -1 },
    { "Se trazerem esses itens pra mim, os recompensarei a altura.", 0, NULL, NULL, -1, -1 }
};

DialogueTree questNpcIntroDialogue = {
    questIntroNodes,
    (int)(sizeof(questIntroNodes) / sizeof(questIntroNodes[0])),
    0,
    0,
    0
};

DialogueNode questCompleteNodes[] = {
    { "Ah, então você conseguiu encontrar todos.", 0, NULL, NULL, 1, -1 },
    { "Melhor do que eu esperava.", 0, NULL, NULL, 2, -1 },
    { "Bem, promessa é divida, aqui está sua recompensa.", 0, NULL, NULL, -1, -1 }
};

DialogueTree questNpcCompleteDialogue = {
    questCompleteNodes,
    (int)(sizeof(questCompleteNodes) / sizeof(questCompleteNodes[0])),
    0,
    0,
    0
};
