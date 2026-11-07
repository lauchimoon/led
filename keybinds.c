#include "keybinds.h"

#include "utils.h"

#include <string.h>

static int get_number_lines_on_screen(LedState *state)
{
    int num_lines = GetScreenHeight()/state->font_size;
    return num_lines - 1;
}

void handle_cursor_movement(LedState *state)
{
    int current_line_len = strlen(state->lines[state->line]);

    if (key_pressed_or_repeat(KEY_LEFT))
        state->cursor -= state->cursor > 0? 1 : 0;
    else if (key_pressed_or_repeat(KEY_RIGHT))
        state->cursor += state->cursor < current_line_len? 1 : 0;
    else if (key_pressed_or_repeat(KEY_UP)) {
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
    } else if (key_pressed_or_repeat(KEY_DOWN)) {
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

void move_to_start(LedState *state)
{
    state->cursor = 0;
}

void move_to_end(LedState *state)
{
    int line_len = strlen(state->lines[state->line]);
    state->cursor = line_len;
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

    state_load_font(state);
}
