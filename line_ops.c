#include "line_ops.h"
#include "notification.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void new_line(LedState *state)
{
    Line new = calloc(LINE_SIZE, sizeof(char));
    ++state->lines_num;

    for (int i = state->lines_num; i > state->line + 1; --i)
        state->lines[i] = state->lines[i - 1];

    ++state->line;
    if (state->lines_num >= state->lines_capacity/2) {
        state->lines_capacity *= 2;
        state->lines = realloc(state->lines, state->lines_capacity*sizeof(Line));
    }

    state->lines[state->line] = new;
    state->cursor = 0;
    state->dirty = true;
}

void split_line(LedState *state)
{
    Line line = state->lines[state->line];
    int line_len = strlen(line);

    if (state->cursor < 0)
        state->cursor = 0;
    if (state->cursor > line_len)
        state->cursor = line_len;

    int cur = state->cursor;
    new_line(state);
    Line new = state->lines[state->line];
    strcpy(new, line + cur);
    line[cur] = '\0';
}

void delete_char_cursor(LedState *state)
{
    Line line = state->lines[state->line];
    int line_len = strlen(line);
    if (line_len < 1)
        return;

    if (state->cursor <= 0)
        return;

    for (int i = state->cursor - 1; i < line_len; ++i)
        line[i] = line[i + 1];

    --state->cursor;
    state->dirty = true;
}

void append_char_cursor(LedState *state, int c)
{
    Line line = state->lines[state->line];
    int line_len = strlen(line);

    if (line_len >= LINE_SIZE - 1)
        return;

    if (state->cursor < 0 || state->cursor > line_len)
        return;

    for (int i = line_len; i > state->cursor; --i)
        line[i] = line[i - 1];

    line[state->cursor++] = c;
    state->dirty = true;
}

void delete_line(LedState *state)
{
    if (state->line < 0 || state->line >= state->lines_num)
        return;

    Line line = state->lines[state->line];
    if (!line)
        return;

    if (state->lines_num == 1) {
        memset(line, 0, strlen(line));
    } else {
        free(line);
        for (int i = state->line; i < state->lines_num - 1; ++i)
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
        append_char_cursor(state, ' ');

    state->dirty = true;
}

void copy_line(LedState *state)
{
    memset(state->copy_buffer, 0, LINE_SIZE);
    if (!state->lines[state->line])
        return;

    int n = strlen(state->lines[state->line]);
    if (n > LINE_SIZE - 1)
        n = LINE_SIZE - 1;
    strncpy(state->copy_buffer, state->lines[state->line], n);
    notify(state, "copied line");
}

void paste_line(LedState *state)
{
    if (!state->copy_buffer)
        return;

    new_line(state);
    int n = strlen(state->copy_buffer);
    if (n > LINE_SIZE - 1)
        n = LINE_SIZE - 1;
    strncpy(state->lines[state->line], state->copy_buffer, n);
    notify(state, "pasted line");
}

void move_line_above(LedState *state)
{
    if (state->line <= 0 || state->line >= state->lines_num)
        return;

    Line line = state->lines[state->line];
    Line prev = state->lines[state->line - 1];
    if (!line || !prev)
        return;

    int prev_len = strlen(prev);
    int line_len = strlen(line);

    // Only join lines if the concatenation fits in a single line
    if (prev_len + line_len >= LINE_SIZE) {
        notify(state, "cannot join lines: line would be too long");
        return;
    }

    int new_curs = prev_len;
    memcpy(prev + prev_len, line, line_len + 1);
    delete_line(state);
    state->cursor = new_curs;
}
