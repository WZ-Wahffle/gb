#ifndef PPU_H_
#define PPU_H_

#include "types.h"

void ui(void);
void set_bg_palette(uint8_t value);
void set_obj_palette_1(uint8_t value);
void set_obj_palette_2(uint8_t value);
void lcd_control(uint8_t value);
void lcd_status_write(uint8_t value);

#endif
