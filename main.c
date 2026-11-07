#include "raylib.h"

#include "draw.h"
#include "events.h"
#include "keybinds.h"
#include "state.h"

#include <stdio.h>

#define WINDOW_WIDTH    800
#define WINDOW_HEIGHT   600
#define FPS             60

int main(int argc, char **argv)
{
    if (argc < 2) {
        printf("usage: led <file>\n");
        return 1;
    }

    const char *filename = argv[1];

    LedState state = {
        .title = TextFormat("led - %s", filename),
        .exit  = false,
    };

    SetTraceLogLevel(LOG_NONE);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, state.title);
    SetWindowMinSize(WINDOW_WIDTH/2, WINDOW_HEIGHT/2);

    state_init(&state, filename);

    SetTargetFPS(FPS);

    while (!state.exit) {
        handle_editor_events(&state);

        handle_cursor_movement(&state);

        BeginDrawing();
        ClearBackground(state.theme.background_color);

        BeginMode2D(state.camera);
            for (int i = 0; i < state.lines_num; ++i)
                draw_text(&state, state.lines[i], 0, i*state.font_size, state.theme.text_color);
            draw_cursor(&state);
        EndMode2D();

        draw_hud(&state);
        EndDrawing();
    }

    state_deinit(&state);
    CloseWindow();
    return 0;
}
