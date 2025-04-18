#include "mbc5.h"

static uint8_t *rom = NULL;
static uint32_t rom_size_bytes = 0;
static uint16_t rom_number = 0;
static uint8_t *ram = NULL;
static uint32_t ram_size_bytes = 0;
static uint8_t ram_number = 0;
bool ram_enable = false;

void mbc5_init(FILE *f, uint8_t rom_size, uint8_t ram_size) {
    ASSERT(rom_size < 9, "MBC1 supports bank keys [0..8], found %d", rom_size);
    ASSERT(ram_size < 5 && ram_size != 1,
           "MBC1 supports bank keys [0, 2, 3, 4], found %d", ram_size);
    rom_size_bytes = 0x8000 * (1 << rom_size);
    rom = calloc(rom_size_bytes, 1);
    fread(rom, 1, rom_size_bytes, f);
    switch (ram_size) {
    case 0:
        ram_size_bytes = 0;
        break;
    case 2:
        ram_size_bytes = 0x2000;
        break;
    case 3:
        ram_size_bytes = 0x8000;
        break;
    case 4:
        ram_size_bytes = 0x20000;
        break;
    default:
        UNREACHABLE_SWITCH(ram_size);
    }
    ram = calloc(ram_size_bytes, 1);
}

uint8_t mbc5_read(uint16_t addr) {
    if (addr < 0x4000) {
        return rom[addr];
    } else if (addr < 0x8000) {
        return rom[rom_number * 0x4000 + (addr % 0x4000)];
    } else if (addr >= 0xa000 && addr < 0xc000) {
        return ram[ram_number * 0x2000 + (addr % 0x2000)];
    }

    ASSERT(0, "Cart read at 0x%04x not allowed", addr);
}

void mbc5_write(uint16_t addr, uint8_t value) {
    if (addr < 0x2000) {
        ram_enable = (value & 0xf) == 0xa;
    } else if (addr < 0x3000) {
        rom_number &= ~0xff;
        rom_number |= value;
    } else if (addr < 0x4000) {
        rom_number &= 0xff;
        rom_number |= (value & 1) << 8;
    } else if (addr < 0x6000) {
        ram_number = value;
    } else if (addr >= 0xa000 && addr < 0xc000) {
        ram[ram_number * 0x2000 + (addr % 0x2000)] = value;
    } else {
        ASSERT(0, "Cart write at 0x%04x not allowed", addr);
    }
}

void mbc5_free(void) {
    if (rom)
        free(rom);
    if (ram)
        free(ram);
}

void mbc5_save(FILE *f) {
    fwrite(&ram_size_bytes, 4, 1, f);
    fwrite(ram, 1, ram_size_bytes, f);
}

void mbc5_load(FILE *f) {
    uint32_t ram_size_bytes_tmp;
    fread(&ram_size_bytes_tmp, 4, 1, f);
    ASSERT(ram_size_bytes == ram_size_bytes_tmp,
           "Loaded save file does not match cartridge, expected RAM size %d, "
           "got %d",
           ram_size_bytes, ram_size_bytes_tmp);
    fread(ram, 1, ram_size_bytes, f);
}
