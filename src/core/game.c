#include "raylib.h"
#include "state.h"
#include "game.h"
#include "dialogue.h"
#include "collision.h"
#include "../ui/menu.h"
#include "../entities/player.h"
#include "../entities/npc.h"
#include "../entities/enemy.h"
#include "../items/inventory.h"
#include "../ui/game_menu.h"
#include "../interactables/interactable.h"
#include "../combat/combat.h"
#include "../combat/ability.h"
#include "../ui/combat_ui.h"
#include "../worlds/worlds.h"
#include "../worlds/world_mc.h"
#include "../worlds/world_cs.h"
#include "../worlds/world_pc.h"
#include "../worlds/world_1andar.h"
#include "../worlds/world_2andar.h"
#include "../combat/boss_ai.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <stdio.h>

#define HISTORY_SIZE 1000

NPC testNPC;
InteractableManager interactableManager;
EnemyManager enemyManager;
Combat combat;

Vector2 positionHistory[HISTORY_SIZE];
Direction directionHistory[HISTORY_SIZE];
int historyIndex = 0;
Texture2D mapTexture;
Camera2D camera;

Player party[PARTY_SIZE];
Inventory playerInventory;

static void clearPlayerInventory(void) {
    ListNode* current = playerInventory.items.head;
    int count = playerInventory.items.size;

    for (int i = 0; i < count && current != NULL; i++) {
        ListNode* next = current->next;
        InventoryItem* item = (InventoryItem*)current->data;
        if (item != NULL) {
            free(item);
        }
        current = next;
    }

    clearList(&playerInventory.items);
}

void initParty(int applyGrowth, Vector2 spawnPosition);
void updateParty(const Rectangle* blockers, int blockerCount);
void drawParty();
void unloadParty();

void applyPartyDamage(int damage);


void initEnemyManager();
void spawnEnemy(const char* name, Vector2 position, const char* texturePath, int maxEnemies);
void updateEnemies(const Rectangle* blockers, int blockerCount);
void drawEnemies();
void getEnemyBlockers(Rectangle* outBlockers, int* outCount);
void unloadEnemyManager();

void startCombat(Vector2 playerPos, float combatDistance);
int getCombatState();
Enemy** getEnemiesInCombat();
int getEnemyCombatCount();
void endCombat();

static Rectangle getCurrentWorldBounds(void) {
    Rectangle worldBounds = {0.0f, 0.0f, 0.0f, 0.0f};

    getCurrentWorldCameraBounds(&worldBounds);
    if (worldBounds.width <= 0.0f || worldBounds.height <= 0.0f) {
        if (mapTexture.id != 0 && mapTexture.width > 0 && mapTexture.height > 0) {
            worldBounds = (Rectangle){0.0f, 0.0f, (float)mapTexture.width, (float)mapTexture.height};
        }
    }

    return worldBounds;
}

static float getWorldFitZoom(Rectangle worldBounds) {
    if (worldBounds.width <= 0.0f || worldBounds.height <= 0.0f) {
        return 1.0f;
    }

    float viewportWidth = (float)GetScreenWidth();
    float viewportHeight = (float)GetScreenHeight();
    float zoomX = viewportWidth / worldBounds.width;
    float zoomY = viewportHeight / worldBounds.height;
    float zoom = (zoomX < zoomY) ? zoomX : zoomY;

    if (zoom <= 0.0f) {
        return 1.0f;
    }

    return zoom;
}

void configureCameraForCurrentWorld(void) {
    Rectangle worldBounds = getCurrentWorldBounds();

    camera.offset = (Vector2){ (float)GetScreenWidth() * 0.5f, (float)GetScreenHeight() * 0.5f };
    camera.rotation = 0.0f;

    if (worldBounds.width > 0.0f && worldBounds.height > 0.0f) {
        camera.target = (Vector2){ worldBounds.x + (worldBounds.width * 0.5f), worldBounds.y + (worldBounds.height * 0.5f) };
        camera.zoom = getWorldFitZoom(worldBounds);
        return;
    }

    camera.target = (Vector2){ 0.0f, 0.0f };
    camera.zoom = 1.0f;
}

void updateCameraTarget(Vector2 target) {
    (void)target;
    configureCameraForCurrentWorld();
}

void initGame() {
    SetConfigFlags(FLAG_FULLSCREEN_MODE);
    InitWindow(1920, 1080, "Cordas & Concreto");
    Image windowIcon = LoadImage("assets/icones/window_icon.png");
    SetWindowIcon(windowIcon);
    UnloadImage(windowIcon);
    SetTargetFPS(60);
    srand((unsigned int) time(NULL));  /* Seed para random number generator */

    initMenu();
    initGameMenu();
    initDialogue();
    initInventory(&playerInventory);
    clearPlayerInventory();
    initCombat();
    initCombatUI();
    initWorldRegistry();
    RegisterWorldMC();
    RegisterWorldCS();
    RegisterWorldPC();
    RegisterWorld1Andar();
    RegisterWorld2Andar();
    loadCurrentWorld();
}

