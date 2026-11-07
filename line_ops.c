#include "line_ops.h"

#include "undo.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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

void copy_line(LedState *state)
{
    memset(state->copy_buffer, 0, LINE_SIZE);
    int n = strlen(state->lines[state->line]);
    strncpy(state->copy_buffer, state->lines[state->line], n);
}

void paste_line(LedState *state)
{
    new_line(state);
    int n = strlen(state->copy_buffer);
    strncpy(state->lines[state->line], state->copy_buffer, n);
}
