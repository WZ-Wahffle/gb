#include "cpu_mmu.h"
#include "apu.h"
#include "cpu.h"
#include "ppu.h"
#include "types.h"

extern cpu_t cpu;
extern ppu_t ppu;
extern apu_t apu;

static uint8_t boot_rom[] = {
    0x31, 0xfe, 0xff, 0xaf, 0x21, 0xff, 0x9f, 0x32, 0xcb, 0x7c, 0x20, 0xfb,
    0x21, 0x26, 0xff, 0x0e, 0x11, 0x3e, 0x80, 0x32, 0xe2, 0x0c, 0x3e, 0xf3,
    0xe2, 0x32, 0x3e, 0x77, 0x77, 0x3e, 0xfc, 0xe0, 0x47, 0x11, 0x04, 0x01,
    0x21, 0x10, 0x80, 0x1a, 0xcd, 0x95, 0x00, 0xcd, 0x96, 0x00, 0x13, 0x7b,
    0xfe, 0x34, 0x20, 0xf3, 0x11, 0xd8, 0x00, 0x06, 0x08, 0x1a, 0x13, 0x22,
    0x23, 0x05, 0x20, 0xf9, 0x3e, 0x19, 0xea, 0x10, 0x99, 0x21, 0x2f, 0x99,
    0x0e, 0x0c, 0x3d, 0x28, 0x08, 0x32, 0x0d, 0x20, 0xf9, 0x2e, 0x0f, 0x18,
    0xf3, 0x67, 0x3e, 0x64, 0x57, 0xe0, 0x42, 0x3e, 0x91, 0xe0, 0x40, 0x04,
    0x1e, 0x02, 0x0e, 0x0c, 0xf0, 0x44, 0xfe, 0x90, 0x20, 0xfa, 0x0d, 0x20,
    0xf7, 0x1d, 0x20, 0xf2, 0x0e, 0x13, 0x24, 0x7c, 0x1e, 0x83, 0xfe, 0x62,
    0x28, 0x06, 0x1e, 0xc1, 0xfe, 0x64, 0x20, 0x06, 0x7b, 0xe2, 0x0c, 0x3e,
    0x87, 0xe2, 0xf0, 0x42, 0x90, 0xe0, 0x42, 0x15, 0x20, 0xd2, 0x05, 0x20,
    0x4f, 0x16, 0x20, 0x18, 0xcb, 0x4f, 0x06, 0x04, 0xc5, 0xcb, 0x11, 0x17,
    0xc1, 0xcb, 0x11, 0x17, 0x05, 0x20, 0xf5, 0x22, 0x23, 0x22, 0x23, 0xc9,
    0xce, 0xed, 0x66, 0x66, 0xcc, 0x0d, 0x00, 0x0b, 0x03, 0x73, 0x00, 0x83,
    0x00, 0x0c, 0x00, 0x0d, 0x00, 0x08, 0x11, 0x1f, 0x88, 0x89, 0x00, 0x0e,
    0xdc, 0xcc, 0x6e, 0xe6, 0xdd, 0xdd, 0xd9, 0x99, 0xbb, 0xbb, 0x67, 0x63,
    0x6e, 0x0e, 0xec, 0xcc, 0xdd, 0xdc, 0x99, 0x9f, 0xbb, 0xb9, 0x33, 0x3e,
    0x3c, 0x42, 0xb9, 0xa5, 0xb9, 0xa5, 0x42, 0x3c, 0x21, 0x04, 0x01, 0x11,
    0xa8, 0x00, 0x1a, 0x13, 0xbe, 0x20, 0xfe, 0x23, 0x7d, 0xfe, 0x34, 0x20,
    0xf5, 0x06, 0x19, 0x78, 0x86, 0x23, 0x05, 0x20, 0xfb, 0x86, 0x20, 0xfe,
    0x3e, 0x01, 0xe0, 0x50};

