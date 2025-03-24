#ifndef TYPES_H_
#define TYPES_H_

#include "raylib.h"
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define DYNARRAY(type)                                                         \
    typedef struct {                                                           \
        type *items;                                                           \
        size_t count;                                                          \
        size_t capacity;                                                       \
    } dyn_##type;                                                              \
    inline void dyn_##type##_append(dyn_##type arr, type elem) {               \
        if (arr.count >= arr.capacity) {                                       \
            if (arr.capacity == 0)                                             \
                arr.capacity = 8;                                              \
            else                                                               \
                arr.capacity *= 2;                                             \
            arr.items =                                                        \
                (type *)realloc(arr.items, arr.capacity * sizeof(*arr.items)); \
        }                                                                      \
        arr.items[arr.count++] = elem;                                         \
    }

DYNARRAY(uint32_t)
// custom assertion macro to allow registering exit callback
#define ASSERT(val, msg, ...)                                                  \
    do {                                                                       \
        if (!(val)) {                                                          \
            printf("%s:%d: Assertion failed:\n", __FILE__, __LINE__);          \
            printf(msg, __VA_ARGS__);                                          \
            exit(1);                                                           \
        }                                                                      \
    } while (0)
#define UNREACHABLE_SWITCH(val)                                                \
    ASSERT(0, "Illegal value in switch: %d, 0x%04x\n", val, val)
#define TODO(val) ASSERT(0, "%sTODO: " val "\n", "")
#define TO_U16(lsb, msb) ((lsb & 0xff) | ((msb & 0xff) << 8))
#define LOBYTE(val) ((val) & 0xff)
#define HIBYTE(val) (((val) >> 8) & 0xff)
#define IN_INTERVAL(val, min, max) ((val) >= (min) && (val) < (max))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define CPU_FREQ 4194304.
#define VIEWPORT_WIDTH 160
#define VIEWPORT_HEIGHT 144
#define WINDOW_WIDTH (VIEWPORT_WIDTH * 6)
#define WINDOW_HEIGHT (VIEWPORT_HEIGHT * 6)
#define DOT_FREQ 4194304.
#define DOT_LENGTH (1. / DOT_FREQ) // independant of CPU frequency!
#define CYCLES_PER_DOT (CPU_FREQ / DOT_FREQ)
#define SCANLINE_COUNT 154
#define SAMPLE_RATE 48000
#define PLAYBACK_SPEED 1.0

typedef enum { B, C, D, E, H, L, INDHL, A } r8_t;

typedef enum { BC, DE, HL, SP } r16_t;

typedef enum { BC_STK, DE_STK, HL_STK, AF_STK } r16_stk_t;

typedef enum { BC_MEM, DE_MEM, HLINC_MEM, HLDEC_MEM } r16_mem_t;

typedef enum { NZ_COND, Z_COND, NC_COND, C_COND } cond_t;

typedef enum { Z_STATUS, N_STATUS, H_STATUS, C_STATUS } status_t;

typedef enum { STOPPED, STEPPED, RUNNING } state_t;

typedef enum { EIGHTH, QUARTER, HALF, THREEQUARTERS } duty_cycle_t;

typedef struct {
    uint8_t y;
    uint8_t x;
    uint8_t tile;
    uint8_t attr;
} oam_t;

typedef struct {
    uint8_t (*read)(uint16_t);
    void (*write)(uint16_t, uint8_t);
    void (*free)(void);
    uint8_t vram[0x2000];
    uint8_t wram[0x2000];
    uint8_t hram[0x7f];
    oam_t oam[40];
    bool vblank_ie, vblank_if;
    bool lcd_ie, lcd_if;
    bool timer_ie, timer_if;
    bool serial_ie, serial_if;
    bool joypad_ie, joypad_if;
    uint8_t timer_counter, timer_modulo, timer_clock_select;
    bool timer_enable;
    bool finished_boot;
} cpu_mmu_t;

typedef struct {
    cpu_mmu_t memory;
    uint8_t a, b, c, d, e, f, h, l;
    uint16_t sp, pc;
    double remaining_cycles;
    uint8_t opcode;
    state_t state;
    uint16_t breakpoint;
    bool ime;
    uint8_t div;
    bool halted;

    uint16_t prev_pc[0x10000];
    uint16_t prev_idx;
    uint8_t prev_opcode[0x10000];

    uint16_t watch_addr;
    bool watching_addr;
    bool watch_addr_interrupt;

    uint8_t watch_opcode;
    bool watching_opcode;
    bool watch_opcode_interrupt;

} cpu_t;

typedef struct {
    uint8_t mode;
    uint8_t bg_color[4];
    uint8_t obj_color_1[4];
    uint8_t obj_color_2[4];
    double remaining_cycles;
    uint8_t scroll_x, scroll_y;
    bool ppu_enable;
    bool window_tile_map_location;
    bool window_enable;
    bool bg_window_tile_data_location;
    bool bg_tile_map_location;
    bool large_objects;
    bool enable_objects;
    bool bg_window_enable;
    uint8_t ly;
    uint8_t lyc;
    bool lyc_int, mode_2_int, mode_1_int, mode_0_int;
    uint8_t wx, wy;
    bool select_buttons, select_dpad;
    bool a, b, start, select, up, down, left, right;
    uint16_t drawing_x, drawing_y;
} ppu_t;

typedef struct {
    AudioStream handle;
    bool pan_left;
    bool pan_right;
    bool enable;

    uint8_t sweep_pace;
    bool sweep_direction;
    uint8_t sweep_step;

    uint8_t wave_duty;
    uint8_t initial_length_timer;

    uint8_t envelope_initial_volume;
    bool envelope_dir;
    uint8_t envelope_pace;
    uint8_t volume;

    uint8_t period_low;

    uint8_t period_high;
    bool length_enable;

    uint32_t frequency;
} pulse_channel_t;

typedef struct {
    AudioStream handle;
    bool pan_left;
    bool pan_right;
    bool enable;

    bool dac_on;

    uint8_t initial_length_timer;

    uint8_t output_level;
    uint8_t volume;

    uint8_t period_low;

    uint8_t period_high;
    bool length_enable;
    bool trigger;

    uint8_t wave_ram[16];

    uint32_t frequency;
} wave_channel_t;

typedef struct {
    AudioStream handle;
    bool pan_left;
    bool pan_right;
    bool enable;

    uint8_t initial_length_timer;

    uint8_t envelope_initial_volume;
    bool envelope_dir;
    uint8_t envelope_pace;
    uint8_t volume;

    uint8_t clock_shift;
    bool narrow_lfsr;
    float clock_divider;
    uint16_t lfsr;

    bool length_enable;
    bool trigger;
} noise_channel_t;

typedef struct {
    bool audio_enable;
    bool vin_left;
    bool vin_right;
    uint8_t vol_left;
    uint8_t vol_right;
    pulse_channel_t ch1;
    pulse_channel_t ch2;
    wave_channel_t ch3;
    noise_channel_t ch4;
} apu_t;

#endif
