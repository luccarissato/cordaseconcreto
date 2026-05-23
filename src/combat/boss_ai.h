#ifndef BOSS_AI_H
#define BOSS_AI_H

#include "../entities/enemy.h"
#include "../entities/status_condition.h"

#define BOSS_AI_MAX_MESSAGES 8
#define BOSS_AI_MESSAGE_LEN 160

typedef struct {
	const char* text;
	const char* optionLeft;
	const char* optionRight;
} BossAiPrompt;

void bossAiOnCombatStart(void);
void bossAiOnCombatEnd(void);
void bossAiOnRoundWrap(void);
int bossAiHandleEnemyTurn(int worldEnemyIndex, Enemy* enemy);
int bossAiGetPlayerWeaknessElement(int playerIndex);
int bossAiHasTrapMark(int playerIndex);
void bossAiNotifyPlayerPhysicalAction(int playerIndex, int isPhysicalAction);
void bossAiNotifyPlayerAbilityUsed(int playerIndex, int abilityIndex);
void bossAiAdjustBoss3Debt(int delta);
int bossAiGetBoss3Debt(void);
int bossAiIsBoss3Phase2(void);
int bossAiHasTemptationMark(int playerIndex);

void bossAiQueueMessage(const char* fmt, ...);
void bossAiQueuePlayerAfflictedMessage(int playerIndex, StatusType statusType);
void bossAiUpdateMessages(float deltaTime);
int bossAiHasActiveMessage(void);
const char* bossAiGetCurrentMessage(void);

void bossAiClearPromptQueue(void);
int bossAiHasPendingPrompt(void);
const BossAiPrompt* bossAiGetCurrentPrompt(void);
int bossAiResolveCurrentPrompt(int accepted);

#endif