uint8_t mmu_read(uint16_t addr) {
    if (!cpu.memory.finished_boot && addr < 0x100) {
        return boot_rom[addr];
    } else if (addr < 0x8000) {
        return cpu.memory.read(addr);
    } else if (addr < 0xa000) {
        return cpu.memory.vram[addr % 0x2000];
    } else if (addr < 0xc000) {
        return cpu.memory.read(addr);
    } else if (addr < 0xe000) {
        return cpu.memory.wram[addr % 0x2000];
    } else if (addr < 0xfe00) {
        ASSERT(0, "Read from 0x%04x, Nintendo says no\n", addr);
    } else if (addr < 0xfea0) {
        return ((uint8_t *)cpu.memory.oam)[addr - 0xfe00];
    } else if (addr < 0xff00) {
        ASSERT(0, "Read from 0x%04x, Nintendo says no\n", addr);
    } else if (addr >= 0xff80 && addr < 0xffff) {
        return cpu.memory.hram[addr - 0xff80];
    } else {
        switch (addr) {
        case 0xff00:
            if (!ppu.select_dpad) {
                return ((!ppu.right) << 0) | ((!ppu.left) << 1) |
                       ((!ppu.up) << 2) | ((!ppu.down) << 3) |
                       (ppu.select_dpad << 4) | (ppu.select_buttons << 5);
            }
            if (!ppu.select_buttons) {
                return ((!ppu.a) << 0) | ((!ppu.b) << 1) |
                       ((!ppu.select) << 2) | ((!ppu.start) << 3) |
                       (ppu.select_dpad << 4) | (ppu.select_buttons << 5);
            }
            return 0xf | (ppu.select_dpad << 4) | (ppu.select_buttons << 5);
        case 0xff04:
            return cpu.div;
        case 0xff4d:
        case 0xff4f:
        case 0xff68:
        case 0xff69:
        case 0xff6a:
        case 0xff6b:
            printf("Write to CGB register\n");
            return 0;
        case 0xff40:
            return (ppu.bg_window_enable << 0) | (ppu.enable_objects << 1) |
                   (ppu.large_objects << 2) | (ppu.bg_tile_map_location << 3) |
                   (ppu.bg_window_tile_data_location << 4) |
                   (ppu.window_enable << 5) |
                   (ppu.window_tile_map_location << 6) | (ppu.ppu_enable << 7);
        case 0xff41:
            return (ppu.mode << 0) | ((ppu.ly == ppu.lyc) << 2) |
                   (ppu.mode_0_int << 3) | (ppu.mode_1_int << 4) |
                   (ppu.mode_2_int << 5) | (ppu.lyc_int << 6);
        case 0xff42:
            return ppu.scroll_y;
        case 0xff44:
            return ppu.ly;
        case 0xffff:
            return (cpu.memory.vblank_ie << 0) | (cpu.memory.lcd_ie << 1) |
                   (cpu.memory.timer_ie << 2) | (cpu.memory.serial_ie << 3) |
                   (cpu.memory.joypad_ie << 4);
        default:
            UNREACHABLE_SWITCH(addr);
        }
    }
}

