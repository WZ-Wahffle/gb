#include "ppu.h"
#include "cpu.h"
#include "raylib.h"
#include "types.h"

void ui(void) {
    SetTraceLogLevel(LOG_ERROR);
    SetTargetFPS(60);
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "gb");
    void *framebuffer = calloc(VIEWPORT_WIDTH * VIEWPORT_HEIGHT * 4, 1);
    Image framebuffer_image = {framebuffer, VIEWPORT_WIDTH, VIEWPORT_HEIGHT, 1,
                               PIXELFORMAT_UNCOMPRESSED_R8G8B8A8};
    Texture texture = LoadTextureFromImage(framebuffer_image);
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);

        execute();

        UpdateTexture(texture, framebuffer);
        DrawTexturePro(texture,
                       (Rectangle){0, 0, VIEWPORT_WIDTH, VIEWPORT_HEIGHT},
                       (Rectangle){0, 0, WINDOW_WIDTH, WINDOW_HEIGHT},
                       (Vector2){0, 0}, 0.f, WHITE);
        EndDrawing();
    }

    CloseWindow();
}
