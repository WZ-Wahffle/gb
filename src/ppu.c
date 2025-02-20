#include "ppu.h"
#include "cpu.h"
#include "raylib.h"
#include "types.h"
#include "ui.h"

extern cpu_t cpu;
extern ppu_t ppu;

static uint32_t colors[] = {0xffffffff, 0xffc8c8c8, 0xff828282, 0xff000000};

void set_bg_palette(uint8_t value) {
    ppu.bg_color[0] = (value >> 0) & 0b11;
    ppu.bg_color[1] = (value >> 2) & 0b11;
    ppu.bg_color[2] = (value >> 4) & 0b11;
    ppu.bg_color[3] = (value >> 6) & 0b11;
}

void set_obj_palette_1(uint8_t value) {
    ppu.obj_color_1[0] = (value >> 0) & 0b11;
    ppu.obj_color_1[1] = (value >> 2) & 0b11;
    ppu.obj_color_1[2] = (value >> 4) & 0b11;
    ppu.obj_color_1[3] = (value >> 6) & 0b11;
}

void set_obj_palette_2(uint8_t value) {
    ppu.obj_color_2[0] = (value >> 0) & 0b11;
    ppu.obj_color_2[1] = (value >> 2) & 0b11;
    ppu.obj_color_2[2] = (value >> 4) & 0b11;
    ppu.obj_color_2[3] = (value >> 6) & 0b11;
}

void lcd_control(uint8_t value) {
    ppu.bg_window_enable = value & 0x1;
    ppu.enable_objects = value & 0x2;
    ppu.large_objects = value & 0x4;
    ppu.bg_tile_map_location = value & 0x8;
    ppu.bg_window_tile_data_location = value & 0x10;
    ppu.window_enable = value & 0x20;
    ppu.window_tile_map_location = value & 0x40;
    ppu.ppu_enable = value & 0x80;
}

void lcd_status_write(uint8_t value) {
    ppu.mode_0_int = value & 0x8;
    ppu.mode_1_int = value & 0x10;
    ppu.mode_2_int = value & 0x20;
    ppu.lyc_int = value & 0x40;
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
            if (cpu.pc == cpu.breakpoint) {
                cpu.state = STOPPED;
                break;
            }
        }
    }
}

void *framebuffer = NULL;

static uint8_t get_window_tile_index(uint8_t x, uint8_t y) {
    uint16_t base = ppu.bg_tile_map_location ? 0x9c00 : 0x9800;
    return read_8(base + 32 * (y / 8) + (x / 8));
}

static uint8_t get_tile_color(uint8_t tile_index, uint8_t x, uint8_t y) {
    ASSERT(x < 8, "Tile X position out of bounds, found %d", x);
    ASSERT(y < 8, "Tile Y position out of bounds, found %d", y);
    uint16_t base = ppu.bg_window_tile_data_location
                        ? (tile_index < 128 ? 0x8000 : 0x8800)
                        : (tile_index < 128 ? 0x9000 : 0x8800);

    uint16_t sprite_base_address = base + 16 * tile_index;
    uint16_t line = read_16(sprite_base_address + y * 2);
    return ((HIBYTE(line) & (1 << (7 - x))) != 0) +
           2 * ((LOBYTE(line) & (1 << (7 - x))) != 0);
}

void ui(void) {
    SetTraceLogLevel(LOG_ERROR);
    SetTargetFPS(60);
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "gb");
    framebuffer = calloc(VIEWPORT_WIDTH * VIEWPORT_HEIGHT * 4, 1);
    Image framebuffer_image = {framebuffer, VIEWPORT_WIDTH, VIEWPORT_HEIGHT, 1,
                               PIXELFORMAT_UNCOMPRESSED_R8G8B8A8};
    Texture texture = LoadTextureFromImage(framebuffer_image);

    bool show_debug = false;

    cpp_init();
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);

        ppu.a = IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT);
        ppu.b = IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN);
        ppu.start = IsGamepadButtonDown(0, GAMEPAD_BUTTON_MIDDLE_RIGHT);
        ppu.select = IsGamepadButtonDown(0, GAMEPAD_BUTTON_MIDDLE_LEFT);
        ppu.up = GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_Y) < -0.5;
        ppu.down = GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_Y) > 0.5;
        ppu.left = GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X) < -0.5;
        ppu.right = GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X) > 0.5;

        for (uint8_t y = 0; y < VIEWPORT_HEIGHT; y++) {
            ppu.ly = y;
            if (ppu.lyc_int && ppu.ly == ppu.lyc)
                cpu.memory.lcd_if = true;
            ppu.mode = 2;
            if (ppu.mode_2_int)
                cpu.memory.lcd_if = true;
            catch_up_cpu(80 * CYCLES_PER_DOT);

            ppu.mode = 3;
            catch_up_cpu(172 * CYCLES_PER_DOT);

            // background drawing
            for (uint8_t x = 0; x < VIEWPORT_WIDTH; x++) {
                uint8_t tile_index =
                    get_window_tile_index(x + ppu.scroll_x, y + ppu.scroll_y);
                uint8_t pixel = get_tile_color(
                    tile_index, (x + ppu.scroll_x) % 8, (y + ppu.scroll_y) % 8);

                ((uint32_t *)framebuffer)[x + VIEWPORT_WIDTH * y] =
                    colors[ppu.bg_color[pixel]];
            }

            ppu.mode = 0;
            if (ppu.mode_0_int)
                cpu.memory.lcd_if = true;
            catch_up_cpu(204 * CYCLES_PER_DOT);
        }

        cpu.memory.vblank_if = true;
        ppu.mode = 1;
        if (ppu.mode_1_int)
            cpu.memory.lcd_if = true;
        for (uint8_t i = 0; i < 10; i++) {
            ppu.ly = 144 + i;
            if (ppu.lyc_int && ppu.ly == ppu.lyc)
                cpu.memory.lcd_if = true;
            catch_up_cpu(456 * CYCLES_PER_DOT);
        }

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

    UnloadTexture(texture);
    UnloadImage(framebuffer_image);
    free(framebuffer);
    cpp_end();
    CloseWindow();
}
