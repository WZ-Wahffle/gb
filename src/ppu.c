#include "ppu.h"
#include "cpu.h"
#include "raylib.h"
#include "types.h"
#include "ui.h"

extern cpu_t cpu;
extern ppu_t ppu;

static uint32_t colors[] = {0xffffffff, 0xffc8c8c8, 0xff828282, 0xff000000};

void set_bg_palette(uint8_t value) {
    ppu.bg_color[0] = colors[(value >> 0) & 0b11];
    ppu.bg_color[1] = colors[(value >> 2) & 0b11];
    ppu.bg_color[2] = colors[(value >> 4) & 0b11];
    ppu.bg_color[3] = colors[(value >> 6) & 0b11];
}

void set_obj_palette_1(uint8_t value) {
    ppu.obj_color_1[0] = colors[(value >> 0) & 0b11];
    ppu.obj_color_1[1] = colors[(value >> 2) & 0b11];
    ppu.obj_color_1[2] = colors[(value >> 4) & 0b11];
    ppu.obj_color_1[3] = colors[(value >> 6) & 0b11];
}

void set_obj_palette_2(uint8_t value) {
    ppu.obj_color_2[0] = colors[(value >> 0) & 0b11];
    ppu.obj_color_2[1] = colors[(value >> 2) & 0b11];
    ppu.obj_color_2[2] = colors[(value >> 4) & 0b11];
    ppu.obj_color_2[3] = colors[(value >> 6) & 0b11];
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

static void try_step_cpu(void) {
    if (cpu.remaining_cycles > 0) {
        uint8_t elapsed_cycles = execute();
        check_interrupts();
        if (cpu.pc == cpu.breakpoint) {
            cpu.state = STOPPED;
            return;
        }
        if (cpu.watch_addr_interrupt) {
            cpu.watch_addr_interrupt = false;
            cpu.state = STOPPED;
            return;
        }

        if (cpu.watch_opcode_interrupt) {
            cpu.watch_opcode_interrupt = false;
            cpu.state = STOPPED;
        }

        static float timer_timer = 0.f;
        static float div_timer = 0.f;

        if (cpu.memory.timer_enable) {
            static uint16_t increment_intervals[] = {256, 4, 16, 64};
            timer_timer += elapsed_cycles / CPU_FREQ;
            if (timer_timer >
                (increment_intervals[cpu.memory.timer_clock_select] /
                 (CPU_FREQ / 4.f))) {
                if (++cpu.memory.timer_counter == 0) {
                    cpu.memory.timer_if = true;
                    cpu.memory.timer_counter = cpu.memory.timer_modulo;
                }

                timer_timer -=
                    increment_intervals[cpu.memory.timer_clock_select] /
                    (CPU_FREQ / 4.f);
            }
        }

        div_timer += elapsed_cycles / CPU_FREQ;
        while (div_timer >= 1.f / 16384.f) {
            cpu.div++;
            div_timer -= 1.f / 16384.f;
        }
    }
}

void *framebuffer = NULL;

static uint8_t get_window_tile_index(uint8_t x, uint8_t y) {
    uint16_t base = ppu.window_tile_map_location ? 0x9c00 : 0x9800;
    return read_8(base + 32 * (y / 8) + (x / 8));
}

static uint8_t get_background_tile_index(uint8_t x, uint8_t y) {
    uint16_t base = ppu.bg_tile_map_location ? 0x9c00 : 0x9800;
    return read_8(base + 32 * (y / 8) + (x / 8));
}

static uint32_t get_object_tile_color(uint8_t tile_index, uint8_t x, uint8_t y,
                                      uint8_t palette) {
    ASSERT(x < 8, "Tile X position out of bounds, found %d", x);
    ASSERT(y < 8, "Tile Y position out of bounds, found %d", y);
    ASSERT(palette < 2, "Tile palette out of bounds, found %d", palette);
    uint16_t base = 0x8000;
    uint16_t sprite_base_address = base + 16 * tile_index;
    uint16_t line = read_16(sprite_base_address + y * 2);
    uint8_t col_idx = 2 * ((HIBYTE(line) & (1 << (7 - x))) != 0) +
                      ((LOBYTE(line) & (1 << (7 - x))) != 0);
    if (col_idx == 0)
        return 0;
    return palette ? ppu.obj_color_2[col_idx] : ppu.obj_color_1[col_idx];
}

static uint32_t get_background_window_tile_color(uint8_t tile_index, uint8_t x,
                                                 uint8_t y) {
    ASSERT(x < 8, "Tile X position out of bounds, found %d", x);
    ASSERT(y < 8, "Tile Y position out of bounds, found %d", y);
    uint16_t base = ppu.bg_window_tile_data_location
                        ? (tile_index < 128 ? 0x8000 : 0x8800)
                        : (tile_index < 128 ? 0x9000 : 0x8800);

    uint16_t sprite_base_address = base + 16 * (tile_index % 128);
    uint16_t line = read_16(sprite_base_address + y * 2);
    uint8_t col_idx = 2 * ((HIBYTE(line) & (1 << (7 - x))) != 0) +
                      ((LOBYTE(line) & (1 << (7 - x))) != 0);
    return ppu.bg_color[col_idx];
}

static void set_pixel(uint8_t x, uint8_t y, uint32_t value) {
    ((uint32_t *)framebuffer)[x + VIEWPORT_WIDTH * y] = value;
}

static uint32_t get_pixel(uint8_t x, uint8_t y) {
    return ((uint32_t *)framebuffer)[x + VIEWPORT_WIDTH * y];
}

static void set_window_pixel(uint8_t x, uint8_t y) {
    if (x < ppu.wx - 7 || y < ppu.wy)
        return;
    uint8_t tile_index = get_window_tile_index(x - (ppu.wx - 7), y - ppu.wy);
    uint32_t pixel = get_background_window_tile_color(
        tile_index, (x - (ppu.wx - 7)) % 8, (y - ppu.wy) % 8);
    set_pixel(x, y, pixel);
}

static void set_background_pixel(uint8_t x, uint8_t y) {
    uint8_t tile_index =
        get_background_tile_index(x + ppu.scroll_x, y + ppu.scroll_y);
    uint32_t pixel = get_background_window_tile_color(
        tile_index, (x + ppu.scroll_x) % 8, (y + ppu.scroll_y) % 8);

    set_pixel(x, y, pixel);
}

static void try_step_ppu(void) {
    if (ppu.remaining_cycles > 0) {
        ppu.remaining_cycles -= CYCLES_PER_DOT;

        ppu.drawing_x++;
        if (ppu.drawing_x == 456) {
            ppu.drawing_x = 0;
            ppu.drawing_y++;
            if (ppu.drawing_y == 144) {
            }
            if (ppu.drawing_y == 154) {
                ppu.drawing_y = 0;
            }
        }

        if (ppu.drawing_x >= 80 && ppu.drawing_x < 240) {
            if (ppu.drawing_y < 144) {
                uint8_t screen_x = ppu.drawing_x - 80;
                uint8_t screen_y = ppu.drawing_y;
                if (ppu.bg_window_enable) {
                    set_background_pixel(screen_x, screen_y);
                    if (ppu.window_enable) {
                        set_window_pixel(screen_x, screen_y);
                    }
                }
                if (ppu.enable_objects) {
                    for (uint8_t i = 0; i < 40; i++) {
                        if (ppu.large_objects) {
                            if (IN_INTERVAL(cpu.memory.oam[i].y - 1, screen_y,
                                            screen_y + 16) &&
                                IN_INTERVAL(cpu.memory.oam[i].x - 1, screen_x,
                                            screen_x + 8)) {
                                uint8_t obj_x =
                                    cpu.memory.oam[i].x - 1 - screen_x;
                                uint8_t obj_y =
                                    cpu.memory.oam[i].y - 1 - screen_y;
                                if (!(cpu.memory.oam[i].attr & 0x20))
                                    obj_x = 7 - obj_x;
                                if (!(cpu.memory.oam[i].attr & 0x40))
                                    obj_y = 15 - obj_y;
                                uint8_t tile_idx = cpu.memory.oam[i].tile;
                                if (obj_y < 8)
                                    tile_idx &= ~1;
                                else
                                    tile_idx |= 1;
                                uint32_t col_idx = get_object_tile_color(
                                    tile_idx, obj_x, obj_y % 8,
                                    (cpu.memory.oam[i].attr & 0x10) != 0 ? 1
                                                                         : 0);
                                if (col_idx != 0 &&
                                    !((cpu.memory.oam[i].attr & 0x80) != 0 &&
                                      get_pixel(screen_x, screen_y) !=
                                          ppu.bg_color[0]))set_pixel(screen_x, screen_y, col_idx);
                            }
                        } else {
                            if (IN_INTERVAL(cpu.memory.oam[i].y - 9, screen_y,
                                            screen_y + 8) &&
                                IN_INTERVAL(cpu.memory.oam[i].x - 1, screen_x,
                                            screen_x + 8)) {
                                uint8_t obj_x =
                                    cpu.memory.oam[i].x - 1 - screen_x;
                                uint8_t obj_y =
                                    cpu.memory.oam[i].y - 9 - screen_y;
                                if (!(cpu.memory.oam[i].attr & 0x20))
                                    obj_x = 7 - obj_x;
                                if (!(cpu.memory.oam[i].attr & 0x40))
                                    obj_y = 7 - obj_y;

                                uint32_t col_idx = get_object_tile_color(
                                    cpu.memory.oam[i].tile, obj_x, obj_y,
                                    (cpu.memory.oam[i].attr & 0x10) != 0 ? 1
                                                                         : 0);
                                if (col_idx != 0 &&
                                    !((cpu.memory.oam[i].attr & 0x80) != 0 &&
                                      get_pixel(screen_x, screen_y) !=
                                          ppu.bg_color[0]))
                                    set_pixel(screen_x, screen_y, col_idx);
                            }
                        }
                    }
                }
            }

            ppu.ly = ppu.drawing_y;
            if (ppu.lyc_int && ppu.ly == ppu.lyc)
                cpu.memory.lcd_if = true;
        }

        if (ppu.drawing_y > 143) {
            ppu.mode = 1;
            cpu.memory.vblank_if = true;
            if (ppu.mode_1_int)
                cpu.memory.lcd_if = true;
        } else {
            if (ppu.drawing_x < 80) {
                ppu.mode = 2;
                if (ppu.mode_2_int)
                    cpu.memory.lcd_if = true;
            } else if (ppu.drawing_x < 252) {
                ppu.mode = 3;
            } else {
                ppu.mode = 0;
                if (ppu.mode_0_int)
                    cpu.memory.lcd_if = true;
            }
        }
    }
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
    bool show_electron_beam = false;

    cpp_init();
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);

        ppu.a = IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT) || IsKeyDown(KEY_A);
        ppu.b = IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN) || IsKeyDown(KEY_B);
        ppu.start = IsGamepadButtonDown(0, GAMEPAD_BUTTON_MIDDLE_RIGHT) || IsKeyDown(KEY_RIGHT_SHIFT);
        ppu.select = IsGamepadButtonDown(0, GAMEPAD_BUTTON_MIDDLE_LEFT) || IsKeyDown(KEY_LEFT_SHIFT);
        ppu.up = GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_Y) < -0.5 || IsKeyDown(KEY_UP);
        ppu.down = GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_Y) > 0.5 || IsKeyDown(KEY_DOWN);
        ppu.left = GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X) < -0.5 || IsKeyDown(KEY_LEFT);
        ppu.right = GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X) > 0.5 || IsKeyDown(KEY_RIGHT);

        for (uint32_t i = 0; i < 70224 * PLAYBACK_SPEED; i++) {
            switch (cpu.state) {
            case STOPPED:
                // this page intentionally left blank
                break;
            case STEPPED:
                ppu.remaining_cycles += (-cpu.remaining_cycles) + 1;
                cpu.remaining_cycles = 1;
                cpu.state = STOPPED;
                try_step_cpu();
                try_step_ppu();
                break;
            case RUNNING:
                cpu.remaining_cycles += CYCLES_PER_DOT;
                ppu.remaining_cycles += CYCLES_PER_DOT;
                while (cpu.remaining_cycles > 0 && cpu.state != STOPPED) {
                    try_step_cpu();
                }
                try_step_ppu();
                break;
            }
        }

        uint32_t old_value;
        if (show_electron_beam && ppu.drawing_y < 144 && ppu.drawing_x >= 80 &&
            ppu.drawing_x < 240) {
            old_value =
                ((uint32_t *)framebuffer)[ppu.drawing_y * VIEWPORT_WIDTH +
                                          ppu.drawing_x - 80];
            ((uint32_t *)framebuffer)[ppu.drawing_y * VIEWPORT_WIDTH +
                                      ppu.drawing_x - 80] = 0xff0000ff;
        }

        UpdateTexture(texture, framebuffer);
        DrawTexturePro(texture,
                       (Rectangle){0, 0, VIEWPORT_WIDTH, VIEWPORT_HEIGHT},
                       (Rectangle){0, 0, WINDOW_WIDTH, WINDOW_HEIGHT},
                       (Vector2){0, 0}, 0.f, WHITE);

        if (show_electron_beam && ppu.drawing_y < 144 && ppu.drawing_x >= 80 &&
            ppu.drawing_x < 240) {
            ((uint32_t *)framebuffer)[ppu.drawing_y * VIEWPORT_WIDTH +
                                      ppu.drawing_x - 80] = old_value;
        }

        if (IsKeyPressed(KEY_TAB)) {
            show_debug = !show_debug;
        }

        if (IsKeyPressed(KEY_LEFT_ALT)) {
            show_electron_beam = !show_electron_beam;
        }

        if (show_debug) {
            cpp_imgui_render();
        }

        EndDrawing();
    }

    UnloadTexture(texture);
    UnloadImage(framebuffer_image);
    cpp_end();
    CloseWindow();
}
