#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#define WINDOW_WIDTH    800
#define WINDOW_HEIGHT   600
#define FPS             60

#define LINE_SIZE       1024

#define DELETE_COOLDOWN 3

typedef char *Line;

typedef struct LameState {
    char *title;
    bool exit;

    Line *lines;
    int lines_capacity;
    int lines_num;
    int line;
    int cursor;

    int delete_cooldown;

    Font font;
    int font_size;
} LameState;

void state_init(LameState *);
void state_deinit(LameState *);

bool any_key_pressed(int *);

void handle_editor_events(LameState *);
void handle_cursor_movement(LameState *);

void new_line(LameState *);
void delete_char_cursor(LameState *);
void append_char_cursor(LameState *, int);

void draw_text(LameState *, const char *, int, int, Color);
void draw_cursor(LameState *);

int main(int argc, char **argv)
{
    LameState state = {
        .title = "lame",
        .exit  = false,
    };

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, state.title);
    SetWindowMinSize(WINDOW_WIDTH/2, WINDOW_HEIGHT/2);

    state_init(&state);

    SetTargetFPS(FPS);

    while (!state.exit) {
        handle_editor_events(&state);

        handle_cursor_movement(&state);

        BeginDrawing();
        ClearBackground(RAYWHITE);
        for (int i = 0; i < state.lines_num; ++i)
            draw_text(&state, state.lines[i], 0, i*state.font_size, BLACK);
        draw_cursor(&state);
        EndDrawing();
    }

    state_deinit(&state);
    CloseWindow();
    return 0;
}

void state_init(LameState *state)
{
    state->font_size = 24;
    state->font = LoadFontEx("fonts/GeistMono-Regular.ttf", state->font_size, NULL, 95);

    state->lines_capacity = 100;
    state->line = -1;
    state->lines = calloc(state->lines_capacity, sizeof(Line));
    new_line(state);

    state->delete_cooldown = 0;

    state->cursor = 0;
}

void state_deinit(LameState *state)
{
    UnloadFont(state->font);
    state->font_size = 0;

    for (int i = 0; i < state->lines_num; ++i)
        free(state->lines[i]);

    free(state->lines);
    state->lines_capacity = 0;
    state->lines_num = 0;
    state->line = 0;
    state->cursor = 0;
}

bool any_key_pressed(int *key)
{
    *key = GetKeyPressed();
    return *key >= ' ' && *key <= KEY_KB_MENU;
}

void handle_editor_events(LameState *state)
{
    ++state->delete_cooldown;
    if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_Q))
        state->exit = true;

    if (state->cursor < 0)
        state->cursor = 0;

    if (IsKeyDown(KEY_BACKSPACE) && state->cursor > 0 && state->delete_cooldown % DELETE_COOLDOWN == 0)
        delete_char_cursor(state);

    int key;
    if (any_key_pressed(&key)) {
        int c = GetCharPressed();
        if (key == KEY_ENTER)
            new_line(state);
        else if (c >= ' ' && c <= '~')
            append_char_cursor(state, c);
    }
}

void handle_cursor_movement(LameState *state)
{
    if (IsKeyPressed(KEY_LEFT))
        state->cursor -= state->cursor > 0? 1 : 0;
    else if (IsKeyPressed(KEY_RIGHT))
        state->cursor += state->cursor < strlen(state->lines[state->line])? 1 : 0;
    else if (IsKeyPressed(KEY_UP)) {
        if (state->line > 0)
            --state->line;
        else
            state->cursor = 0;
    }
    else if (IsKeyPressed(KEY_DOWN)) {
        if (state->line + 1 < state->lines_num)
            ++state->line;
        else
            state->cursor = strlen(state->lines[state->line]);
    }
}

void new_line(LameState *state)
{
    Line new = calloc(LINE_SIZE, sizeof(char));
    ++state->lines_num;
    ++state->line;
    if (state->line >= state->lines_capacity/2) {
        state->lines_capacity *= 2;
        state->lines = realloc(state->lines, state->lines_capacity*sizeof(Line));
    }
    
    state->lines[state->line] = new;
    state->cursor = 0;
}

void delete_char_cursor(LameState *state)
{
    state->lines[state->line][state->cursor - 1] = 0;
    --state->cursor;
}

void append_char_cursor(LameState *state, int c)
{
    state->lines[state->line][state->cursor++] = c;
}

void draw_text(LameState *state, const char *text, int x, int y, Color color)
{
    DrawTextEx(state->font, text, (Vector2){x, y}, (float)state->font_size, 1.0f, color);
}

void draw_cursor(LameState *state)
{
    char ch = state->lines[state->line][state->cursor - 1];
    GlyphInfo glyph = GetGlyphInfo(state->font, ch);

    Rectangle cursor_rec = { state->cursor*(glyph.advanceX + 1), state->line*state->font_size, glyph.advanceX, 20.0f };
    DrawRectangleRec(cursor_rec, Fade(BLACK, 0.5f));
}
