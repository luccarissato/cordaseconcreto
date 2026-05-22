#ifndef BOSS_AI_H
#define BOSS_AI_H

#include "../entities/enemy.h"
#include "../entities/status_condition.h"

#define BOSS_AI_MAX_MESSAGES 8
#define BOSS_AI_MESSAGE_LEN 160

void bossAiOnCombatStart(void);
void bossAiOnCombatEnd(void);
int bossAiHandleEnemyTurn(int worldEnemyIndex, Enemy* enemy);
int bossAiGetPlayerWeaknessElement(int playerIndex);

void bossAiQueueMessage(const char* fmt, ...);
void bossAiQueuePlayerAfflictedMessage(int playerIndex, StatusType statusType);
void bossAiUpdateMessages(float deltaTime);
int bossAiHasActiveMessage(void);
const char* bossAiGetCurrentMessage(void);

#endif
