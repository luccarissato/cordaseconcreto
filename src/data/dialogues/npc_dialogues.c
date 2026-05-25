#include "npc_dialogues.h"
#include <stddef.h>

DialogueNode npc1Nodes[] = {
    {
        "Esses monstros acabaram com nosso bairro... mas nunca vao acabar com a gente.",
        0,
        NULL,
        NULL,
        1,
        -1
    },
    {
        "Enquanto a gente ainda tiver nossa cultura, nossos costumes, nosso frevo,\n\na gente nunca vai cair.",
        0,
        NULL,
        NULL,
        -1,
        -1
    }
};

DialogueTree npc1Dialogue = {
    npc1Nodes,
    2,
    0,
    0,
    0
};

DialogueNode npc2Nodes[] = {
    {
        "Sem a banda, o Recife Antigo ficou tao quieto, e esquisito demais.",
        0,
        NULL,
        NULL,
        1,
        -1
    },
    {
        "Eu ate tentei juntar o pessoal de novo, trouxe ate meu trompete,\n\nmas eles tao com medo demais pra isso.",
        0,
        NULL,
        NULL,
        -1,
        -1
    }
};

DialogueTree npc2Dialogue = {
    npc2Nodes,
    2,
    0,
    0,
    0
};

DialogueNode npc3Nodes[] = {
    {
        "Sinto tanta saudade da minha Joaquina, nos faziamos tudo juntos...",
        0,
        NULL,
        NULL,
        1,
        -1
    },
    {
        "Comiamos, cantavamos, ate frevo dancavamos juntos, mesmo que nao fosse\n\ncerto a gente dava um jeito.",
        0,
        NULL,
        NULL,
        -1,
        -1
    }
};

DialogueTree npc3Dialogue = {
    npc3Nodes,
    2,
    0,
    0,
    0
};
