#include "cpu.h"
#include "types.h"

extern cpu_t cpu;

// local helper functions

static void set_status_bit(status_t bit, bool value) {
    if (value) {
        cpu.f |= 1 << (7 - bit);
    } else {
        cpu.f &= ~(1 << (7 - bit));
    }
}

static uint8_t read_8(uint16_t addr) { return mmu_read(addr); }

static uint16_t read_16(uint16_t addr) {
    return TO_U16(read_8(addr), read_8(addr + 1));
}

static uint8_t next_8(void) { return read_8(cpu.pc++); }

static uint16_t next_16(void) {
    uint16_t ret = read_16(cpu.pc);
    cpu.pc += 2;
    return ret;
}

// register modify functions

static uint16_t r16_read(r16_t src) {
    switch (src) {
    case BC:
        return TO_U16(cpu.c, cpu.b);
    case DE:
        return TO_U16(cpu.e, cpu.d);
    case HL:
        return TO_U16(cpu.l, cpu.h);
    case SP:
        return cpu.sp;
    default:
        UNREACHABLE_SWITCH(src);
    }
}

static void r16_write(r16_t dst, uint16_t val) {
    switch (dst) {
    case BC:
        cpu.b = HIBYTE(val);
        cpu.c = LOBYTE(val);
        break;
    case DE:
        cpu.d = HIBYTE(val);
        cpu.e = LOBYTE(val);
        break;
    case HL:
        cpu.h = HIBYTE(val);
        cpu.l = LOBYTE(val);
        break;
    case SP:
        cpu.sp = val;
        break;
    }
}

static uint8_t r8_read(r8_t src) {
    switch (src) {
    case B:
        return cpu.b;
    case C:
        return cpu.c;
    case D:
        return cpu.d;
    case E:
        return cpu.e;
    case H:
        return cpu.h;
    case L:
        return cpu.l;
    case INDHL:
        return read_8(r16_read(HL));
    case A:
        return cpu.a;
    default:
        UNREACHABLE_SWITCH(src);
    }
}

void execute(void) {
    uint8_t opcode = next_8();
    switch (opcode >> 6) {
    case 0:
        if (opcode == 0b00000000)
            TODO("nop");
        else if (opcode == 0b00000111)
            TODO("rlca");
        else if (opcode == 0b00001000)
            TODO("ld [imm16], sp");
        else if (opcode == 0b00001111)
            TODO("rrca");
        else if (opcode == 0b00010000)
            TODO("stop");
        else if (opcode == 0b00010111)
            TODO("rla");
        else if (opcode == 0b00011000)
            TODO("jr imm8");
        else if (opcode == 0b00011111)
            TODO("rra");
        else if (opcode == 0b00100111)
            TODO("daa");
        else if (opcode == 0b00101111)
            TODO("cpl");
        else if (opcode == 0b00110111)
            TODO("scf");
        else if (opcode == 0b00111111)
            TODO("ccf");
        else if ((opcode & 0b11100111) == 0b100000)
            TODO("jr cond, imm8");
        else if ((opcode & 0b1000) == 0) {
            switch (opcode & 0b1111) {
            case 1:
                r16_write((opcode >> 4) & 0b11, next_16());
                break;
            case 2:
                TODO("ld [r16mem], a");
                break;
            case 3:
                TODO("inc r16");
                break;
            case 9:
                TODO("add hl, r16");
                break;
            case 10:
                TODO("ld a, [r16mem]");
                break;
            case 11:
                TODO("dec r16");
                break;
            }
        } else {
            switch (opcode & 0b11) {
            case 0:
                TODO("inc r8");
                break;
            case 1:
                TODO("dec r8");
                break;
            case 2:
                TODO("ld r8, imm8");
                break;
            default:
                UNREACHABLE_SWITCH(opcode & 0b11);
            }
        }
        break;
    case 1:
        TODO("1");
        break;
    case 2:
        switch ((opcode >> 3) & 0b111) {
        case 0:
            TODO("add a, r8");
            break;
        case 1:
            TODO("adc a, r8");
            break;
        case 2:
            TODO("sub a, r8");
            break;
        case 3:
            TODO("sbc a, r8");
            break;
        case 4:
            TODO("and a, r8");
            break;
        case 5: {
            cpu.a ^= r8_read(opcode & 0b111);
            set_status_bit(Z_STATUS, cpu.a == 0);
            set_status_bit(N_STATUS, false);
            set_status_bit(H_STATUS, false);
            set_status_bit(C_STATUS, false);
            break;
        }
        case 6:
            TODO("or a, r8");
            break;
        case 7:
            TODO("cp a, r8");
            break;
        }
        break;
    case 3:
        TODO("3");
        break;
    default:
        UNREACHABLE_SWITCH(opcode >> 6);
    }
}
