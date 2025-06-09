#ifndef LED_THEME
#define LED_THEME

typedef struct LedTheme {
    Color background_color;
    Color text_color;
    Color hud_color;
} LedTheme;

#define NUM_THEMES 2

LedTheme themes[NUM_THEMES] = {
    (LedTheme){ (Color){ 26, 26, 26, 255 }, RAYWHITE, DARKGRAY },
    (LedTheme){ RAYWHITE, BLACK, GRAY },
};

#endif // LED_THEME
