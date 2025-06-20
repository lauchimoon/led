#include "raylib.h"

#include "theme.h"
#include "fonts/GeistMono-Regular.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#define WINDOW_WIDTH    800
#define WINDOW_HEIGHT   600
#define FPS             60

#define LINE_SIZE       1024

#define REPEAT_COOLDOWN 3

#define FONT_SIZE_INIT     24
#define FONT_RESIZE_FACTOR 4
#define FONT_RESIZE_MIN    FONT_SIZE_INIT/2
#define FONT_RESIZE_MAX    FONT_SIZE_INIT*2

enum {
    UNDO_ACTION_DELETE_CHAR = 0,
    UNDO_ACTION_APPEND_CHAR,
};

enum {
    RESIZE_ACTION_INCREASE = 0,
    RESIZE_ACTION_DECREASE,
};

typedef char *Line;

typedef struct UndoAction {
    int type;
    int line;
    int cursor;
    char ch;
} UndoAction;

typedef struct _UndoBuffer {
    UndoAction action;
    struct _UndoBuffer *next;
} UndoBuffer;

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
    int repeat_cooldown;

    Font font;
    int font_size;

    bool dirty;

    Camera2D camera;

    UndoBuffer *undo;
} LedState;

UndoBuffer *undo_init(void);
UndoBuffer *undo_append(UndoBuffer *, UndoAction);
UndoBuffer *undo_delete(UndoBuffer *);
void undo_free(UndoBuffer *);

void state_init(LedState *, const char *);
void load_file(LedState *);
void state_deinit(LedState *);

bool any_key_pressed(int *);

void handle_editor_events(LedState *);
void handle_cursor_movement(LedState *);
int get_number_lines_on_screen(LedState *);

void new_line(LedState *);
void delete_char_cursor(LedState *, bool);
void append_char_cursor(LedState *, int, bool);
void delete_line(LedState *);
void append_tab(LedState *);
void write_file(LedState *);
void undo(LedState *);
void resize_font(LedState *, int action);

void move_to_start(LedState *);
void move_to_end(LedState *);

void draw_text(LedState *, const char *, int, int, Color);
void draw_cursor(LedState *);
void draw_hud(LedState *);

int main(int argc, char **argv)
{
    if (argc < 2) {
        printf("usage: led <file>\n");
        return 1;
    }

    const char *filename = argv[1];

    LedState state = {
        .title = TextFormat("led - %s", filename),
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
        ClearBackground(state.theme.background_color);

        BeginMode2D(state.camera);
            for (int i = 0; i < state.lines_num; ++i)
                draw_text(&state, state.lines[i], 0, i*state.font_size, state.theme.text_color);
            draw_cursor(&state);
        EndMode2D();

        draw_hud(&state);
        EndDrawing();
    }

    state_deinit(&state);
    CloseWindow();
    return 0;
}

UndoBuffer *undo_init(void)
{
    return NULL;
}

UndoBuffer *undo_append(UndoBuffer *head, UndoAction action)
{
    UndoBuffer *node = malloc(sizeof(UndoBuffer));
    node->action = action;
    node->next = head;
    return node;
}

UndoBuffer *undo_delete(UndoBuffer *head)
{
    if (!head)
      return NULL;

    UndoBuffer *node = head;
    head = head->next;
    free(node);
    return head;
}

void undo_free(UndoBuffer *head)
{
    UndoBuffer *node = head;
    while (node) {
        UndoBuffer *tmp = node;
        node = node->next;
        free(tmp);
    }
}

void state_init(LedState *state, const char *filename)
{
    state->filename = filename;

    state->cursor = 0;
    state->repeat_cooldown = 0;

    state->font_size = FONT_SIZE_INIT;
    state->font = LoadFontFromMemory(".ttf", GeistMono_Regular_ttf, GeistMono_Regular_ttf_len, state->font_size, NULL, 95);
    state->theme = themes[0];

    state->line = -1;
    state->lines_capacity = 100;
    state->lines = calloc(state->lines_capacity, sizeof(Line));
    state->line_scroll = 1;

    state->camera.offset = (Vector2){ 0.0f, 0.0f };
    state->camera.target = (Vector2){ 0.0f, 0.0f };
    state->camera.rotation = 0.0f;
    state->camera.zoom = 1.0f;

    state->undo = undo_init();

    if (FileExists(state->filename)) {
        load_file(state);
        state->line = 0;
        return;
    }

    new_line(state);
}

