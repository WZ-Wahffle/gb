#include "ppu.h"
#include "cpu.h"
#include "raylib.h"
#include "types.h"
#include "ui.h"

extern cpu_t cpu;
extern ppu_t ppu;

void set_palette(uint8_t value) {
    ppu.color_0 = (value >> 0) & 0b11;
    ppu.color_1 = (value >> 2) & 0b11;
    ppu.color_2 = (value >> 4) & 0b11;
    ppu.color_3 = (value >> 6) & 0b11;
}

static void catch_up_cpu(double cycles_to_add) {
    if (cpu.state == RUNNING || cpu.state == STEPPED) {
        cpu.remaining_cycles += cycles_to_add;
        while (cpu.remaining_cycles > 0) {
            execute();
            if (cpu.state == STEPPED) {
                cpu.state = STOPPED;
                break;
            }
            if(cpu.pc == cpu.breakpoint) {
                cpu.state = STOPPED;
                break;
            }
        }
    }
}

void ui(void) {
    SetTraceLogLevel(LOG_ERROR);
    SetTargetFPS(60);
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "gb");
    void *framebuffer = calloc(VIEWPORT_WIDTH * VIEWPORT_HEIGHT * 4, 1);
    Image framebuffer_image = {framebuffer, VIEWPORT_WIDTH, VIEWPORT_HEIGHT, 1,
                               PIXELFORMAT_UNCOMPRESSED_R8G8B8A8};
    Texture texture = LoadTextureFromImage(framebuffer_image);

    bool show_debug = false;

    cpp_init();
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);

        for (uint8_t y = 0; y < 144; y++) {
            ppu.mode = 2;
            catch_up_cpu(80 * CYCLES_PER_DOT);

            ppu.mode = 3;
            catch_up_cpu(172 * CYCLES_PER_DOT);

            ppu.mode = 0;
            catch_up_cpu(204 * CYCLES_PER_DOT);
        }

        ppu.mode = 1;
        catch_up_cpu(4560 * CYCLES_PER_DOT);

        UpdateTexture(texture, framebuffer);
        DrawTexturePro(texture,
                       (Rectangle){0, 0, VIEWPORT_WIDTH, VIEWPORT_HEIGHT},
                       (Rectangle){0, 0, WINDOW_WIDTH, WINDOW_HEIGHT},
                       (Vector2){0, 0}, 0.f, WHITE);

        if (IsKeyPressed(KEY_TAB)) {
            show_debug = !show_debug;
        }

        if (show_debug) {
            cpp_imgui_render();
        }

        EndDrawing();
    }

    cpp_end();
    CloseWindow();
}
