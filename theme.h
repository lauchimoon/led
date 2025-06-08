#ifndef LAME_THEME
#define LAME_THEME

typedef struct LameTheme {
    Color background_color;
    Color text_color;
    Color hud_color;
} LameTheme;

#define NUM_THEMES 2

LameTheme themes[NUM_THEMES] = {
    (LameTheme){ (Color){ 26, 26, 26, 255 }, RAYWHITE, DARKGRAY },
    (LameTheme){ RAYWHITE, BLACK, GRAY },
};

#endif // LAME_THEME
