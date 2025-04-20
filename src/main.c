#include "apu.h"
#include "carts/mbc1.h"
#include "carts/mbc3.h"
#include "carts/mbc5.h"
#include "carts/nocart.h"
#include "ppu.h"
#include "ui.h"
#include <string.h>
#include <threads.h>

extern cpu_t cpu;
extern ppu_t ppu;
extern apu_t apu;

void exit_cb(int code, void *param) {
    (void)code;
    (void)param;

    printf("Trace:\n");
    for (uint16_t idx = cpu.prev_idx + 1; idx != cpu.prev_idx; idx++) {
        printf("0x%04x: %s\n", cpu.prev_pc[idx],
               opcode_to_string(cpu.prev_opcode[idx]));
    }
}

int main(int argc, char **argv) {
    ASSERT(
        argc == 2,
        "Incorrect number of parameters, found %d. Usage: \n./gbc <game>.gbc\n",
        argc);
    ASSERT((strncmp(".gbc", argv[1] + strlen(argv[1]) - 4, 4) == 0) ||
               (strncmp(".gb", argv[1] + strlen(argv[1]) - 3, 3) == 0),
           "File extension %s not supported, please provide .gb or .gbc file!",
           argv[1] + strlen(argv[1]) - 4);
    FILE *f = fopen(argv[1], "rb");
    *strrchr(argv[1], '.') = 0;
    cpu.filename = argv[1];
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
    case 0x00:
        nocart_init(f, rom_size, ram_size);
        cpu.memory.read = nocart_read;
        cpu.memory.write = nocart_write;
        cpu.memory.free = nocart_free;
        break;
    case 0x01:
    case 0x02:
    case 0x03:
        mbc1_init(f, rom_size, ram_size);
        cpu.memory.read = mbc1_read;
        cpu.memory.write = mbc1_write;
        cpu.memory.free = mbc1_free;
        cpu.save_callback = mbc1_save;
        cpu.load_callback = mbc1_load;
        break;
    case 0x0f:
    case 0x10:
    case 0x11:
    case 0x12:
    case 0x13:
        mbc3_init(f, rom_size, ram_size);
        cpu.memory.read = mbc3_read;
        cpu.memory.write = mbc3_write;
        cpu.memory.free = mbc3_free;
        cpu.cycles_callback = mbc3_cycle_callback;
        cpu.save_callback = mbc3_save;
        cpu.load_callback = mbc3_load;
        break;
    case 0x19:
    case 0x1a:
    case 0x1b:
    case 0x1c:
    case 0x1d:
    case 0x1e:
        mbc5_init(f, rom_size, ram_size);
        cpu.memory.read = mbc5_read;
        cpu.memory.write = mbc5_write;
        cpu.memory.free = mbc5_free;
        cpu.save_callback = mbc5_save;
        cpu.load_callback = mbc5_load;
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

    cpu.playback_speed = 1;
    ppu.ppu_enable = true;
    thrd_t apu_thread;
    thrd_create(&apu_thread, apu_init, NULL);
    thrd_detach(apu_thread);
    on_exit(exit_cb, NULL);
    ui();

    cpu.memory.free();
    fclose(f);
    return 0;
}
