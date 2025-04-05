#ifndef CPU_H_
#define CPU_H_

#include "types.h"
#include "cpu_mmu.h"

#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

uint8_t execute(void);
void check_interrupts(void);
EXTERNC uint8_t read_8(uint16_t addr);
EXTERNC uint16_t read_16(uint16_t addr);
EXTERNC void write_8(uint16_t addr, uint8_t val);

#endif
