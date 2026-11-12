#ifndef LED_LINE_OPS
#define LED_LINE_OPS

#include "state.h"

bool new_line(LedState *state);
void split_line(LedState *state);
void delete_char_cursor(LedState *state);
void append_char_cursor(LedState *state, int c);
void delete_line(LedState *state);
void append_tab(LedState *state);
void copy_line(LedState *state);
void paste_line(LedState *state);
void move_line_above(LedState *state);

#endif // LED_LINE_OPS