void updateGame() {
    /* Update boss message timers so queued messages appear during exploration/dialogue. */
    bossAiUpdateMessages(GetFrameTime());
    if (currentGameState != STATE_COMBAT) {
        processPendingWorldLoad();
    }

    switch (currentGameState) {
        case STATE_MENU:
            updateMenu();
            break;

        case STATE_EXPLORATION:
            if (IsKeyPressed(KEY_X)) {
                openGameMenu();
            }

            /* Coleta blockers de NPC, interagíveis e inimigos */
            int worldBlockerCount = 0;
            int interactableBlockerCount = 0;
            collectCurrentWorldBlockers(NULL, &worldBlockerCount);
            Rectangle* interactableBlockers = getInteractableBlockers(
                &interactableManager, 
                &interactableBlockerCount
            );
            int npcBlockerCount = 0;
            collectLoadedNpcBlockers(NULL, &npcBlockerCount);
            
            /* Coleta blockers de inimigos */
            Rectangle enemyBlockers[MAX_ENEMIES];
            int enemyBlockerCount = 0;
            getEnemyBlockers(enemyBlockers, &enemyBlockerCount);
            
            /* Monta array com todos os blockers (NPC + mundo + interactables + enemies) */
            int totalBlockerCount = npcBlockerCount + worldBlockerCount + interactableBlockerCount + enemyBlockerCount;
            Rectangle* allBlockers = malloc(sizeof(Rectangle) * totalBlockerCount);
            int blockerOffset = 0;

            if (npcBlockerCount > 0) {
                collectLoadedNpcBlockers(&allBlockers[blockerOffset], &npcBlockerCount);
                blockerOffset += npcBlockerCount;
            }

            if (worldBlockerCount > 0) {
                collectCurrentWorldBlockers(&allBlockers[blockerOffset], &worldBlockerCount);
                blockerOffset += worldBlockerCount;
            }

            for (int i = 0; i < interactableBlockerCount; i++) {
                allBlockers[blockerOffset + i] = interactableBlockers[i];
            }
            for (int i = 0; i < enemyBlockerCount; i++) {
                allBlockers[blockerOffset + interactableBlockerCount + i] = enemyBlockers[i];
            }
            
            updateParty(allBlockers, totalBlockerCount);
            updateEnemies(allBlockers, totalBlockerCount);
            
            free(allBlockers);
            free(interactableBlockers);
            
            updateLoadedNpcs(party[0].position);
            updateInteractables(&interactableManager, party[0].position);
            processCurrentWorldTriggers(party[0].position);
            if (processPendingWorldLoad()) {
                break;
            }
            
            updateEnemiesInteraction(party[0].position);
            
            break;

        case STATE_DIALOGUE:
            updateDialogue();
            break;

        case STATE_GAME_MENU:
            updateGameMenu();
            if (IsKeyPressed(KEY_X)) {
                closeGameMenu();
            }
            break;
        
        case STATE_COMBAT:
            updateCombatUI();
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
            drawCurrentWorldOverlay();
            drawLoadedNpcs();
            drawInteractables(&interactableManager);
            drawEnemies();
            drawParty();
            EndMode2D();
            /* Show queued boss messages during exploration (same styling used in combat UI) */
            if (bossAiHasActiveMessage()) {
                const char* message = bossAiGetCurrentMessage();
                if (message != NULL) {
                    DrawRectangle(220, 740, 1480, 180, ColorAlpha(BLACK, 0.85f));
                    DrawRectangleLines(220, 740, 1480, 180, GRAY);
                    DrawText(message, 260, 810, 30, RAYWHITE);
                }
            }
            break;
        
        case STATE_DIALOGUE:
            BeginMode2D(camera);
            DrawTexture(mapTexture, 0, 0, WHITE);
            drawCurrentWorldOverlay();
            drawLoadedNpcs();
            drawInteractables(&interactableManager);
            drawEnemies();
            drawParty();
            EndMode2D();
            drawDialogue();
            if (bossAiHasActiveMessage()) {
                const char* message = bossAiGetCurrentMessage();
                if (message != NULL) {
                    DrawRectangle(220, 740, 1480, 180, ColorAlpha(BLACK, 0.85f));
                    DrawRectangleLines(220, 740, 1480, 180, GRAY);
                    DrawText(message, 260, 810, 30, RAYWHITE);
                }
            }
            break;
        
        case STATE_COMBAT:
            drawCombatUI();
            break;
        
        case STATE_GAME_MENU:
            BeginMode2D(camera);
            DrawTexture(mapTexture, 0, 0, WHITE);
            drawCurrentWorldOverlay();
            drawLoadedNpcs();
            drawInteractables(&interactableManager);
            drawEnemies();
            drawParty();
            EndMode2D();
            drawGameMenu();
            break;
        }

    EndDrawing();
}

