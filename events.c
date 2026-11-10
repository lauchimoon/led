#include "events.h"

#include "keybinds.h"
#include "line_ops.h"
#include "utils.h"

#include <string.h>
#include <stdio.h>

void handle_editor_events(LedState *state)
{
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
        else if (IsKeyPressed(KEY_G) && state->lines_num > 0) {
            state->line = 0;
            state->cursor = 0;
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
