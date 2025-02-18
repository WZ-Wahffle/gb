#include "carts/nocart.h"
#include "ppu.h"
#include "types.h"
#include <stdio.h>

extern cpu_t cpu;
extern ppu_t ppu;
extern apu_t apu;

int main(int argc, char **argv) {
    ASSERT(argc == 2, "Incorrect number of parameters, found %d. Usage: \n./gb <game>.gb\n", argc);
    FILE *f = fopen(argv[1], "rb");
    ASSERT(f != NULL, "Could not open %s!\n", argv[1]);
    fseek(f, 0x147, SEEK_SET);
    uint8_t cart_type, rom_size, ram_size;
    fread(&cart_type, 1, 1, f);
    fread(&rom_size, 1, 1, f);
    fread(&ram_size, 1, 1, f);
    fseek(f, 0, SEEK_SET);

    switch (cart_type) {
    case 0:
        nocart_init(f, rom_size, ram_size);
        cpu.memory.read = nocart_read;
        cpu.memory.write = nocart_write;
        cpu.memory.free = nocart_free;
        break;
    default:
        ASSERT(0, "Unsupported cart type %02x\n", cart_type);
        break;
    }

    ui();

    cpu.memory.free();
    fclose(f);
    return 0;
}
