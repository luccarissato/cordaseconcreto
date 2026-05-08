#include "raylib.h"
#include "state.h"
#include "dialogue.h"
#include "collision.h"
#include "../ui/menu.h"
#include "../entities/player.h"
#include "../entities/npc.h"
#include <stdio.h>

#include "../data/dialogues/teste_dialogue.h"

#define PARTY_SIZE 4
#define HISTORY_SIZE 1000

NPC testNPC;

Vector2 positionHistory[HISTORY_SIZE];
Direction directionHistory[HISTORY_SIZE];
int historyIndex = 0;
Texture2D mapTexture;
Camera2D camera;

Player party[PARTY_SIZE];

void initParty();
void updateParty(const Rectangle* blockers, int blockerCount);
void drawParty();
void unloadParty();

void initGame() {
    InitWindow(1920, 1080, "Cordas & Concreto");
    SetTargetFPS(60);

    initNPC(&testNPC, (Vector2){1400, 700}, "assets/NPCs/npc_placeholder.png", &testeTree);
    initMenu();
    initParty();
    initDialogue();

    camera.target = party[0].position; // segue o líder
    camera.offset = (Vector2){800, 540};
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    mapTexture = LoadTexture("assets/cenarios/bg_placeholder.png");
}

void updateGame() {
    switch (currentGameState) {
        case STATE_MENU:
            updateMenu();
            break;

        case STATE_EXPLORATION:
            Rectangle blockers[1] = {getColliderRect(testNPC.position, testNPC.collider)};
            updateParty(blockers, 1);
            camera.target = party[0].position;
            updateNPC(&testNPC, party[0].position);
            break;

        case STATE_DIALOGUE:
            updateDialogue();
            break;
    }
}

void drawGame() {
    BeginDrawing();
    ClearBackground(BLACK);

    switch (currentGameState) {
        case STATE_MENU:
            drawMenu();
            break;

        case STATE_EXPLORATION:
            BeginMode2D(camera);
            DrawTexture(mapTexture, 0, 0, WHITE);
            drawNPC(&testNPC);
            drawParty();
            EndMode2D();
            break;
        
        case STATE_DIALOGUE:
            BeginMode2D(camera);
            DrawTexture(mapTexture, 0, 0, WHITE);
            drawNPC(&testNPC);
            drawParty();
            EndMode2D();
            drawDialogue();
            break;
        }

    EndDrawing();
}

void closeGame() {
    unloadNPC(&testNPC);
    unloadMenu();
    closeDialogue();
    unloadParty();
    UnloadTexture(mapTexture);
    CloseWindow();
}

void initParty() {
    initPlayer(&party[0], "p1", (Vector2){960, 540});
    initPlayer(&party[1], "p2", (Vector2){960, 540});
    initPlayer(&party[2], "p3", (Vector2){960, 540});
    initPlayer(&party[3], "p4", (Vector2){960, 540});

    // inicializa histórico com posição inicial
    for (int i = 0; i < HISTORY_SIZE; i++) {
        positionHistory[i] = party[0].position;
        directionHistory[i] = party[0].direction;
    }
}

void updateParty(const Rectangle* blockers, int blockerCount) {
    // guarda posição anterior do líder
    Vector2 oldLeaderPos = party[0].position;

    // atualiza líder (input + movimento + animação)
    updatePlayer(&party[0], blockers, blockerCount);

    // verifica se o líder se moveu
    int leaderMoved = (oldLeaderPos.x != party[0].position.x || 
                       oldLeaderPos.y != party[0].position.y);

    // salva no histórico apenas se moveu
    if (leaderMoved) {
        positionHistory[historyIndex] = party[0].position;
        directionHistory[historyIndex] = party[0].direction;

        historyIndex = (historyIndex + 1) % HISTORY_SIZE;
    }

    // delay base entre personagens
    int delay = 30;

    // followers
    for (int i = 1; i < PARTY_SIZE; i++) {
        int index = historyIndex - (i * delay);

        while (index < 0) index += HISTORY_SIZE;

        Vector2 targetPos = positionHistory[index];
        Direction targetDir = directionHistory[index];

        // verifica se esse personagem se moveu
        int isMoving = (targetPos.x != party[i].position.x || 
                        targetPos.y != party[i].position.y);

        // aplica posição e direção
        party[i].position = targetPos;
        party[i].direction = targetDir;

        // animação dos followers
        updatePlayerAnimation(&party[i], isMoving);
    }
}

void drawParty() {
    for (int i = 0; i < PARTY_SIZE; i++) {
        drawPlayer(&party[i]);
    }
}

void unloadParty() {
    for (int i = 0; i < PARTY_SIZE; i++) {
        unloadPlayer(&party[i]);
    }
}