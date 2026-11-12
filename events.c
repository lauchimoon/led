#include "events.h"

#include "keybinds.h"
#include "line_ops.h"
#include "utils.h"
#include "notification.h"

#include <string.h>
#include <stdio.h>

static void handle_goto_line_prompt(LedState *state)
{
    if (IsKeyPressed(KEY_ESCAPE)) {
        state->goto_line_active = false;
        return;
    }

    if (IsKeyPressed(KEY_ENTER)) {
        state->goto_line_active = false;
        long long line = 0;
        if (!isint(state->goto_line_input, &line)) {
            notify(state, "invalid line number");
            return;
        }

        if (line <= 0)
            state->line = 0;
        else if (line >= state->lines_num)
            state->line = state->lines_num - 1;
        else
            state->line = line - 1;

        // TODO: should preserve cursor position
        state->cursor = 0;
        return;
    }

    if (key_pressed_or_repeat(KEY_BACKSPACE)) {
        int len = strlen(state->goto_line_input);
        if (len > 0)
            state->goto_line_input[len - 1] = '\0';
    }

    int len = strlen(state->goto_line_input);
    int c;
    while ((c = GetCharPressed()) > 0) {
        if (c < '0' || c > '9' || len >= GOTO_LINE_MAX_LEN)
            continue;
        state->goto_line_input[len++] = c;
    }
    state->goto_line_input[len] = '\0';
}

void handle_editor_events(LedState *state)
{
    if (state->goto_line_active) {
        handle_goto_line_prompt(state);
        return;
    }

    if (IsKeyDown(KEY_LEFT_CONTROL)) {
        if (IsKeyPressed(KEY_Q))
            state->exit = true;
        else if (state->fileperm == FILEPERM_RW && IsKeyPressed(KEY_D))
            delete_line(state);
        else if (IsKeyPressed(KEY_S))
            write_file(state);
        else if (IsKeyPressed(KEY_K))
            resize_font(state, RESIZE_ACTION_INCREASE);
        else if (IsKeyPressed(KEY_J))
            resize_font(state, RESIZE_ACTION_DECREASE);
        else if (IsKeyDown(KEY_T)) {
            if (IsKeyPressed(KEY_ONE))
                state->theme = themes[0];
            else if (IsKeyPressed(KEY_TWO))
                state->theme = themes[1];
        } else if (IsKeyPressed(KEY_C))
            copy_line(state);
        else if (state->fileperm == FILEPERM_RW && IsKeyPressed(KEY_V))
            paste_line(state);
        else if (IsKeyPressed(KEY_N))
            state->number = !state->number;
        else if (IsKeyPressed(KEY_G)) {
            state->goto_line_active = true;
            state->goto_line_input[0] = '\0';
            return;
        } else if (IsKeyPressed(KEY_PAGE_UP) && state->lines_num > 0) {
            state->line = 0;
            state->cursor = 0;
        } else if (IsKeyPressed(KEY_PAGE_DOWN) && state->lines_num > 0) {
            state->line = state->lines_num - 1;
            state->cursor = strlen(state->lines[state->line]);
        }
    }

    if (state->fileperm == FILEPERM_RW && key_pressed_or_repeat(KEY_BACKSPACE)) {
        int curs = state->cursor;
        if (curs > 0)
            delete_char_cursor(state);
        else
            if (state->line > 0)
                move_line_above(state);
    }

    if (state->fileperm == FILEPERM_RW && IsKeyPressed(KEY_TAB))
        append_tab(state);

    if (state->fileperm == FILEPERM_RW && key_pressed_or_repeat(KEY_ENTER))
        split_line(state);

    if (state->cursor < 0)
        state->cursor = 0;

    int c;
    while ((c = GetCharPressed()) > 0) {
        if (state->fileperm == FILEPERM_RW && c >= ' ' && c <= '~')
            append_char_cursor(state, c);
    }
}
