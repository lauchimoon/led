#ifndef LED_STATE
#define LED_STATE

#include <stdbool.h>

#include "raylib.h"
#include "theme.h"

#define LINE_SIZE       1024

#define FONT_SIZE_INIT     24
#define FONT_RESIZE_FACTOR 4
#define FONT_RESIZE_MIN    FONT_SIZE_INIT/2
#define FONT_RESIZE_MAX    FONT_SIZE_INIT*2

#define FILEPERM_RW 0
#define FILEPERM_RO 1
#define FILEPERM_WO 2
#define FILEPERM_NO 3

typedef char *Line;

typedef struct LedState {
    const char *filename;
    int cursor;
    int font_size;
    Font font;
    LedTheme theme;
    int line;
    int lines_capacity;
    int lines_num;
    Line *lines;
    Camera2D camera;
    Line copy_buffer;

    int fileperm;
    bool dirty;
    const char *title;
    bool exit;
} LedState;

int state_init(LedState *state, const char *filename);
int load_file(LedState *state);
void write_file(LedState *state);
void state_load_font(LedState *state);
void state_deinit(LedState *state);

#endif // LED_STATE
