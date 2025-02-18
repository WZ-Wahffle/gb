#ifndef CPU_MMU_H_
#define CPU_MMU_H_

#include "types.h"

uint8_t mmu_read(uint16_t addr);
void mmu_write(uint16_t addr, uint8_t value);

#endif
