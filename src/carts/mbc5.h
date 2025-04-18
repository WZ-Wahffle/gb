#ifndef MBC5_H_
#define MBC5_H_

#include "../types.h"

void mbc5_init(FILE* f, uint8_t rom_size, uint8_t ram_size);
uint8_t mbc5_read(uint16_t addr);
void mbc5_write(uint16_t addr, uint8_t value);
void mbc5_free(void);
void mbc5_save(FILE*);
void mbc5_load(FILE*);

#endif
