#include "door_questions.h"
#include <stddef.h>

DialogueNode doorQuestion1Nodes[] = {
    /* Node 0: A pergunta */
    {
        "Quanto e 2 + 2?",
        1,
        "4",           
        "5",          
        1,             
        2              
    },
    
    {
        "Correto! A porta se abre...",
        0,
        NULL,
        NULL,
        -1,            
        -1
    },
    
    {
        "Errado! A porta continua fechada.",
        0,
        NULL,
        NULL,
        -1,           
        -1
    }
};

DialogueTree doorQuestion1 = {
    doorQuestion1Nodes,
    3,
    0,
    0,
    0
};

DialogueNode doorQuestion2Nodes[] = {
    {
        "Qual eh a capital do Brasil?",
        1,
        "Brasilia",   
        "Rio",         
        1,
        2
    },
    
    {
        "Correto! A porta se abre...",
        0,
        NULL,
        NULL,
        -1,
        -1
    },
    
    {
        "Errado! A porta continua fechada.",
        0,
        NULL,
        NULL,
        -1,
        -1
    }
};

DialogueTree doorQuestion2 = {
    doorQuestion2Nodes,
    3,
    0,
    0,
    0
};

DialogueNode doorQuestion3Nodes[] = {
    {
        "Raylib eh uma biblioteca para...",
        1,
        "Graficos",    /* Resposta correta (optionLeft) */
        "Dados",       /* Resposta errada (optionRight) */
        1,
        2
    },
    
    {
        "Correto! A porta se abre...",
        0,
        NULL,
        NULL,
        -1,
        -1
    },
    
    {
        "Errado! A porta continua fechada.",
        0,
        NULL,
        NULL,
        -1,
        -1
    }
};

DialogueTree doorQuestion3 = {
    doorQuestion3Nodes,
    3,
    0,
    0,
    0
};
