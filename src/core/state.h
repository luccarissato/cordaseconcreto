#ifndef STATE_H
#define STATE_H

typedef enum {
	STATE_MENU,
	STATE_EXPLORATION,
	STATE_COMBAT,
	STATE_DIALOGUE,
    STATE_GAME_MENU
} GameState;

typedef enum {
    MENU_MAIN,
    MENU_GAME_MAIN,
    MENU_ITEMS_CATEGORY,
    MENU_ITEMS_LIST,
    MENU_ITEM_TARGET
} MenuState;

extern GameState currentGameState;
extern MenuState currentMenuState;

#endif
