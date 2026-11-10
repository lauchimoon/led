#include "draw.h"
#include "utils.h"

#include <string.h>

void draw_text(LedState *state, const char *text, int x, int y, Color color)
{
    DrawTextEx(state->font, text, (Vector2){x, y}, (float)state->font_size, 1.0f, color);
}

void draw_cursor(LedState *state)
{
    int cursor_idx = (state->cursor > 0)? state->cursor - 1 : 0;
    char ch = state->lines[state->line][cursor_idx];
    GlyphInfo glyph = GetGlyphInfo(state->font, ch);

    Rectangle cursor_rec = { state->cursor*(glyph.advanceX + 1), state->line*state->font_size, glyph.advanceX, state->font_size };
    DrawRectangleRec(cursor_rec, Fade(state->theme.text_color, 0.5f));
}

void draw_line_numbers(LedState *state)
{
    if (!state->number)
        return;

    int char_width = get_char_width(state);
    int number_column_width = get_number_column_width(state);
    int line_height = state->font_size;
    int screen_height = GetScreenHeight();

    DrawRectangle(0, 0, number_column_width, screen_height, state->theme.background_color);

    int scroll_y = state->camera.target.y;
    int first_line = scroll_y/line_height;
    if (first_line < 0)
        first_line = 0;

    int last_line = first_line + screen_height/line_height + 2;
    if (last_line > state->lines_num)
        last_line = state->lines_num;

    for (int i = first_line; i < last_line; ++i) {
        const char *number = TextFormat("%d", i + 1);
        int number_len = strlen(number);
        int x = number_column_width - (number_len + 1)*char_width;
        int y = i*line_height - scroll_y;
        draw_text(state, number, x, y, Fade(state->theme.text_color, 0.5f));
    }
}

void draw_hud(LedState *state)
{
    DrawRectangle(0, GetScreenHeight() - state->font_size, GetScreenWidth(), state->font_size, state->theme.hud_color);
    const char *line_information = TextFormat("%d:%d", state->line + 1, state->cursor + 1);
    const char *info = TextFormat((state->dirty? "%s [*] | %s" : "%s | %s"),
                                  state->filename, line_information);
    const char *text = TextFormat("%s%s", info, (state->fileperm != FILEPERM_RW)? " | Read-only" : "");
    draw_text(state, text, 0, GetScreenHeight() - state->font_size, state->theme.text_color);
}

void draw_goto_line_prompt(LedState *state)
{
    if (!state->goto_line_active)
        return;

    const char *prompt = TextFormat("go to line: %s", state->goto_line_input);
    Vector2 text_size = MeasureTextEx(state->font, prompt, state->font_size, 1.0f);

    float pad = 8.0f;
    float margin = 12.0f;
    Rectangle prompt_box = {
        margin,
        GetScreenHeight() - text_size.y - 2*pad - margin,
        text_size.x + 2*pad,
        text_size.y + 2*pad
    };

    DrawRectangleRec(prompt_box, state->theme.hud_color);
    DrawRectangleLinesEx(prompt_box, 1.0f, state->theme.text_color);
    DrawTextEx(state->font, prompt,
               (Vector2){ prompt_box.x + pad, prompt_box.y + pad },
               state->font_size, 1.0f, state->theme.text_color);
}
