#ifndef LED_THEME
#define LED_THEME

#include "raylib.h"

typedef struct LedTheme {
    Color background_color;
    Color text_color;
    Color hud_color;
} LedTheme;

#define NUM_THEMES 2

extern const LedTheme themes[NUM_THEMES];

#endif // LED_THEME
