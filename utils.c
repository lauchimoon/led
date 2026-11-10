#include "utils.h"
#include "raylib.h"

#include <limits.h>

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

bool isint(const char *s, long long *result)
{
    if (!s || !result || *s == '\0')
        return false;

    int i = 0;
    int sign = 1;

    // Optional sign
    if (s[0] == '-') {
        sign = -1;
        ++i;
    } else if (s[0] == '+')
        ++i;

    // String must have digits
    if (s[i] == '\0')
        return false;

    long long res = 0;

    while (s[i] != '\0') {
        if (s[i] < '0' || s[i] > '9')
            return false;

        int digit = s[i] - '0';
        if (sign == 1) {
            if (res > (LLONG_MAX - digit) / 10)
                return false; // Overflow
            res = res * 10 + digit;
        } else {
            if (res < (LLONG_MIN + digit) / 10)
                return false; // Underflow
            res = res * 10 - digit;
        }
        ++i;
    }

    *result = res;
    return true;
}
