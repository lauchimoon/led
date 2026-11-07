#ifndef LED_DRAW
#define LED_DRAW

#include "state.h"

void draw_text(LedState *state, const char *text, int x, int y, Color color);
void draw_cursor(LedState *state);
void draw_hud(LedState *state);

#endif // LED_DRAW
