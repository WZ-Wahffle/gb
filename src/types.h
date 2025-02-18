#ifndef TYPES_H_
#define TYPES_H_

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#define ASSERT(val, msg, ...) do { if(!(val)) {printf(msg, __VA_ARGS__);assert(0);} } while(0)

typedef struct {
    uint8_t (*read)(uint16_t);
    void (*write)(uint16_t, uint8_t);
    void (*free)(void);
} cpu_mmu_t;

typedef struct {
    cpu_mmu_t memory;
} cpu_t;

typedef struct {

} ppu_t;

typedef struct {

} apu_t;

#endif
