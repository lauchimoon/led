#include "notification.h"

#include <stdio.h>

void notify(LedState *state, const char *msg)
{
    snprintf(state->notification, NOTIFICATION_MAX_LEN, "%s", msg);
    state->notification_end_time = GetTime() + NOTIFICATION_DURATION;
}

void draw_notification(LedState *state)
{
    if (state->notification[0] == '\0')
        return;

    double time_left = state->notification_end_time - GetTime();
    if (time_left <= 0.0)
        return;

    float pad = 8.0f;
    float margin = 12.0f;
    Vector2 text_size = MeasureTextEx(state->font, state->notification,
                                      state->font_size, 1.0f);
    Rectangle notify_box = {
        GetScreenWidth() - text_size.x - 2*pad - margin,
        GetScreenHeight() - text_size.y - 2*pad - margin,
        text_size.x + 2*pad, text_size.y + 2*pad
    };

    DrawRectangleRec(notify_box, state->theme.hud_color);
    DrawRectangleLinesEx(notify_box, 1.0f, state->theme.text_color);
    DrawTextEx(state->font, state->notification,
               (Vector2){ notify_box.x + pad, notify_box.y + pad },
               state->font_size, 1.0f, state->theme.text_color);
}
