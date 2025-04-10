#include "apu.h"
#include "carts/mbc1.h"
#include "carts/nocart.h"
#include "ppu.h"
#include <threads.h>
#include "ui.h"

extern cpu_t cpu;
extern ppu_t ppu;
extern apu_t apu;

void exit_cb(int code, void *param) {
    (void)code;
    (void)param;

    printf("Trace:\n");
    for (uint16_t idx = cpu.prev_idx + 1; idx != cpu.prev_idx; idx++) {
        printf("0x%04x: %s\n", cpu.prev_pc[idx], opcode_to_string(cpu.prev_opcode[idx]));
    }
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
    printf("Cartridge Type %d\n", cart_type);
    printf("ROM Size %d Bytes\n", 32768 * (1 << rom_size));

    switch (cart_type) {
    case 0:
        nocart_init(f, rom_size, ram_size);
        cpu.memory.read = nocart_read;
        cpu.memory.write = nocart_write;
        cpu.memory.free = nocart_free;
        break;
    case 1:
    case 2:
    case 3:
        mbc1_init(f, rom_size, ram_size);
        cpu.memory.read = mbc1_read;
        cpu.memory.write = mbc1_write;
        cpu.memory.free = mbc1_free;
        break;
    default:
        ASSERT(0, "Unsupported cart type 0x%02x\n", cart_type);
        break;
    }

#ifdef SKIP_BOOT
    cpu.a = 0x01;
    cpu.f = 0xb0;
    cpu.b = 0x00;
    cpu.c = 0x13;
    cpu.d = 0x00;
    cpu.e = 0xd8;
    cpu.h = 0x01;
    cpu.l = 0x4d;
    cpu.sp = 0xfffe;
    cpu.pc = 0x100;
    cpu.memory.finished_boot = true;
#endif

    thrd_t apu_thread;
    thrd_create(&apu_thread, apu_init, NULL);
    thrd_detach(apu_thread);
    on_exit(exit_cb, NULL);
    ui();

    cpu.memory.free();
    fclose(f);
    return 0;
}
