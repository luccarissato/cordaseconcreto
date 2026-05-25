#ifndef DIALOGUE_H
#define DIALOGUE_H
#include "raylib.h"

typedef struct {
    const char* text;
    int hasChoice;
    const char* optionLeft;
    const char* optionRight;
    int nextLeft;
    int nextRight;

} DialogueNode;

typedef struct {
    DialogueNode* nodes;
    int nodeCount;
    int currentNode;
    int active;
    int selectedChoice;
} DialogueTree;

void initDialogue();
void startDialogue(DialogueTree* tree, const char* speaker);
void updateDialogue();
void drawDialogue();
int isDialogueActive();
void closeDialogue();

#endif