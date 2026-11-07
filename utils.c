#include "utils.h"

#include "raylib.h"

bool key_pressed_or_repeat(int key)
{
    return IsKeyPressed(key) || IsKeyPressedRepeat(key);
}
