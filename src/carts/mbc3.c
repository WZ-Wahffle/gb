#include "mbc3.h"
#include "time.h"

static uint8_t *rom = NULL;
static uint32_t rom_size_bytes = 0;
static uint8_t rom_number = 0;
static uint8_t *ram = NULL;
static uint32_t ram_size_bytes = 0;
static uint8_t ram_number = 0;
static bool ram_rtc_enable = false;

static uint8_t rtc_s = 0, rtc_m = 0, rtc_h = 0;
static uint16_t rtc_d = 0;
static bool rtc_overflow = 0;
static bool rtc_halt = 0;

void mbc3_init(FILE *f, uint8_t rom_size, uint8_t ram_size) {
    ASSERT(rom_size < 7, "MBC1 supports bank keys [0..6], found %d", rom_size);
    ASSERT(ram_size < 4 && ram_size != 1,
           "MBC1 supports bank keys [0, 2, 3], found %d", ram_size);
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

uint8_t mbc3_read(uint16_t addr) {
    if (addr < 0x4000) {
        return rom[addr];
    } else if (addr < 0x8000) {
        return rom[0x4000 * rom_number + (addr % 0x4000)];
    } else if (addr >= 0xa000 && addr < 0xc000) {
        if (ram_number < 4) {
            return ram[0x2000 * ram_number + (addr % 0x2000)];
        } else {
            switch (ram_number) {
            case 8:
                return rtc_s;
            case 9:
                return rtc_m;
            case 10:
                return rtc_h;
            case 11:
                return rtc_d & 0xff;
            case 12:
                return (rtc_d >> 8) | (rtc_halt << 6) | (rtc_overflow << 7);
            default:
                UNREACHABLE_SWITCH(ram_number);
            }
        }
    }

    ASSERT(0, "Cart read at 0x%04x not allowed", addr);
}

void mbc3_write(uint16_t addr, uint8_t value) {
    if (addr < 0x2000) {
        if (value == 0x0a)
            ram_rtc_enable = true;
        if (value == 0)
            ram_rtc_enable = false;
    } else if (addr < 0x4000) {
        rom_number = value & 0x7f;
        if (rom_number == 0)
            rom_number = 1;
    } else if (addr < 0x6000) {
        ram_number = value;
    } else if (addr < 0x8000) {
        // this page intentionally left blank
    } else if (addr >= 0xa000 && addr < 0xc000) {
        if (ram_number < 4) {
            ram[ram_number * 0x2000 + (addr % 0x2000)] = value;
        } else {
            switch (ram_number) {
            case 8:
                rtc_s = value;
                break;
            case 9:
                rtc_m = value;
                break;
            case 10:
                rtc_h = value;
                break;
            case 11:
                rtc_d &= ~0xff;
                rtc_d |= value;
                break;
            case 12:
                rtc_d &= ~0x100;
                rtc_d |= (value & 1) << 8;
                rtc_halt = value & 0x40;
                rtc_overflow = value & 0x80;
                break;
            default:
                UNREACHABLE_SWITCH(ram_number);
            }
        }
    }
}

void mbc3_free(void) {
    if (rom)
        free(rom);
    if (ram)
        free(ram);
}

void mbc3_cycle_callback(uint32_t count) {
    static uint32_t cycle_count = 0;
    cycle_count += count;
    // TODO: potentially adjust for CGB in future
    if (cycle_count >= CPU_FREQ) {
        cycle_count -= CPU_FREQ;
        rtc_s++;
        if (rtc_s == 60) {
            rtc_s = 0;
            rtc_m++;
            if (rtc_m == 60) {
                rtc_m = 0;
                rtc_h++;
                if (rtc_h == 24) {
                    rtc_h = 0;
                    rtc_d++;
                    if (rtc_d == 512) {
                        rtc_d = 0;
                        rtc_overflow = true;
                    }
                }
            }
        }
    }
}

void mbc3_save(FILE * f) {
    fwrite(&ram_size_bytes, 4, 1, f);
    fwrite(ram, 1, ram_size_bytes, f);
}

void mbc3_load(FILE * f) {
    uint32_t ram_size_bytes_tmp;
    fread(&ram_size_bytes_tmp, 4, 1, f);
    ASSERT(ram_size_bytes == ram_size_bytes_tmp, "Loaded save file does not match cartridge, expected RAM size %d, got %d", ram_size_bytes, ram_size_bytes_tmp);
    fread(ram, 1, ram_size_bytes, f);
}
