#ifndef TYPES_H_
#define TYPES_H_

#include "raylib.h"
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

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
#define CPU_FREQ 4194304.
#define VIEWPORT_WIDTH 160
#define VIEWPORT_HEIGHT 144
#define WINDOW_WIDTH (VIEWPORT_WIDTH * 6)
#define WINDOW_HEIGHT (VIEWPORT_HEIGHT * 6)
#define DOT_FREQ 4194304.
#define DOT_LENGTH (1. / DOT_FREQ) // independant of CPU frequency!
#define CYCLES_PER_DOT (CPU_FREQ / DOT_FREQ)
#define SCANLINE_COUNT 154

typedef enum { B, C, D, E, H, L, INDHL, A } r8_t;

typedef enum { BC, DE, HL, SP } r16_t;

typedef enum { BC_STK, DE_STK, HL_STK, AF_STK } r16_stk_t;

typedef enum { BC_MEM, DE_MEM, HLINC_MEM, HLDEC_MEM } r16_mem_t;

typedef enum { NZ_COND, Z_COND, NC_COND, C_COND } cond_t;

typedef enum { Z_STATUS, N_STATUS, H_STATUS, C_STATUS } status_t;

typedef enum { STOPPED, STEPPED, RUNNING } state_t;

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
} cpu_mmu_t;

typedef struct {
    cpu_mmu_t memory;
    uint8_t a, b, c, d, e, f, h, l;
    uint16_t sp, pc;
    double remaining_cycles;
    uint8_t opcode;
    state_t state;
    uint16_t breakpoint;
} cpu_t;

typedef struct {
    uint8_t mode;
    uint8_t color_0, color_1, color_2, color_3;
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

    uint8_t period_low;

    uint8_t period_high;
    bool length_enable;
    bool trigger;

} pulse_channel_t;

typedef struct {
    AudioStream handle;
    bool pan_left;
    bool pan_right;
    bool enable;

    bool dac_on;

    uint8_t initial_length_timer;

    uint8_t output_level;

    uint8_t period_low;

    uint8_t period_high;
    bool length_enable;
    bool trigger;

    uint8_t wave_ram[16];
} wave_channel_t;

typedef struct {
    AudioStream handle;
    bool pan_left;
    bool pan_right;
    bool enable;

    uint8_t intitial_length_timer;

    uint8_t envelope_initial_volume;
    bool envelope_dir;
    uint8_t envelope_pace;

    uint8_t clock_shift;
    bool narrow_lfsr;
    uint8_t clock_divider;

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