void mmu_write(uint16_t addr, uint8_t value) {
    if (addr == cpu.watch_addr && cpu.watching_addr) {
        cpu.watch_addr_interrupt = true;
    }

    if (!cpu.memory.finished_boot && addr < 0x100) {
        boot_rom[addr] = value;
    } else if (addr < 0x8000) {
        cpu.memory.write(addr, value);
    } else if (addr < 0xa000) {
        cpu.memory.vram[addr % 0x2000] = value;
    } else if (addr < 0xc000) {
        cpu.memory.write(addr, value);
    } else if (addr < 0xe000) {
        cpu.memory.wram[addr % 0x2000] = value;
    } else if (addr < 0xfe00) {
        ASSERT(0, "Write of 0x%02x to 0x%04x, Nintendo says no\n", value, addr);
    } else if (addr < 0xfea0) {
        ((uint8_t *)cpu.memory.oam)[addr - 0xfe00] = value;
    } else if (addr < 0xff00) {
        // this page intentionally left blank
    } else if (addr >= 0xff80 && addr < 0xffff) {
        cpu.memory.hram[addr - 0xff80] = value;
    } else {
        switch (addr) {
        case 0xff00:
            ppu.select_dpad = value & 0x10;
            ppu.select_buttons = value & 0x20;
            break;
        case 0xff01:
            // printf("TODO: Serial transfer data 0x%02x %c\n", value, value);
            break;
        case 0xff02:
            // printf("TODO: Serial transfer control 0x%02x\n", value);
            break;
        case 0xff06:
            cpu.memory.timer_modulo = value;
            break;
        case 0xff07:
            cpu.memory.timer_clock_select = value & 0b11;
            cpu.memory.timer_enable = value & 0x4;
            break;
        case 0xff0f:
            cpu.memory.vblank_if = value & 0x1;
            cpu.memory.lcd_if = value & 0x2;
            cpu.memory.timer_if = value & 0x4;
            cpu.memory.serial_if = value & 0x8;
            cpu.memory.joypad_if = value & 0x10;
            break;
        case 0xff10:
            ch1_sweep(value);
            break;
        case 0xff11:
            ch1_length_timer_duty_cycle(value);
            break;
        case 0xff12:
            ch1_volume_envelope(value);
            break;
        case 0xff13:
            ch1_period_low(value);
            break;
        case 0xff14:
            ch1_period_high_control(value);
            break;
        case 0xff16:
            ch2_length_timer_duty_cycle(value);
            break;
        case 0xff17:
            ch2_volume_envelope(value);
            break;
        case 0xff18:
            ch2_period_low(value);
            break;
        case 0xff19:
            ch2_period_high_control(value);
            break;
        case 0xff1a:
            apu.ch3.dac_on = value & 0x80;
            break;
        case 0xff1b:
            apu.ch3.initial_length_timer = value;
            break;
        case 0xff1c:
            apu.ch3.output_level = (value >> 5) & 0b11;
            break;
        case 0xff1d:
            ch3_period_low(value);
            break;
        case 0xff1e:
            ch3_period_high_control(value);
            break;
        case 0xff20:
            ch4_length_timer(value);
            break;
        case 0xff21:
            ch4_volume_envelope(value);
            break;
        case 0xff22:
            ch4_frequency_randomness(value);
            break;
        case 0xff23:
            ch4_control(value);
            break;
        case 0xff24:
            master_volume_vin_panning(value);
            break;
        case 0xff25:
            sound_panning(value);
            break;
        case 0xff26:
            audio_master_control(value);
            break;
        case 0xff30:
        case 0xff31:
        case 0xff32:
        case 0xff33:
        case 0xff34:
        case 0xff35:
        case 0xff36:
        case 0xff37:
        case 0xff38:
        case 0xff39:
        case 0xff3a:
        case 0xff3b:
        case 0xff3c:
        case 0xff3d:
        case 0xff3e:
        case 0xff3f:
            apu.ch3.wave_ram[addr - 0xff30] = value;
            break;
        case 0xff40:
            lcd_control(value);
            break;
        case 0xff41:
            lcd_status_write(value);
            break;
        case 0xff42:
            ppu.scroll_y = value;
            break;
        case 0xff43:
            ppu.scroll_x = value;
            break;
        case 0xff46: {
            uint16_t start = value * 0x100;
            for (uint8_t i = 0; i < 40; i++) {
                cpu.memory.oam[i].y = read_8(start + i * 4 + 0);
                cpu.memory.oam[i].x = read_8(start + i * 4 + 1);
                cpu.memory.oam[i].tile = read_8(start + i * 4 + 2);
                cpu.memory.oam[i].attr = read_8(start + i * 4 + 3);
            }
        } break;
        case 0xff47:
            set_bg_palette(value);
            break;
        case 0xff48:
            set_obj_palette_1(value);
            break;
        case 0xff49:
            set_obj_palette_2(value);
            break;
        case 0xff4a:
            ppu.wy = value;
            break;
        case 0xff4b:
            ppu.wx = value;
            break;
        case 0xff50:
            cpu.memory.finished_boot = true;
            break;
        case 0xff4d:
        case 0xff4f:
        case 0xff68:
        case 0xff69:
        case 0xff6a:
        case 0xff6b:
            printf("Write to CGB register\n");
            break;
        case 0xff7f:
            // this page intentionally left blank
            break;
        case 0xffff:
            cpu.memory.vblank_ie = value & 0x1;
            cpu.memory.lcd_ie = value & 0x2;
            cpu.memory.timer_ie = value & 0x4;
            cpu.memory.serial_ie = value & 0x8;
            cpu.memory.joypad_ie = value & 0x10;
            break;
        default:
            UNREACHABLE_SWITCH(addr);
        }
    }
}
