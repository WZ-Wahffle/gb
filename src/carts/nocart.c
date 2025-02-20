#include "nocart.h"

static uint8_t* rom = NULL;

void nocart_init(FILE* f, uint8_t rom_size, uint8_t ram_size) {
    ASSERT(rom_size == 0, "Unmapped cartridge not allowed ROM storage value of 0x%02x!\n", rom_size);
    ASSERT(ram_size == 0, "Unmapped cartridge not allowed RAM storage value of 0x%02x!\n", ram_size);

    rom = calloc(0x8000, 1);
    fread(rom, 1, 0x8000, f);
}

uint8_t nocart_read(uint16_t addr) {
    ASSERT(addr < 0x8000, "Attempting to read from cart at address 0x%04x!\n", addr);

    return rom[addr];
}

void nocart_write(uint16_t addr, uint8_t value) {
    printf("Attempting to write 0x%02x to cart at address 0x%04x\n", value, addr);
}

void nocart_free(void) {
    if(rom) free(rom);
}
