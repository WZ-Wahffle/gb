#ifndef MBC1_H_
#define MBC1_H_

#include "../types.h"

void mbc1_init(FILE* f, uint8_t rom_size, uint8_t ram_size);
uint8_t mbc1_read(uint16_t addr);
void mbc1_write(uint16_t addr, uint8_t value);
void mbc1_free(void);
void mbc1_save(FILE*);
void mbc1_load(FILE*);

#endif
