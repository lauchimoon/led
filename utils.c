#include "utils.h"
#include "raylib.h"

static int digit_count(int n)
{
    int digits = 1;
    while (n >= 10) {
        n /= 10;
        ++digits;
    }
    return digits;
}

int get_char_width(LedState *state)
{
    // Using 'M' as a guide; we only care about the advanceX field.
    return GetGlyphInfo(state->font, 'M').advanceX + 1;
}

int get_number_column_width(LedState *state)
{
    if (!state->number)
        return 0;

    int digits = digit_count(state->lines_num);
    return (digits + 2)*get_char_width(state);
}

bool key_pressed_or_repeat(int key)
{
    return IsKeyPressed(key) || IsKeyPressedRepeat(key);
}
