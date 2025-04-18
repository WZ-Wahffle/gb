#include "mbc1.h"

extern cpu_t cpu;

static uint8_t *rom = NULL;
static uint32_t rom_size_bytes = 0;
static uint8_t rom_number = 0;
static uint8_t secondary_number = 0;
static uint8_t *ram = NULL;
static uint32_t ram_size_bytes = 0;
static bool ram_enable = false;
static bool banking_mode = false;

void mbc1_init(FILE *f, uint8_t rom_size, uint8_t ram_size) {
    ASSERT(rom_size < 5, "MBC1 supports bank keys [0..4], found %d", rom_size);
    ASSERT(ram_size < 4 && ram_size != 1,
           "MBC1 supports bank keys [0, 2, 3], found %d", rom_size);

    rom_number = 1;
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
    default:
        UNREACHABLE_SWITCH(ram_size);
    }
    ram = calloc(ram_size_bytes, 1);
}

uint8_t mbc1_read(uint16_t addr) {
    if (addr < 0x4000) {
        if (banking_mode) {
            uint32_t idx = addr + 0x4000 * (secondary_number << 5);
            ASSERT(idx < rom_size_bytes,
                   "ROM access out of bounds, maximum size %d, adressed at %d",
                   rom_size_bytes, idx);
            return rom[idx];
        } else {
            return rom[addr];
        }
    } else if (addr < 0x8000) {
        uint32_t idx = rom_number * 0x4000 + (addr % 0x4000);
        ASSERT(idx < rom_size_bytes,
               "ROM access out of bounds, maximum size %d, adressed at %d",
               rom_size_bytes, idx);
        return rom[idx];
    } else if (addr >= 0xa000 && addr < 0xc000) {
        if (ram_enable) {
            if (banking_mode) {
                uint32_t idx = secondary_number * 0x2000 + (addr % 0x2000);
                ASSERT(
                    idx < ram_size_bytes,
                    "RAM access out of bounds, maximum size %d, adressed at %d",
                    ram_size_bytes, idx);

                return ram[idx];
            } else {
                return ram[addr % 0x2000];
            }
        } else
            return 0xff;
    }

    ASSERT(0, "Cart read at 0x%04x not allowed", addr);
}

void mbc1_write(uint16_t addr, uint8_t value) {
    if (addr < 0x2000) {
        ram_enable = (value & 0xf) == 0xa;
    } else if (addr < 0x4000) {
        rom_number = value & 0b11111;
        if (rom_number == 0)
            rom_number = 1;
        rom_number %= rom_size_bytes / 0x4000;
        cpu.prev_opcode[cpu.prev_idx] = 0xd3;
        cpu.prev_pc[cpu.prev_idx] = rom_number;
        cpu.prev_idx++;
    } else if (addr < 0x6000) {
        secondary_number = value & 0b11;
    } else if (addr < 0x8000) {
        banking_mode = value & 0b1;
    } else if (addr >= 0xa000 && addr < 0xc000) {
        if (ram_enable) {
            if (banking_mode) {
                uint32_t idx = secondary_number * 0x2000 + (addr % 0x2000);
                ASSERT(
                    idx < ram_size_bytes,
                    "RAM access out of bounds, maximum size %d, adressed at %d",
                    ram_size_bytes, idx);

                ram[idx] = value;
            } else {
                ram[addr % 0x2000] = value;
            }
        }
    }
}

void mbc1_free(void) {
    if (rom)
        free(rom);
    if (ram)
        free(ram);
}

void mbc1_save(FILE* f) {
    fwrite(&ram_size_bytes, 4, 1, f);
    fwrite(ram, 1, ram_size_bytes, f);
}

void mbc1_load(FILE* f) {
    uint32_t ram_size_bytes_tmp;
    fread(&ram_size_bytes_tmp, 4, 1, f);
    ASSERT(ram_size_bytes == ram_size_bytes_tmp, "Loaded save file does not match cartridge, expected RAM size %d, got %d", ram_size_bytes, ram_size_bytes_tmp);
    fread(ram, 1, ram_size_bytes, f);
}