void load_file(LedState *state)
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

void state_deinit(LedState *state)
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

    undo_free(state->undo);
}

bool any_key_pressed(int *key)
{
    *key = GetKeyPressed();
    return *key >= ' ' && *key <= KEY_KB_MENU;
}

void handle_editor_events(LedState *state)
{
    if (IsKeyDown(KEY_LEFT_CONTROL)) {
        if (IsKeyPressed(KEY_Q))
            state->exit = true;
        else if (IsKeyPressed(KEY_D))
            delete_line(state);
        else if (IsKeyPressed(KEY_S))
            write_file(state);
        else if (IsKeyPressed(KEY_Z))
            undo(state);
        else if (IsKeyPressed(KEY_K))
            resize_font(state, RESIZE_ACTION_INCREASE);
        else if (IsKeyPressed(KEY_J))
            resize_font(state, RESIZE_ACTION_DECREASE);
        else if (IsKeyDown(KEY_T))
            if (IsKeyPressed(KEY_ONE))
                state->theme = themes[0];
            else if (IsKeyPressed(KEY_TWO))
                state->theme = themes[1];
    }

    if (IsKeyDown(KEY_BACKSPACE) && state->cursor > 0 &&
            state->repeat_cooldown % REPEAT_COOLDOWN == 0)
        delete_char_cursor(state, true);

    if (IsKeyPressed(KEY_TAB))
        append_tab(state);

    if (state->cursor < 0)
        state->cursor = 0;

    int key;
    if (any_key_pressed(&key)) {
        int c = GetCharPressed();
        if (key == KEY_ENTER)
            new_line(state);
        else if (c >= ' ' && c <= '~')
            append_char_cursor(state, c, true);
    }
}

void handle_cursor_movement(LedState *state)
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

            --state->line_scroll;
            if (state->line_scroll <= 0) {
                state->camera.target.y -= state->font_size;
                state->line_scroll = 1;
            }

            if (state->cursor > line_above_len)
                state->cursor = line_above_len;
        } else
            state->cursor = 0;
    } else if (IsKeyDown(KEY_DOWN) && state->repeat_cooldown % REPEAT_COOLDOWN == 0) {
        if (state->lines[state->line + 1]) {
            int line_below_len = strlen(state->lines[state->line + 1]);
            ++state->line;
            ++state->line_scroll;

            if (state->line_scroll > get_number_lines_on_screen(state)) {
                state->camera.target.y += state->font_size;
                --state->line_scroll;
            }

            if (state->cursor > line_below_len)
                state->cursor = line_below_len;
        } else
            state->cursor = strlen(state->lines[state->line]);
    }

    if (IsKeyPressed(KEY_PAGE_DOWN)) {
        int lines_on_screen = get_number_lines_on_screen(state);
        state->line_scroll = 1;
        state->line += lines_on_screen;

        if (state->line > state->lines_num)
            state->line = state->lines_num - 1;

        state->cursor = strlen(state->lines[state->line]);
        state->camera.target.y = state->font_size*state->line;
    }

    if (IsKeyPressed(KEY_PAGE_UP)) {
        int lines_on_screen = get_number_lines_on_screen(state);
        state->line_scroll = 1;
        state->line -= lines_on_screen;

        if (state->line < 0)
            state->line = 0;

        state->cursor = strlen(state->lines[state->line]);
        state->camera.target.y = state->font_size*state->line;
    }

    if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_ZERO))
        move_to_start(state);

    if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_E))
        move_to_end(state);
}

int get_number_lines_on_screen(LedState *state)
{
    int num_lines = GetScreenHeight()/state->font_size;
    return num_lines - 1;
}

void new_line(LedState *state)
{
    Line new = calloc(LINE_SIZE, sizeof(char));
    ++state->lines_num;

    for (int i = state->lines_num; i > state->line; --i)
        state->lines[i] = state->lines[i - 1];

    ++state->line;
    ++state->line_scroll;
    if (state->lines_num >= state->lines_capacity/2) {
        state->lines_capacity *= 2;
        state->lines = realloc(state->lines, state->lines_capacity*sizeof(Line));
    }

    state->lines[state->line] = new;
    state->cursor = 0;
}

