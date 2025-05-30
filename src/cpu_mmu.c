#include "cpu_mmu.h"
#include "apu.h"
#include "cpu.h"
#include "ppu.h"
#include "types.h"

extern cpu_t cpu;
extern ppu_t ppu;
extern apu_t apu;

static uint8_t boot_rom[] = {
    0x31, 0xfe, 0xff, 0x3e, 0x02, 0xc3, 0x7c, 0x00, 0xd3, 0x00, 0x98, 0xa0,
    0x12, 0xd3, 0x00, 0x80, 0x00, 0x40, 0x1e, 0x53, 0xd0, 0x00, 0x1f, 0x42,
    0x1c, 0x00, 0x14, 0x2a, 0x4d, 0x19, 0x8c, 0x7e, 0x00, 0x7c, 0x31, 0x6e,
    0x4a, 0x45, 0x52, 0x4a, 0x00, 0x00, 0xff, 0x53, 0x1f, 0x7c, 0xff, 0x03,
    0x1f, 0x00, 0xff, 0x1f, 0xa7, 0x00, 0xef, 0x1b, 0x1f, 0x00, 0xef, 0x1b,
    0x00, 0x7c, 0x00, 0x00, 0xff, 0x03, 0xce, 0xed, 0x66, 0x66, 0xcc, 0x0d,
    0x00, 0x0b, 0x03, 0x73, 0x00, 0x83, 0x00, 0x0c, 0x00, 0x0d, 0x00, 0x08,
    0x11, 0x1f, 0x88, 0x89, 0x00, 0x0e, 0xdc, 0xcc, 0x6e, 0xe6, 0xdd, 0xdd,
    0xd9, 0x99, 0xbb, 0xbb, 0x67, 0x63, 0x6e, 0x0e, 0xec, 0xcc, 0xdd, 0xdc,
    0x99, 0x9f, 0xbb, 0xb9, 0x33, 0x3e, 0x3c, 0x42, 0xb9, 0xa5, 0xb9, 0xa5,
    0x42, 0x3c, 0x58, 0x43, 0xe0, 0x70, 0x3e, 0xfc, 0xe0, 0x47, 0xcd, 0x75,
    0x02, 0xcd, 0x00, 0x02, 0x26, 0xd0, 0xcd, 0x03, 0x02, 0x21, 0x00, 0xfe,
    0x0e, 0xa0, 0xaf, 0x22, 0x0d, 0x20, 0xfc, 0x11, 0x04, 0x01, 0x21, 0x10,
    0x80, 0x4c, 0x1a, 0xe2, 0x0c, 0xcd, 0xc6, 0x03, 0xcd, 0xc7, 0x03, 0x13,
    0x7b, 0xfe, 0x34, 0x20, 0xf1, 0x11, 0x72, 0x00, 0x06, 0x08, 0x1a, 0x13,
    0x22, 0x23, 0x05, 0x20, 0xf9, 0xcd, 0xf0, 0x03, 0x3e, 0x01, 0xe0, 0x4f,
    0x3e, 0x91, 0xe0, 0x40, 0x21, 0xb2, 0x98, 0x06, 0x4e, 0x0e, 0x44, 0xcd,
    0x91, 0x02, 0xaf, 0xe0, 0x4f, 0x0e, 0x80, 0x21, 0x42, 0x00, 0x06, 0x18,
    0xf2, 0x0c, 0xbe, 0x20, 0xfe, 0x23, 0x05, 0x20, 0xf7, 0x21, 0x34, 0x01,
    0x06, 0x19, 0x78, 0x86, 0x2c, 0x05, 0x20, 0xfb, 0x86, 0x20, 0xfe, 0xcd,
    0x1c, 0x03, 0x18, 0x02, 0x00, 0x00, 0xcd, 0xd0, 0x05, 0xaf, 0xe0, 0x70,
    0x3e, 0x11, 0xe0, 0x50, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x21, 0x00, 0x80, 0xaf,
    0x22, 0xcb, 0x6c, 0x28, 0xfb, 0xc9, 0x2a, 0x12, 0x13, 0x0d, 0x20, 0xfa,
    0xc9, 0xe5, 0x21, 0x0f, 0xff, 0xcb, 0x86, 0xcb, 0x46, 0x28, 0xfc, 0xe1,
    0xc9, 0x11, 0x00, 0xff, 0x21, 0x03, 0xd0, 0x0e, 0x0f, 0x3e, 0x30, 0x12,
    0x3e, 0x20, 0x12, 0x1a, 0x2f, 0xa1, 0xcb, 0x37, 0x47, 0x3e, 0x10, 0x12,
    0x1a, 0x2f, 0xa1, 0xb0, 0x4f, 0x7e, 0xa9, 0xe6, 0xf0, 0x47, 0x2a, 0xa9,
    0xa1, 0xb0, 0x32, 0x47, 0x79, 0x77, 0x3e, 0x30, 0x12, 0xc9, 0x3e, 0x80,
    0xe0, 0x68, 0xe0, 0x6a, 0x0e, 0x6b, 0x2a, 0xe2, 0x05, 0x20, 0xfb, 0x4a,
    0x09, 0x43, 0x0e, 0x69, 0x2a, 0xe2, 0x05, 0x20, 0xfb, 0xc9, 0xc5, 0xd5,
    0xe5, 0x21, 0x00, 0xd8, 0x06, 0x01, 0x16, 0x3f, 0x1e, 0x40, 0xcd, 0x4a,
    0x02, 0xe1, 0xd1, 0xc1, 0xc9, 0x3e, 0x80, 0xe0, 0x26, 0xe0, 0x11, 0x3e,
    0xf3, 0xe0, 0x12, 0xe0, 0x25, 0x3e, 0x77, 0xe0, 0x24, 0x21, 0x30, 0xff,
    0xaf, 0x0e, 0x10, 0x22, 0x2f, 0x0d, 0x20, 0xfb, 0xc9, 0xcd, 0x11, 0x02,
    0xcd, 0x62, 0x02, 0x79, 0xfe, 0x38, 0x20, 0x14, 0xe5, 0xaf, 0xe0, 0x4f,
    0x21, 0xa7, 0x99, 0x3e, 0x38, 0x22, 0x3c, 0xfe, 0x3f, 0x20, 0xfa, 0x3e,
    0x01, 0xe0, 0x4f, 0xe1, 0xc5, 0xe5, 0x21, 0x43, 0x01, 0xcb, 0x7e, 0xcc,
    0x89, 0x05, 0xe1, 0xc1, 0xcd, 0x11, 0x02, 0x79, 0xd6, 0x30, 0xd2, 0x06,
    0x03, 0x79, 0xfe, 0x01, 0xca, 0x06, 0x03, 0x7d, 0xfe, 0xd1, 0x28, 0x21,
    0xc5, 0x06, 0x03, 0x0e, 0x01, 0x16, 0x03, 0x7e, 0xe6, 0xf8, 0xb1, 0x22,
    0x15, 0x20, 0xf8, 0x0c, 0x79, 0xfe, 0x06, 0x20, 0xf0, 0x11, 0x11, 0x00,
    0x19, 0x05, 0x20, 0xe7, 0x11, 0xa1, 0xff, 0x19, 0xc1, 0x04, 0x78, 0x1e,
    0x83, 0xfe, 0x62, 0x28, 0x06, 0x1e, 0xc1, 0xfe, 0x64, 0x20, 0x07, 0x7b,
    0xe0, 0x13, 0x3e, 0x87, 0xe0, 0x14, 0xfa, 0x02, 0xd0, 0xfe, 0x00, 0x28,
    0x0a, 0x3d, 0xea, 0x02, 0xd0, 0x79, 0xfe, 0x01, 0xca, 0x91, 0x02, 0x0d,
    0xc2, 0x91, 0x02, 0xc9, 0x0e, 0x26, 0xcd, 0x4a, 0x03, 0xcd, 0x11, 0x02,
    0xcd, 0x62, 0x02, 0x0d, 0x20, 0xf4, 0xcd, 0x11, 0x02, 0x3e, 0x01, 0xe0,
    0x4f, 0xcd, 0x3e, 0x03, 0xcd, 0x41, 0x03, 0xaf, 0xe0, 0x4f, 0xcd, 0x3e,
    0x03, 0xc9, 0x21, 0x08, 0x00, 0x11, 0x51, 0xff, 0x0e, 0x05, 0xcd, 0x0a,
    0x02, 0xc9, 0xc5, 0xd5, 0xe5, 0x21, 0x40, 0xd8, 0x0e, 0x20, 0x7e, 0xe6,
    0x1f, 0xfe, 0x1f, 0x28, 0x01, 0x3c, 0x57, 0x2a, 0x07, 0x07, 0x07, 0xe6,
    0x07, 0x47, 0x3a, 0x07, 0x07, 0x07, 0xe6, 0x18, 0xb0, 0xfe, 0x1f, 0x28,
    0x01, 0x3c, 0x0f, 0x0f, 0x0f, 0x47, 0xe6, 0xe0, 0xb2, 0x22, 0x78, 0xe6,
    0x03, 0x5f, 0x7e, 0x0f, 0x0f, 0xe6, 0x1f, 0xfe, 0x1f, 0x28, 0x01, 0x3c,
    0x07, 0x07, 0xb3, 0x22, 0x0d, 0x20, 0xc7, 0xe1, 0xd1, 0xc1, 0xc9, 0x0e,
    0x00, 0x1a, 0xe6, 0xf0, 0xcb, 0x49, 0x28, 0x02, 0xcb, 0x37, 0x47, 0x23,
    0x7e, 0xb0, 0x22, 0x1a, 0xe6, 0x0f, 0xcb, 0x49, 0x20, 0x02, 0xcb, 0x37,
    0x47, 0x23, 0x7e, 0xb0, 0x22, 0x13, 0xcb, 0x41, 0x28, 0x0d, 0xd5, 0x11,
    0xf8, 0xff, 0xcb, 0x49, 0x28, 0x03, 0x11, 0x08, 0x00, 0x19, 0xd1, 0x0c,
    0x79, 0xfe, 0x18, 0x20, 0xcc, 0xc9, 0x47, 0xd5, 0x16, 0x04, 0x58, 0xcb,
    0x10, 0x17, 0xcb, 0x13, 0x17, 0x15, 0x20, 0xf6, 0xd1, 0x22, 0x23, 0x22,
    0x23, 0xc9, 0x3e, 0x19, 0xea, 0x10, 0x99, 0x21, 0x2f, 0x99, 0x0e, 0x0c,
    0x3d, 0x28, 0x08, 0x32, 0x0d, 0x20, 0xf9, 0x2e, 0x0f, 0x18, 0xf3, 0xc9,
    0x3e, 0x01, 0xe0, 0x4f, 0xcd, 0x00, 0x02, 0x11, 0x07, 0x06, 0x21, 0x80,
    0x80, 0x0e, 0xc0, 0x1a, 0x22, 0x23, 0x22, 0x23, 0x13, 0x0d, 0x20, 0xf7,
    0x11, 0x04, 0x01, 0xcd, 0x8f, 0x03, 0x01, 0xa8, 0xff, 0x09, 0xcd, 0x8f,
    0x03, 0x01, 0xf8, 0xff, 0x09, 0x11, 0x72, 0x00, 0x0e, 0x08, 0x23, 0x1a,
    0x22, 0x13, 0x0d, 0x20, 0xf9, 0x21, 0xc2, 0x98, 0x06, 0x08, 0x3e, 0x08,
    0x0e, 0x10, 0x22, 0x0d, 0x20, 0xfc, 0x11, 0x10, 0x00, 0x19, 0x05, 0x20,
    0xf3, 0xaf, 0xe0, 0x4f, 0x21, 0xc2, 0x98, 0x3e, 0x08, 0x22, 0x3c, 0xfe,
    0x18, 0x20, 0x02, 0x2e, 0xe2, 0xfe, 0x28, 0x20, 0x03, 0x21, 0x02, 0x99,
    0xfe, 0x38, 0x20, 0xed, 0x21, 0xd8, 0x08, 0x11, 0x40, 0xd8, 0x06, 0x08,
    0x3e, 0xff, 0x12, 0x13, 0x12, 0x13, 0x0e, 0x02, 0xcd, 0x0a, 0x02, 0x3e,
    0x00, 0x12, 0x13, 0x12, 0x13, 0x13, 0x13, 0x05, 0x20, 0xea, 0xcd, 0x62,
    0x02, 0x21, 0x4b, 0x01, 0x7e, 0xfe, 0x33, 0x20, 0x0b, 0x2e, 0x44, 0x1e,
    0x30, 0x2a, 0xbb, 0x20, 0x49, 0x1c, 0x18, 0x04, 0x2e, 0x4b, 0x1e, 0x01,
    0x2a, 0xbb, 0x20, 0x3e, 0x2e, 0x34, 0x01, 0x10, 0x00, 0x2a, 0x80, 0x47,
    0x0d, 0x20, 0xfa, 0xea, 0x00, 0xd0, 0x21, 0xc7, 0x06, 0x0e, 0x00, 0x2a,
    0xb8, 0x28, 0x08, 0x0c, 0x79, 0xfe, 0x4f, 0x20, 0xf6, 0x18, 0x1f, 0x79,
    0xd6, 0x41, 0x38, 0x1c, 0x21, 0x16, 0x07, 0x16, 0x00, 0x5f, 0x19, 0xfa,
    0x37, 0x01, 0x57, 0x7e, 0xba, 0x28, 0x0d, 0x11, 0x0e, 0x00, 0x19, 0x79,
    0x83, 0x4f, 0xd6, 0x5e, 0x38, 0xed, 0x0e, 0x00, 0x21, 0x33, 0x07, 0x06,
    0x00, 0x09, 0x7e, 0xe6, 0x1f, 0xea, 0x08, 0xd0, 0x7e, 0xe6, 0xe0, 0x07,
    0x07, 0x07, 0xea, 0x0b, 0xd0, 0xcd, 0xe9, 0x04, 0xc9, 0x11, 0x91, 0x07,
    0x21, 0x00, 0xd9, 0xfa, 0x0b, 0xd0, 0x47, 0x0e, 0x1e, 0xcb, 0x40, 0x20,
    0x02, 0x13, 0x13, 0x1a, 0x22, 0x20, 0x02, 0x1b, 0x1b, 0xcb, 0x48, 0x20,
    0x02, 0x13, 0x13, 0x1a, 0x22, 0x13, 0x13, 0x20, 0x02, 0x1b, 0x1b, 0xcb,
    0x50, 0x28, 0x05, 0x1b, 0x2b, 0x1a, 0x22, 0x13, 0x1a, 0x22, 0x13, 0x0d,
    0x20, 0xd7, 0x21, 0x00, 0xd9, 0x11, 0x00, 0xda, 0xcd, 0x64, 0x05, 0xc9,
    0x21, 0x12, 0x00, 0xfa, 0x05, 0xd0, 0x07, 0x07, 0x06, 0x00, 0x4f, 0x09,
    0x11, 0x40, 0xd8, 0x06, 0x08, 0xe5, 0x0e, 0x02, 0xcd, 0x0a, 0x02, 0x13,
    0x13, 0x13, 0x13, 0x13, 0x13, 0xe1, 0x05, 0x20, 0xf0, 0x11, 0x42, 0xd8,
    0x0e, 0x02, 0xcd, 0x0a, 0x02, 0x11, 0x4a, 0xd8, 0x0e, 0x02, 0xcd, 0x0a,
    0x02, 0x2b, 0x2b, 0x11, 0x44, 0xd8, 0x0e, 0x02, 0xcd, 0x0a, 0x02, 0xc9,
    0x0e, 0x60, 0x2a, 0xe5, 0xc5, 0x21, 0xe8, 0x07, 0x06, 0x00, 0x4f, 0x09,
    0x0e, 0x08, 0xcd, 0x0a, 0x02, 0xc1, 0xe1, 0x0d, 0x20, 0xec, 0xc9, 0xfa,
    0x08, 0xd0, 0x11, 0x18, 0x00, 0x3c, 0x3d, 0x28, 0x03, 0x19, 0x20, 0xfa,
    0xc9, 0xcd, 0x1d, 0x02, 0x78, 0xe6, 0xff, 0x28, 0x0f, 0x21, 0xe4, 0x08,
    0x06, 0x00, 0x2a, 0xb9, 0x28, 0x08, 0x04, 0x78, 0xfe, 0x0c, 0x20, 0xf6,
    0x18, 0x2d, 0x78, 0xea, 0x05, 0xd0, 0x3e, 0x1e, 0xea, 0x02, 0xd0, 0x11,
    0x0b, 0x00, 0x19, 0x56, 0x7a, 0xe6, 0x1f, 0x5f, 0x21, 0x08, 0xd0, 0x3a,
    0x22, 0x7b, 0x77, 0x7a, 0xe6, 0xe0, 0x07, 0x07, 0x07, 0x5f, 0x21, 0x0b,
    0xd0, 0x3a, 0x22, 0x7b, 0x77, 0xcd, 0xe9, 0x04, 0xcd, 0x28, 0x05, 0xc9,
    0xcd, 0x11, 0x02, 0xfa, 0x43, 0x01, 0xcb, 0x7f, 0x28, 0x04, 0xe0, 0x4c,
    0x18, 0x28, 0x3e, 0x04, 0xe0, 0x4c, 0x3e, 0x01, 0xe0, 0x6c, 0x21, 0x00,
    0xda, 0xcd, 0x7b, 0x05, 0x06, 0x10, 0x16, 0x00, 0x1e, 0x08, 0xcd, 0x4a,
    0x02, 0x21, 0x7a, 0x00, 0xfa, 0x00, 0xd0, 0x47, 0x0e, 0x02, 0x2a, 0xb8,
    0xcc, 0xda, 0x03, 0x0d, 0x20, 0xf8, 0xc9, 0x01, 0x0f, 0x3f, 0x7e, 0xff,
    0xff, 0xc0, 0x00, 0xc0, 0xf0, 0xf1, 0x03, 0x7c, 0xfc, 0xfe, 0xfe, 0x03,
    0x07, 0x07, 0x0f, 0xe0, 0xe0, 0xf0, 0xf0, 0x1e, 0x3e, 0x7e, 0xfe, 0x0f,
    0x0f, 0x1f, 0x1f, 0xff, 0xff, 0x00, 0x00, 0x01, 0x01, 0x01, 0x03, 0xff,
    0xff, 0xe1, 0xe0, 0xc0, 0xf0, 0xf9, 0xfb, 0x1f, 0x7f, 0xf8, 0xe0, 0xf3,
    0xfd, 0x3e, 0x1e, 0xe0, 0xf0, 0xf9, 0x7f, 0x3e, 0x7c, 0xf8, 0xe0, 0xf8,
    0xf0, 0xf0, 0xf8, 0x00, 0x00, 0x7f, 0x7f, 0x07, 0x0f, 0x9f, 0xbf, 0x9e,
    0x1f, 0xff, 0xff, 0x0f, 0x1e, 0x3e, 0x3c, 0xf1, 0xfb, 0x7f, 0x7f, 0xfe,
    0xde, 0xdf, 0x9f, 0x1f, 0x3f, 0x3e, 0x3c, 0xf8, 0xf8, 0x00, 0x00, 0x03,
    0x03, 0x07, 0x07, 0xff, 0xff, 0xc1, 0xc0, 0xf3, 0xe7, 0xf7, 0xf3, 0xc0,
    0xc0, 0xc0, 0xc0, 0x1f, 0x1f, 0x1e, 0x3e, 0x3f, 0x1f, 0x3e, 0x3e, 0x80,
    0x00, 0x00, 0x00, 0x7c, 0x1f, 0x07, 0x00, 0x0f, 0xff, 0xfe, 0x00, 0x7c,
    0xf8, 0xf0, 0x00, 0x1f, 0x0f, 0x0f, 0x00, 0x7c, 0xf8, 0xf8, 0x00, 0x3f,
    0x3e, 0x1c, 0x00, 0x0f, 0x0f, 0x0f, 0x00, 0x7c, 0xff, 0xff, 0x00, 0x00,
    0xf8, 0xf8, 0x00, 0x07, 0x0f, 0x0f, 0x00, 0x81, 0xff, 0xff, 0x00, 0xf3,
    0xe1, 0x80, 0x00, 0xe0, 0xff, 0x7f, 0x00, 0xfc, 0xf0, 0xc0, 0x00, 0x3e,
    0x7c, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x88, 0x16, 0x36, 0xd1,
    0xdb, 0xf2, 0x3c, 0x8c, 0x92, 0x3d, 0x5c, 0x58, 0xc9, 0x3e, 0x70, 0x1d,
    0x59, 0x69, 0x19, 0x35, 0xa8, 0x14, 0xaa, 0x75, 0x95, 0x99, 0x34, 0x6f,
    0x15, 0xff, 0x97, 0x4b, 0x90, 0x17, 0x10, 0x39, 0xf7, 0xf6, 0xa2, 0x49,
    0x4e, 0x43, 0x68, 0xe0, 0x8b, 0xf0, 0xce, 0x0c, 0x29, 0xe8, 0xb7, 0x86,
    0x9a, 0x52, 0x01, 0x9d, 0x71, 0x9c, 0xbd, 0x5d, 0x6d, 0x67, 0x3f, 0x6b,
    0xb3, 0x46, 0x28, 0xa5, 0xc6, 0xd3, 0x27, 0x61, 0x18, 0x66, 0x6a, 0xbf,
    0x0d, 0xf4, 0x42, 0x45, 0x46, 0x41, 0x41, 0x52, 0x42, 0x45, 0x4b, 0x45,
    0x4b, 0x20, 0x52, 0x2d, 0x55, 0x52, 0x41, 0x52, 0x20, 0x49, 0x4e, 0x41,
    0x49, 0x4c, 0x49, 0x43, 0x45, 0x20, 0x52, 0x7c, 0x08, 0x12, 0xa3, 0xa2,
    0x07, 0x87, 0x4b, 0x20, 0x12, 0x65, 0xa8, 0x16, 0xa9, 0x86, 0xb1, 0x68,
    0xa0, 0x87, 0x66, 0x12, 0xa1, 0x30, 0x3c, 0x12, 0x85, 0x12, 0x64, 0x1b,
    0x07, 0x06, 0x6f, 0x6e, 0x6e, 0xae, 0xaf, 0x6f, 0xb2, 0xaf, 0xb2, 0xa8,
    0xab, 0x6f, 0xaf, 0x86, 0xae, 0xa2, 0xa2, 0x12, 0xaf, 0x13, 0x12, 0xa1,
    0x6e, 0xaf, 0xaf, 0xad, 0x06, 0x4c, 0x6e, 0xaf, 0xaf, 0x12, 0x7c, 0xac,
    0xa8, 0x6a, 0x6e, 0x13, 0xa0, 0x2d, 0xa8, 0x2b, 0xac, 0x64, 0xac, 0x6d,
    0x87, 0xbc, 0x60, 0xb4, 0x13, 0x72, 0x7c, 0xb5, 0xae, 0xae, 0x7c, 0x7c,
    0x65, 0xa2, 0x6c, 0x64, 0x85, 0x80, 0xb0, 0x40, 0x88, 0x20, 0x68, 0xde,
    0x00, 0x70, 0xde, 0x20, 0x78, 0x20, 0x20, 0x38, 0x20, 0xb0, 0x90, 0x20,
    0xb0, 0xa0, 0xe0, 0xb0, 0xc0, 0x98, 0xb6, 0x48, 0x80, 0xe0, 0x50, 0x1e,
    0x1e, 0x58, 0x20, 0xb8, 0xe0, 0x88, 0xb0, 0x10, 0x20, 0x00, 0x10, 0x20,
    0xe0, 0x18, 0xe0, 0x18, 0x00, 0x18, 0xe0, 0x20, 0xa8, 0xe0, 0x20, 0x18,
    0xe0, 0x00, 0x20, 0x18, 0xd8, 0xc8, 0x18, 0xe0, 0x00, 0xe0, 0x40, 0x28,
    0x28, 0x28, 0x18, 0xe0, 0x60, 0x20, 0x18, 0xe0, 0x00, 0x00, 0x08, 0xe0,
    0x18, 0x30, 0xd0, 0xd0, 0xd0, 0x20, 0xe0, 0xe8, 0xff, 0x7f, 0xbf, 0x32,
    0xd0, 0x00, 0x00, 0x00, 0x9f, 0x63, 0x79, 0x42, 0xb0, 0x15, 0xcb, 0x04,
    0xff, 0x7f, 0x31, 0x6e, 0x4a, 0x45, 0x00, 0x00, 0xff, 0x7f, 0xef, 0x1b,
    0x00, 0x02, 0x00, 0x00, 0xff, 0x7f, 0x1f, 0x42, 0xf2, 0x1c, 0x00, 0x00,
    0xff, 0x7f, 0x94, 0x52, 0x4a, 0x29, 0x00, 0x00, 0xff, 0x7f, 0xff, 0x03,
    0x2f, 0x01, 0x00, 0x00, 0xff, 0x7f, 0xef, 0x03, 0xd6, 0x01, 0x00, 0x00,
    0xff, 0x7f, 0xb5, 0x42, 0xc8, 0x3d, 0x00, 0x00, 0x74, 0x7e, 0xff, 0x03,
    0x80, 0x01, 0x00, 0x00, 0xff, 0x67, 0xac, 0x77, 0x13, 0x1a, 0x6b, 0x2d,
    0xd6, 0x7e, 0xff, 0x4b, 0x75, 0x21, 0x00, 0x00, 0xff, 0x53, 0x5f, 0x4a,
    0x52, 0x7e, 0x00, 0x00, 0xff, 0x4f, 0xd2, 0x7e, 0x4c, 0x3a, 0xe0, 0x1c,
    0xed, 0x03, 0xff, 0x7f, 0x5f, 0x25, 0x00, 0x00, 0x6a, 0x03, 0x1f, 0x02,
    0xff, 0x03, 0xff, 0x7f, 0xff, 0x7f, 0xdf, 0x01, 0x12, 0x01, 0x00, 0x00,
    0x1f, 0x23, 0x5f, 0x03, 0xf2, 0x00, 0x09, 0x00, 0xff, 0x7f, 0xea, 0x03,
    0x1f, 0x01, 0x00, 0x00, 0x9f, 0x29, 0x1a, 0x00, 0x0c, 0x00, 0x00, 0x00,
    0xff, 0x7f, 0x7f, 0x02, 0x1f, 0x00, 0x00, 0x00, 0xff, 0x7f, 0xe0, 0x03,
    0x06, 0x02, 0x20, 0x01, 0xff, 0x7f, 0xeb, 0x7e, 0x1f, 0x00, 0x00, 0x7c,
    0xff, 0x7f, 0xff, 0x3f, 0x00, 0x7e, 0x1f, 0x00, 0xff, 0x7f, 0xff, 0x03,
    0x1f, 0x00, 0x00, 0x00, 0xff, 0x03, 0x1f, 0x00, 0x0c, 0x00, 0x00, 0x00,
    0xff, 0x7f, 0x3f, 0x03, 0x93, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x42,
    0x7f, 0x03, 0xff, 0x7f, 0xff, 0x7f, 0x8c, 0x7e, 0x00, 0x7c, 0x00, 0x00,
    0xff, 0x7f, 0xef, 0x1b, 0x80, 0x61, 0x00, 0x00, 0xff, 0x7f, 0x00, 0x7c,
    0xe0, 0x03, 0x1f, 0x7c, 0x1f, 0x00, 0xff, 0x03, 0x40, 0x41, 0x42, 0x20,
    0x21, 0x22, 0x80, 0x81, 0x82, 0x10, 0x11, 0x12, 0x12, 0xb0, 0x79, 0xb8,
    0xad, 0x16, 0x17, 0x07, 0xba, 0x05, 0x7c, 0x13, 0x00, 0x00, 0x00, 0x00};

