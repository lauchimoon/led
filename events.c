#include "events.h"

#include "keybinds.h"
#include "line_ops.h"
#include "undo.h"
#include "utils.h"

#include <stdio.h>

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
        else if (IsKeyDown(KEY_T)) {
            if (IsKeyPressed(KEY_ONE))
                state->theme = themes[0];
            else if (IsKeyPressed(KEY_TWO))
                state->theme = themes[1];
        }
    } else {
        if (key_pressed_or_repeat(KEY_BACKSPACE) && state->cursor > 0)
            delete_char_cursor(state, true);

        if (IsKeyPressed(KEY_TAB))
            append_tab(state);

        if (state->cursor < 0)
            state->cursor = 0;

        int c = GetKeyPressed();
        while (c > 0) {
            if (c == KEY_ENTER)
                new_line(state);
            else if (c >= ' ' && c <= '~')
                append_char_cursor(state, c, true);
            c = GetCharPressed();
        }
    }
}
