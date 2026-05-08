#include "teste_dialogue.h"
#include <stdio.h>

DialogueNode testeNodes[] = {

    {
        "Esse diálogo está funcionando?",
        1,
        "SIM",
        "NAO",
        1,
        2
    },

    {
        "Ótimo!",
        0,
        NULL,
        NULL,
        -1,
        -1
    },

    {
        "Droga.",
        0,
        NULL,
        NULL,
        -1,
        -1
    }
};

DialogueTree testeTree = {
    testeNodes,
    3,
    0,
    0,
    0
};