void closeGame() {
    shutdownWorldRegistry();
    unloadMenu();
    closeDialogue();
    unloadCombatUI();
    unloadGameMenu();
    CloseWindow();
}

void initParty(int applyGrowth, Vector2 spawnPosition) {
    initPlayer(&party[0], "p1", spawnPosition, "O Mestre");
    initPlayer(&party[1], "p2", spawnPosition, "O Arauto");
    initPlayer(&party[2], "p3", spawnPosition, "A Cirandeira");
    initPlayer(&party[3], "p4", spawnPosition, "O Repentista");

    //maracatu
    party[0].stats.baseHP = 60;
    party[0].stats.baseMana = 25;
    party[0].stats.fortitude = 8;
    party[0].stats.mente = 5;
    party[0].stats.forca = 6;
    party[0].stats.defesa = 7;
    party[0].stats.velocidade = 2;
    party[0].stats.defCalor = 60;
    party[0].stats.defVento = 30;
    party[0].stats.defMare = 50;
    party[0].stats.defTerra = 80;

    //manguebeat
    party[1].stats.baseHP = 50;
    party[1].stats.baseMana = 25;
    party[1].stats.fortitude = 6;
    party[1].stats.mente = 4;
    party[1].stats.forca = 7;
    party[1].stats.defesa = 4;
    party[1].stats.velocidade = 7;
    party[1].stats.defCalor = 50;
    party[1].stats.defVento = 80;
    party[1].stats.defMare = 60;
    party[1].stats.defTerra = 30;

    //cirandeira
    party[2].stats.baseHP = 45;
    party[2].stats.baseMana = 35;
    party[2].stats.fortitude = 5;
    party[2].stats.mente = 10;
    party[2].stats.forca = 3;
    party[2].stats.defesa = 4;
    party[2].stats.velocidade = 6;
    party[2].stats.defCalor = 40;
    party[2].stats.defVento = 80;
    party[2].stats.defMare = 70;
    party[2].stats.defTerra = 50;

    //repentista
    party[3].stats.baseHP = 35;
    party[3].stats.baseMana = 40;
    party[3].stats.fortitude = 4;
    party[3].stats.mente = 9;
    party[3].stats.forca = 2;
    party[3].stats.defesa = 3;
    party[3].stats.velocidade = 7;
    party[3].stats.defCalor = 80;
    party[3].stats.defVento = 60;
    party[3].stats.defMare = 40;
    party[3].stats.defTerra = 60;

    for (int i = 0; i < PARTY_SIZE; i++) {
        calculateStats(&party[i].stats);
        /* Inicializa o tipo de personagem (Tank, DPS, Healer, Mage) */
        party[i].characterID = i;
        /* Personagem começa nível 1 — preserve se já tiver level > 0 */
        if (party[i].level == 0) {
            party[i].level = 1;
        }
        
        if (applyGrowth) {
            /* Aplica growth para atingir nível 4 (3 level ups: 1->2->3->4) */
            for (int levelUp = 0; levelUp < 3; levelUp++) {
                applyLevelGrowth(&party[i].stats);
                party[i].level++;
            }
        }
        
        /* Recalcula stats derivados com o nível final */
        calculateStats(&party[i].stats);
    }

    historyIndex = 0;

    // inicializa histórico com posição inicial
    for (int i = 0; i < HISTORY_SIZE; i++) {
        positionHistory[i] = spawnPosition;
        directionHistory[i] = party[0].direction;
    }
}

void resetGameState() {
    for (int i = 0; i < PARTY_SIZE; i++) {
        party[i].level = 0;
    }

    currentMenuState = MENU_MAIN;
    currentGameState = STATE_MENU;
    clearPlayerInventory();
    unloadEnemyManager();
    initEnemyManager();
    requestWorldLoadFirst();
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
    for (int i = 1; i < PARTY_SIZE; i++) {
        drawPlayer(&party[i]);
    }

    drawPlayer(&party[0]);
}

void unloadParty() {
    for (int i = 0; i < PARTY_SIZE; i++) {
        unloadPlayer(&party[i]);
    }
}

void applyPartyDamage(int damage) {
    if (damage <= 0) {
        return;
    }

    for (int i = 0; i < PARTY_SIZE; i++) {
        Player* member = &party[i];

        if (member->stats.currentHP <= 0) {
            member->stats.currentHP = 0;
            member->isAlive = 0;
            continue;
        }

        member->stats.currentHP -= damage;
        if (member->stats.currentHP <= 0) {
            member->stats.currentHP = 0;
            member->isAlive = 0;
        }
    }
}

Player* getPartyMembers(int* outCount) {
    if (outCount != NULL) {
        *outCount = PARTY_SIZE;
    }

    return party;
}
