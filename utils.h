#ifndef LED_UTILS
#define LED_UTILS

#include <stdbool.h>

#include "state.h"

int get_char_width(LedState *state);
int get_number_column_width(LedState *state);
bool key_pressed_or_repeat(int key);

#endif // LED_UTILS
