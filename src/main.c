#include "apu.h"
#include "carts/mbc1.h"
#include "carts/nocart.h"
#include "ppu.h"
#include "types.h"

extern cpu_t cpu;
extern ppu_t ppu;
extern apu_t apu;

void exit_cb(int code, void *param) {
    (void)code;
    (void)param;
    printf("\nPC: 0x%04x\n", cpu.pc);
    printf("Previous opcode: 0x%02x\n", cpu.opcode);
}

int main(int argc, char **argv) {
    ASSERT(
        argc == 2,
        "Incorrect number of parameters, found %d. Usage: \n./gb <game>.gb\n",
        argc);
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
    case 1:
        mbc1_init(f, rom_size, ram_size);
        cpu.memory.read = mbc1_read;
        cpu.memory.write = mbc1_write;
        cpu.memory.free = mbc1_free;
        break;
    default:
        ASSERT(0, "Unsupported cart type 0x%02x\n", cart_type);
        break;
    }

    apu_init();
    on_exit(exit_cb, NULL);
    ui();

    cpu.memory.free();
    fclose(f);
    return 0;
}
