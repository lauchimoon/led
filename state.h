#ifndef LED_STATE
#define LED_STATE

#include <stdbool.h>

#include "raylib.h"
#include "theme.h"
#include "undo.h"

#define LINE_SIZE       1024

#define FONT_SIZE_INIT     24
#define FONT_RESIZE_FACTOR 4
#define FONT_RESIZE_MIN    FONT_SIZE_INIT/2
#define FONT_RESIZE_MAX    FONT_SIZE_INIT*2

typedef char *Line;

typedef struct LedState {
    const char *title;
    const char *filename;
    bool exit;
    LedTheme theme;

    Line *lines;
    int lines_capacity;
    int lines_num;

    int line;
    int line_scroll;
    int cursor;

    Font font;
    int font_size;

    bool dirty;

    Camera2D camera;

    UndoBuffer *undo;
} LedState;

void state_init(LedState *state, const char *filename);
void load_file(LedState *state);
void write_file(LedState *state);
void state_load_font(LedState *state);
void state_deinit(LedState *state);

#endif // LED_STATE
