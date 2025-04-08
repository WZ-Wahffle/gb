#ifndef MBC3_H_
#define MBC3_H_

#include "../types.h"

void mbc3_init(FILE* f, uint8_t rom_size, uint8_t ram_size);
uint8_t mbc3_read(uint16_t addr);
void mbc3_write(uint16_t addr, uint8_t value);
void mbc3_free(void);

#endif