void delete_char_cursor(LedState *state, bool undo)
{
    Line line = state->lines[state->line];
    int line_len = strlen(line);
    if (line_len < 1)
        return;

    if (undo) {
        UndoAction action = {
            .type = UNDO_ACTION_DELETE_CHAR,
            .line = state->line,
            .cursor = state->cursor,
            .ch = line[state->cursor - 1],
        };
        state->undo = undo_append(state->undo, action);
    }

    for (int i = state->cursor - 1; i < line_len; ++i)
        line[i] = line[i + 1];

    --state->cursor;
    state->dirty = true;
}

void append_char_cursor(LedState *state, int c, bool undo)
{
    Line line = state->lines[state->line];
    int line_len = strlen(line);

    if (undo) {
        UndoAction action = {
            .type = UNDO_ACTION_APPEND_CHAR,
            .line = state->line,
            .cursor = state->cursor,
            .ch = c,
        };
        state->undo = undo_append(state->undo, action);
    }

    for (int i = line_len; i > state->cursor; --i)
        line[i] = line[i - 1];

    line[state->cursor++] = c;
    state->dirty = true;
}

void delete_line(LedState *state)
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

void append_tab(LedState *state)
{
    for (int i = 0; i < 4; ++i)
        append_char_cursor(state, ' ', true);

    state->dirty = true;
}

void write_file(LedState *state)
{
    state->dirty = false;
    FILE *f = fopen(state->filename, "w");
    for (int i = 0; i < state->lines_num; ++i)
        fprintf(f, "%s\n", state->lines[i]);

    fclose(f);
}

void undo(LedState *state)
{
    if (!state->undo)
        return;

    UndoAction action = state->undo->action;
    state->line = action.line;
    state->cursor = action.cursor - 1;

    switch (action.type) {
        case UNDO_ACTION_DELETE_CHAR:
            append_char_cursor(state, action.ch, false);
            break;
        case UNDO_ACTION_APPEND_CHAR:
            delete_char_cursor(state, false);
            break;
    }

    state->undo = undo_delete(state->undo);
}

void resize_font(LedState *state, int action)
{
    int sign = (action == RESIZE_ACTION_INCREASE)? 1 : -1;
    int resize_factor = sign*FONT_RESIZE_FACTOR;
    state->font_size += resize_factor;

    if (state->font_size < FONT_RESIZE_MIN) {
        state->font_size = FONT_RESIZE_MIN;
        return;
    }
    if (state->font_size > FONT_RESIZE_MAX) {
        state->font_size = FONT_RESIZE_MAX;
        return;
    }

    UnloadFont(state->font);
    state->font = LoadFontFromMemory(".ttf", GeistMono_Regular_ttf, GeistMono_Regular_ttf_len, state->font_size, NULL, 95);
}

void move_to_start(LedState *state)
{
    state->cursor = 0;
}

void move_to_end(LedState *state)
{
    int line_len = strlen(state->lines[state->line]);
    state->cursor = line_len;
}

void draw_text(LedState *state, const char *text, int x, int y, Color color)
{
    DrawTextEx(state->font, text, (Vector2){x, y}, (float)state->font_size, 1.0f, color);
}

void draw_cursor(LedState *state)
{
    char ch = state->lines[state->line][state->cursor - 1];
    GlyphInfo glyph = GetGlyphInfo(state->font, ch);

    Rectangle cursor_rec = { state->cursor*(glyph.advanceX + 1), state->line*state->font_size, glyph.advanceX, state->font_size };
    DrawRectangleRec(cursor_rec, Fade(state->theme.text_color, 0.5f));
}

void draw_hud(LedState *state)
{
    DrawRectangle(0, GetScreenHeight() - state->font_size, GetScreenWidth(), state->font_size, state->theme.hud_color);
    const char *line_information = TextFormat("%d:%d", state->line + 1, state->cursor + 1);
    const char *text = TextFormat((state->dirty? "%s [*] | %s" : "%s | %s"), state->filename, line_information);
    draw_text(state, text, 0, GetScreenHeight() - state->font_size, state->theme.text_color);
}
