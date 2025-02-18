#ifndef TYPES_H_
#define TYPES_H_

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>

#define ASSERT(val, msg, ...) do { if(!(val)) {printf(msg, __VA_ARGS__);assert(0);} } while(0)
#define UNREACHABLE_SWITCH(val) ASSERT(0, "Illegal value in switch: %d, 0x%04x\n", val, val)
#define TODO(val) ASSERT(0, "%sTODO: " val "\n", "")
#define TO_U16(lsb, msb) ((lsb & 0xff) | ((msb & 0xff) << 8))
#define LOBYTE(val) ((val) & 0xff)
#define HIBYTE(val) (((val) >> 8) & 0xff)
#define CPU_FREQ 4194304
#define VIEWPORT_WIDTH 160
#define VIEWPORT_HEIGHT 144
#define WINDOW_WIDTH (VIEWPORT_WIDTH * 4)
#define WINDOW_HEIGHT (VIEWPORT_HEIGHT * 4)
#define DOT (1. / 4194304) // independant of CPU frequency!

typedef enum {
    B, C, D, E, H, L, INDHL, A
} r8_t;

typedef enum {
    BC, DE, HL, SP
} r16_t;

typedef enum {
    BC_STK, DE_STK, HL_STK, AF_STK
} r16_stk;

typedef enum {
    BC_MEM, DE_MEM, HLINC_MEM, HLDEC_MEM
} r16_mem;

typedef enum {
    NZ_COND, Z_COND, NC_COND, C_COND
} cond_t;

typedef enum {
    Z_STATUS, N_STATUS, H_STATUS, C_STATUS
} status_t;

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
    oam_t oam[40];
} cpu_mmu_t;

typedef struct {
    cpu_mmu_t memory;
    uint8_t a, b, c, d, e, f, h, l;
    uint16_t sp, pc;
} cpu_t;

typedef struct {

} ppu_t;

typedef struct {

} apu_t;

#endif
