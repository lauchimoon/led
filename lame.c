#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#define WINDOW_WIDTH    800
#define WINDOW_HEIGHT   600
#define FPS             60

#define LINE_SIZE       1024

#define REPEAT_COOLDOWN 3

typedef char *Line;

typedef struct LameState {
    const char *title;
    const char *filename;
    bool exit;

    Line *lines;
    int lines_capacity;
    int lines_num;

    int line;
    int cursor;
    int repeat_cooldown;

    Font font;
    int font_size;

    bool dirty;
} LameState;

void state_init(LameState *, const char *);
void load_file(LameState *);
void state_deinit(LameState *);

bool any_key_pressed(int *);

void handle_editor_events(LameState *);
void handle_cursor_movement(LameState *);

void new_line(LameState *);
void delete_char_cursor(LameState *);
void append_char_cursor(LameState *, int);
void delete_line(LameState *);
void append_tab(LameState *);
void write_file(LameState *);

void draw_text(LameState *, const char *, int, int, Color);
void draw_cursor(LameState *);
void draw_hud(LameState *);

int main(int argc, char **argv)
{
    if (argc < 2) {
        printf("usage: lame <file>\n");
        return 1;
    }

    const char *filename = argv[1];

    LameState state = {
        .title = TextFormat("lame - %s", filename),
        .exit  = false,
    };

    SetTraceLogLevel(LOG_NONE);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, state.title);
    SetWindowMinSize(WINDOW_WIDTH/2, WINDOW_HEIGHT/2);

    state_init(&state, filename);

    SetTargetFPS(FPS);

    while (!state.exit) {
        ++state.repeat_cooldown;
        state.repeat_cooldown %= REPEAT_COOLDOWN;

        handle_editor_events(&state);

        handle_cursor_movement(&state);

        BeginDrawing();
        ClearBackground(RAYWHITE);
        for (int i = 0; i < state.lines_num; ++i)
            draw_text(&state, state.lines[i], 0, i*state.font_size, BLACK);
        draw_cursor(&state);
        draw_hud(&state);
        EndDrawing();
    }

    state_deinit(&state);
    CloseWindow();
    return 0;
}

void state_init(LameState *state, const char *filename)
{
    state->filename = filename;

    state->cursor = 0;
    state->repeat_cooldown = 0;

    state->font_size = 24;
    state->font = LoadFontEx("fonts/GeistMono-Regular.ttf", state->font_size, NULL, 95);

    state->line = -1;
    state->lines_capacity = 100;
    state->lines = calloc(state->lines_capacity, sizeof(Line));

    if (FileExists(state->filename)) {
        load_file(state);
        state->line = 0;
        return;
    }

    new_line(state);
}

void load_file(LameState *state)
{
    FILE *f = fopen(state->filename, "r");
    char buffer[LINE_SIZE] = { 0 };

    while (fgets(buffer, LINE_SIZE, f) != NULL) {
        state->lines[state->lines_num] = calloc(LINE_SIZE, sizeof(char));
        int line_len = strlen(buffer);
        strncpy(state->lines[state->lines_num], buffer, line_len - 1);

        ++state->lines_num;
        if (state->lines_num >= state->lines_capacity/2) {
            state->lines_capacity *= 2;
            state->lines = realloc(state->lines, state->lines_capacity*sizeof(Line));
        }
    }

    fclose(f);
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
    if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_Q))
        state->exit = true;

    if (IsKeyDown(KEY_BACKSPACE) && state->cursor > 0 &&
            state->repeat_cooldown % REPEAT_COOLDOWN == 0)
        delete_char_cursor(state);

    if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_D))
        delete_line(state);

    if (IsKeyPressed(KEY_TAB))
        append_tab(state);

    if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_S))
        write_file(state);

    if (state->cursor < 0)
        state->cursor = 0;

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
    int current_line_len = strlen(state->lines[state->line]);

    if (IsKeyDown(KEY_LEFT) && state->repeat_cooldown % REPEAT_COOLDOWN == 0)
        state->cursor -= state->cursor > 0? 1 : 0;
    else if (IsKeyDown(KEY_RIGHT) && state->repeat_cooldown % REPEAT_COOLDOWN == 0)
        state->cursor += state->cursor < current_line_len? 1 : 0;
    else if (IsKeyDown(KEY_UP) && state->repeat_cooldown % REPEAT_COOLDOWN == 0) {
        if (state->line > 0) {
            int line_above_len = strlen(state->lines[state->line - 1]);
            --state->line;

            if (state->cursor > line_above_len)
                state->cursor = line_above_len;
        } else
            state->cursor = 0;
    } else if (IsKeyDown(KEY_DOWN) && state->repeat_cooldown % REPEAT_COOLDOWN == 0) {
        if (state->lines[state->line + 1]) {
            int line_below_len = strlen(state->lines[state->line + 1]);
            ++state->line;

            if (state->cursor > line_below_len)
                state->cursor = line_below_len;
        } else
            state->cursor = strlen(state->lines[state->line]);
    }
}

void new_line(LameState *state)
{
    Line new = calloc(LINE_SIZE, sizeof(char));
    ++state->lines_num;

    for (int i = state->lines_num; i > state->line; --i)
        state->lines[i] = state->lines[i - 1];

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
    Line line = state->lines[state->line];
    int line_len = strlen(line);
    if (line_len < 1)
        return;

    for (int i = state->cursor - 1; i < line_len; ++i)
        line[i] = line[i + 1];

    --state->cursor;
    state->dirty = true;
}

void append_char_cursor(LameState *state, int c)
{
    Line line = state->lines[state->line];
    int line_len = strlen(line);

    for (int i = line_len; i > state->cursor; --i)
        line[i] = line[i - 1];

    line[state->cursor++] = c;
    state->dirty = true;
}

void delete_line(LameState *state)
{
    Line line = state->lines[state->line];
    if (state->lines_num == 1) {
        memset(line, 0, strlen(line));
    } else {
        free(line);
        for (int i = state->line; i < state->lines_num; ++i)
            state->lines[i] = state->lines[i + 1];

        --state->lines_num;
        state->line -= state->line > 0? 1 : 0;
    }

    state->cursor = 0;
    state->dirty = true;
}

void append_tab(LameState *state)
{
    for (int i = 0; i < 4; ++i)
        append_char_cursor(state, ' ');

    state->dirty = true;
}

void write_file(LameState *state)
{
    state->dirty = false;
    FILE *f = fopen(state->filename, "w");
    for (int i = 0; i < state->lines_num; ++i)
        fprintf(f, "%s\n", state->lines[i]);

    fclose(f);
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

void draw_hud(LameState *state)
{
    draw_text(state, TextFormat((state->dirty? "%s [*]" : "%s"), state->filename), 0, GetScreenHeight() - state->font_size, BLACK);
}
