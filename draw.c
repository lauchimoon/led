#include "draw.h"

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
