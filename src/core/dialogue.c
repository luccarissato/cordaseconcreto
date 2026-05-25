#include "dialogue.h"
#include "state.h"
#include <stdio.h>

static Texture2D dialogueBg;
static DialogueTree* currentTree = NULL;

    void initDialogue() {
        dialogueBg = LoadTexture(
            "assets/interface/dialogo_background.png"
        );
    }

    void startDialogue(DialogueTree* tree) {
        currentTree = tree;
        currentTree->currentNode = 0;
        currentTree->active = 1;
        currentTree->selectedChoice = 0;
        currentGameState = STATE_DIALOGUE;
    }

    void updateDialogue() {
        if (!currentTree || !currentTree->active) return;

        DialogueNode* node = &currentTree->nodes[currentTree->currentNode];

        if (node->hasChoice) {
            if (IsKeyPressed(KEY_LEFT)) {
                currentTree->selectedChoice = 0;
            }

            if (IsKeyPressed(KEY_RIGHT)) {
                currentTree->selectedChoice = 1;
            }
        }


        if (IsKeyPressed(KEY_Z)) {
            int nextNode;
            if (node->hasChoice) {
                nextNode =
                    currentTree->selectedChoice == 0
                    ? node->nextLeft
                    : node->nextRight;

            } else {
                nextNode = node->nextLeft;
            }

            if (nextNode == -1) {
                currentTree->active = 0;
                currentGameState = STATE_EXPLORATION;
                return;
            }

            currentTree->currentNode = nextNode;
            currentTree->selectedChoice = 0;
        }
}

void drawDialogue() {
    if (!currentTree || !currentTree->active) return;

    DialogueNode* node = &currentTree->nodes[currentTree->currentNode];

    int bgX = 160;
    int bgY = 700;

    DrawTexture(dialogueBg, bgX, bgY, WHITE);

    DrawText(
        node->text,
        bgX + 160,
        bgY + 160,
        32,
        BLACK
    );

    if (node->hasChoice) {

        Rectangle leftBox = {
            700,
            880,
            200,
            80
        };

        Rectangle rightBox = {
            1000,
            880,
            200,
            80
        };

        DrawRectangleRec(leftBox, currentTree->selectedChoice == 0 ? DARKGRAY: GRAY);
        DrawRectangleRec(rightBox, currentTree->selectedChoice == 1 ? DARKGRAY: GRAY);
        DrawText(node->optionLeft, leftBox.x + 40, leftBox.y + 25, 30, WHITE);
        DrawText(node->optionRight, rightBox.x + 40, rightBox.y + 25, 30, WHITE);
    }
}

int isDialogueActive() {
    return currentTree && currentTree->active;
}

void closeDialogue() {
    UnloadTexture(dialogueBg);
}