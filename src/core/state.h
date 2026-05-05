#ifndef STATE_H
#define STATE_H

typedef enum {
	STATE_MENU,
	STATE_EXPLORATION,
	STATE_COMBAT,
	STATE_DIALOGUE
} GameState;

typedef enum {
	MENU_MAIN,
	MENU_PAUSE,
	MENU_ITEMS
} MenuState;

extern GameState currentGameState;
extern MenuState currentMenuState;

#endif
