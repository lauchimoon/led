#ifndef LED_KEYBINDS
#define LED_KEYBINDS

#include "state.h"

enum {
    RESIZE_ACTION_INCREASE = 0,
    RESIZE_ACTION_DECREASE,
};

void handle_cursor_movement(LedState *state);
void move_to_start(LedState *state);
void move_to_end(LedState *state);
void resize_font(LedState *state, int action);

#endif // LED_KEYBINDS
