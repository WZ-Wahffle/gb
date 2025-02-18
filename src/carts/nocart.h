#ifndef NOCART_H_
#define NOCART_H_

#include "../types.h"

void nocart_init(FILE* f, uint8_t rom_size, uint8_t ram_size);
uint8_t nocart_read(uint16_t addr);
void nocart_write(uint16_t addr, uint8_t value);
void nocart_free(void);

#endif