uint8_t mmu_read(uint16_t addr) {
    if (!cpu.memory.finished_boot &&
        (addr < 0x100 || (addr >= 0x200 && addr < 0x900))) {
        return boot_rom[addr];
    } else if (addr < 0x8000) {
        return cpu.memory.read(addr);
    } else if (addr < 0xa000) {
        return cpu.memory
            .vram[(cpu.memory.select_upper_vram * 0x2000) + addr % 0x2000];
    } else if (addr < 0xc000) {
        return cpu.memory.read(addr);
    } else if (addr < 0xd000) {
        return cpu.memory.wram[addr % 0x1000];
    } else if (addr < 0xe000) {
        return cpu.memory
            .wram[((cpu.memory.wram_number ? cpu.memory.wram_number : 1) *
                   0x1000) +
                  addr % 0x1000];
    } else if (addr < 0xf000) {
        return cpu.memory.wram[addr % 0x1000];
    } else if (addr < 0xfe00) {
        return cpu.memory
            .wram[((cpu.memory.wram_number ? cpu.memory.wram_number : 1) *
                   0x1000) +
                  addr % 0x1000];
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
        case 0xff05:
            return cpu.memory.timer_counter;
        case 0xff0f:
            return (cpu.memory.vblank_if << 0) | (cpu.memory.lcd_if << 1) |
                   (cpu.memory.timer_if << 2) | (cpu.memory.serial_if << 3) |
                   (cpu.memory.joypad_if << 4);
        case 0xff11:
            return apu.ch1.initial_length_timer | (apu.ch1.wave_duty << 6);
        case 0xff14:
            return (apu.ch1.enable << 7) | (apu.ch1.length_enable << 6) |
                   (apu.ch1.period_low);
        case 0xff16:
            return apu.ch2.initial_length_timer | (apu.ch2.wave_duty << 6);
        case 0xff19:
            return (apu.ch2.enable << 7) | (apu.ch2.length_enable << 6) |
                   (apu.ch2.period_low);
        case 0xff1a:
            return apu.ch3.dac_on << 7;
        case 0xff1e:
            return (apu.ch3.enable << 7) | (apu.ch3.length_enable << 6) |
                   (apu.ch3.period_low);
        case 0xff22:
            return (apu.ch4.clock_shift << 4) | (apu.ch4.narrow_lfsr << 3) |
                   ((uint8_t)apu.ch4.clock_divider);
        case 0xff23:
            return (apu.ch4.enable << 7) | (apu.ch4.length_enable << 6);
        case 0xff24:
            return (apu.vin_left << 7) | (apu.vol_left << 4) |
                   (apu.vin_right << 3) | (apu.vol_right << 0);
        case 0xff25:
            return (apu.ch4.pan_left << 7) | (apu.ch3.pan_left << 6) |
                   (apu.ch2.pan_left << 5) | (apu.ch1.pan_left << 4) |
                   (apu.ch4.pan_right << 3) | (apu.ch3.pan_right << 2) |
                   (apu.ch2.pan_right << 1) | (apu.ch1.pan_right << 0);
        case 0xff26:
            return (apu.audio_enable << 7) | (apu.ch4.enable << 3) |
                   (apu.ch3.enable << 2) | (apu.ch2.enable << 1) |
                   (apu.ch1.enable << 0);
        case 0xff4d:
            return cpu.fast_mode << 7;
        case 0xff4f:
            return cpu.memory.select_upper_vram;
        case 0xff68:
        case 0xff69:
        case 0xff6a:
        case 0xff6b:
            printf("Read from CGB register 0x%x\n", addr);
            return 0;
        case 0xff40:
            return (ppu.bg_window_enable << 0) | (ppu.enable_objects << 1) |
                   (ppu.large_objects << 2) | (ppu.bg_tile_map_location << 3) |
                   (ppu.bg_window_tile_data_location << 4) |
                   (ppu.window_enable << 5) |
                   (ppu.window_tile_map_location << 6) | (ppu.ppu_enable << 7);
        case 0xff41:
            return (ppu.ppu_enable ? ppu.mode : 0) |
                   ((ppu.ly == ppu.lyc) << 2) | (ppu.mode_0_int << 3) |
                   (ppu.mode_1_int << 4) | (ppu.mode_2_int << 5) |
                   (ppu.lyc_int << 6);
        case 0xff42:
            return ppu.scroll_y;
        case 0xff44:
            return ppu.ly;
        case 0xff47:
            return ppu.bg_color_reg;
        case 0xff48:
            return ppu.obj_color_1_reg;
        case 0xff49:
            return ppu.obj_color_2_reg;
        case 0xff4a:
            return ppu.wy;
        case 0xff4b:
            return ppu.wx;
        case 0xff55:
            return cpu.memory.hblank_dma_reg;
        case 0xff70:
            return cpu.memory.wram_number;
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

    if (!cpu.memory.finished_boot &&
        (addr < 0x100 || (addr >= 0x200 && addr < 0x900))) {
        boot_rom[addr] = value;
    } else if (addr < 0x8000) {
        cpu.memory.write(addr, value);
    } else if (addr < 0xa000) {
        cpu.memory
            .vram[(cpu.memory.select_upper_vram * 0x2000) + addr % 0x2000] =
            value;
    } else if (addr < 0xc000) {
        cpu.memory.write(addr, value);
    } else if (addr < 0xd000) {
        cpu.memory.wram[addr % 0x1000] = value;
    } else if (addr < 0xe000) {
        cpu.memory.wram[((cpu.memory.wram_number ? cpu.memory.wram_number : 1) *
                         0x1000) +
                        addr % 0x1000] = value;
    } else if (addr < 0xf000) {
        cpu.memory.wram[addr % 0x1000] = value;
    } else if (addr < 0xfe00) {
        cpu.memory.wram[((cpu.memory.wram_number ? cpu.memory.wram_number : 1) *
                         0x1000) +
                        addr % 0x1000] = value;
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
        case 0xff05:
            cpu.memory.timer_counter = value;
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
        case 0xff15:
            // this page intentionally left blank
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
            if (apu.ch3.output_level == 0)
                apu.ch3.volume = 0;
            else
                apu.ch3.volume = 4 >> (apu.ch3.output_level - 1);
            break;
        case 0xff1d:
            ch3_period_low(value);
            break;
        case 0xff1e:
            ch3_period_high_control(value);
            break;
        case 0xff1f:
            // this page intentionally left blank
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
        case 0xff45:
            ppu.lyc = value;
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
        case 0xff4c:
            if (value & 0b100 && !cpu.memory.finished_boot)
                cpu.compatibility_mode = true;
            break;
        case 0xff4d:
            if (value & 1)
                cpu.speed_switch_pending = true;
            break;
        case 0xff4f:
            cpu.memory.select_upper_vram = value & 1;
            break;
        case 0xff50:
            cpu.memory.finished_boot = true;
            break;
        case 0xff51:
            cpu.memory.vram_dma_src &= 0xff;
            cpu.memory.vram_dma_src |= value << 8;
            break;
        case 0xff52:
            cpu.memory.vram_dma_src &= 0xff00;
            cpu.memory.vram_dma_src |= value & 0xf0;
            break;
        case 0xff53:
            cpu.memory.vram_dma_dst &= 0xff;
            cpu.memory.vram_dma_dst |= ((value & 0x1f) | 0x80) << 8;
            break;
        case 0xff54:
            cpu.memory.vram_dma_dst &= 0xff00;
            cpu.memory.vram_dma_dst |= value & 0xf0;
            break;
        case 0xff55:
            if (value & 0x80) {
                cpu.memory.hblank_dma_active = true;
                cpu.memory.hblank_dma_remaining = ((value & 0x7f) + 1) * 16;
                cpu.memory.hblank_dma_reg = value & 0x7f;

                // <hack>
                // for some reason HBlank-based VRAM OAM transfer is too
                // slow by one iteration, meaning that without the snippet below
                // games actively watching the process based on line numbers
                // will cancel the transfer 16 bytes early.
                for (uint8_t i = 0; i < 16; i++) {
                    write_8(cpu.memory.vram_dma_dst++,
                                read_8(cpu.memory.vram_dma_src++));
                    cpu.memory.hblank_dma_remaining--;
                }
                cpu.memory.hblank_dma_reg--;
                if (cpu.memory.hblank_dma_remaining == 0) {
                    cpu.memory.hblank_dma_active = false;
                }
                cpu.remaining_cycles -= 4;
                // </hack>
            } else if (!cpu.memory.hblank_dma_active) {
                uint16_t to_transfer = ((value & 0x7f) + 1) * 16;
                while (to_transfer--) {
                    write_8(cpu.memory.vram_dma_dst++,
                            read_8(cpu.memory.vram_dma_src++));
                }
                cpu.memory.hblank_dma_reg = 0xff;
            } else {
                cpu.memory.hblank_dma_active = false;
                cpu.memory.hblank_dma_reg =
                    (cpu.memory.hblank_dma_remaining / 16) - 1;
                cpu.memory.hblank_dma_reg |= 0x80;
            }
            break;
        case 0xff56:
            printf("Attempted to write to infrared register\n");
            break;
        case 0xff68:
            ppu.cgb_bg_color_increment = value & 0x80;
            ppu.cgb_bg_address = value & 0x3f;
            break;
        case 0xff69:
            if (ppu.mode != 3) {
                uint32_t old_color =
                    ppu.cgb_bg_color_palettes[ppu.cgb_bg_address / 8]
                                             [(ppu.cgb_bg_address % 8) / 2];
                if (ppu.cgb_bg_address % 2) {
                    old_color &= 0x3fff;
                    old_color |= (value & 0x3) << 14;
                    old_color |= ((value >> 2) & 0x1f) << 19;
                    old_color |= 0xff000000;
                } else {
                    old_color &= ~0x3fff;
                    old_color |= (value & 0x1f) << 3;
                    old_color |= (value >> 5) << 11;
                    old_color |= 0xff000000;
                }
                ppu.cgb_bg_color_palettes[ppu.cgb_bg_address / 8]
                                         [(ppu.cgb_bg_address % 8) / 2] =
                    old_color;
            }
            if (ppu.cgb_bg_color_increment)
                ppu.cgb_bg_address++;
            break;
        case 0xff6a:
            ppu.cgb_obj_color_increment = value & 0x80;
            ppu.cgb_obj_address = value & 0x3f;
            break;
        case 0xff6b:
            if (ppu.mode != 3) {
                uint32_t old_color =
                    ppu.cgb_obj_color_palettes[ppu.cgb_obj_address / 8]
                                              [(ppu.cgb_obj_address % 8) / 2];
                if (ppu.cgb_obj_address % 2) {
                    old_color &= 0x3fff;
                    old_color |= (value & 0x3) << 14;
                    old_color |= ((value >> 2) & 0x1f) << 19;
                    old_color |= 0xff000000;
                } else {
                    old_color &= ~0x3fff;
                    old_color |= (value & 0x1f) << 3;
                    old_color |= (value >> 5) << 11;
                    old_color |= 0xff000000;
                }
                ppu.cgb_obj_color_palettes[ppu.cgb_obj_address / 8]
                                          [(ppu.cgb_obj_address % 8) / 2] =
                    old_color;
            }
            if (ppu.cgb_obj_color_increment)
                ppu.cgb_obj_address++;
            break;
        case 0xff6c:
            printf("OAM priority changed, ignored\n");
            break;
        case 0xff70:
            cpu.memory.wram_number = value & 0b111;
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
