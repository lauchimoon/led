#include "state.h"
#include "notification.h"

#include "fonts/GeistMono-Regular.h"
#include "line_ops.h"

#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int get_file_permissions(const char *filename)
{
    bool is_write = (access(filename, W_OK) == 0);
    bool is_read = (access(filename, R_OK) == 0);

    if (is_write && is_read)
        return FILEPERM_RW;
    else if (!is_write && is_read)
        return FILEPERM_RO;
    else if (is_write && !is_read)
        return FILEPERM_WO;
    else
        return FILEPERM_NO;
}

int state_init(LedState *state, const char *filename)
{
    state->filename = filename;

    state->cursor = 0;

    state->font_size = FONT_SIZE_INIT;
    state->font = (Font){ 0 };
    state_load_font(state);
    state->theme = themes[0];

    state->line = -1;
    state->lines_capacity = 100;
    state->lines_num = 0;
    state->lines = calloc(state->lines_capacity, sizeof(Line));

    state->camera.offset = (Vector2){ 0.0f, 0.0f };
    state->camera.target = (Vector2){ 0.0f, 0.0f };
    state->camera.rotation = 0.0f;
    state->camera.zoom = 1.0f;

    state->copy_buffer = calloc(LINE_SIZE, sizeof(char));
    state->dirty = false;
    state->number = false;

    state->goto_line_active = false;
    state->goto_line_input[0] = '\0';

    if (FileExists(state->filename)) {
        state->fileperm = get_file_permissions(state->filename);
        int res = load_file(state);
        if (res != 0) {
            printf("led: could not open '%s' for reading\n", state->filename);
            return 1;
        }

        if (state->lines_num == 0) {
            if (!new_line(state)) {
                printf("led: could not allocate memory\n");
                return 1;
            }
        } else
            state->line = 0;
        return 0;
    }

    state->fileperm = FILEPERM_RW;
    if (!new_line(state)) {
        printf("led: could not allocate memory\n");
        return 1;
    }
    return 0;
}

bool state_reserve_lines(LedState *state, long long required)
{
    if (required < 0 || required > INT_MAX)
        return false;

    int old_capacity = state->lines_capacity;
    if (old_capacity < 0 || !state->lines)
        old_capacity = 0;

    if (required <= old_capacity)
        return true;

    if ((size_t)required > SIZE_MAX / sizeof(Line))
        return false;

    long long new_capacity = old_capacity > 0 ? old_capacity : 1;
    while (new_capacity < required) {
        if (new_capacity > INT_MAX / 2) {
            new_capacity = required;
            break;
        }
        new_capacity *= 2;
    }

    // Keep the original pointer until realloc succeeds.
    Line *new_lines = realloc(state->lines, (size_t)new_capacity * sizeof(Line));
    if (!new_lines)
        return false;

    // Initialize because realloc doesn't do so.
    memset(new_lines + old_capacity, 0,
           (size_t)(new_capacity - old_capacity) * sizeof(Line));

    state->lines = new_lines;
    state->lines_capacity = (int)new_capacity;
    return true;
}

static void discard_lines(LedState *state, long long first_line)
{
    while (state->lines_num > first_line) {
        --state->lines_num;
        free(state->lines[state->lines_num]);
        state->lines[state->lines_num] = NULL;
    }
}

int load_file(LedState *state)
{
    FILE *f = fopen(state->filename, "r");
    if (!f)
        return 1;

    long long first_line = state->lines_num;
    char buffer[LINE_SIZE] = { 0 };

    while (fgets(buffer, LINE_SIZE, f) != NULL) {
        // Reserve room for the new line and NULL
        if (!state_reserve_lines(state, state->lines_num + 2)) {
            discard_lines(state, first_line);
            fclose(f);
            return 1;
        }

        Line line = calloc(LINE_SIZE, sizeof(char));
        if (!line) {
            discard_lines(state, first_line);
            fclose(f);
            return 1;
        }

        int line_len = strlen(buffer);
        if (line_len > 0 && buffer[line_len - 1] == '\n')
            --line_len;
        memcpy(line, buffer, line_len);

        state->lines[state->lines_num] = line;
        ++state->lines_num;
        state->lines[state->lines_num] = NULL;
    }

    fclose(f);
    return 0;
}

void write_file(LedState *state)
{
    FILE *f = fopen(state->filename, "w");
    if (!f) {
        notify(state, "could not save; file is read-only");
        return;
    }

    for (int i = 0; i < state->lines_num; ++i)
        fprintf(f, "%s\n", state->lines[i]);

    fclose(f);
    notify(state, "saved file");
    state->dirty = false;
}

void state_load_font(LedState *state)
{
    if (state->font.texture.id != 0)
        UnloadFont(state->font);

    state->font = LoadFontFromMemory(".ttf", GeistMono_Regular_ttf, GeistMono_Regular_ttf_len, state->font_size, NULL, 95);
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

    free(state->copy_buffer);
}
