#include "cpu.h"
#include "cpu_mmu.h"
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

static bool get_status_bit(status_t bit) {
    return (cpu.f & (1 << (7 - bit))) != 0;
}

static bool resolve_cond(cond_t cond) {
    switch (cond) {
    case NZ_COND:
        return !get_status_bit(Z_STATUS);
    case Z_COND:
        return get_status_bit(Z_STATUS);
    case NC_COND:
        return !get_status_bit(C_STATUS);
    case C_COND:
        return get_status_bit(C_STATUS);
    default:
        UNREACHABLE_SWITCH(cond);
    }
}

static uint8_t read_8(uint16_t addr) { return mmu_read(addr); }

static void write_8(uint16_t addr, uint8_t val) { return mmu_write(addr, val); }

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

static void r8_write(r8_t dst, uint8_t val) {
    switch (dst) {
    case B:
        cpu.b = val;
        break;
    case C:
        cpu.c = val;
        break;
    case D:
        cpu.d = val;
        break;
    case E:
        cpu.e = val;
        break;
    case H:
        cpu.h = val;
        break;
    case L:
        cpu.l = val;
        break;
    case INDHL:
        write_8(r16_read(HL), val);
        break;
    case A:
        cpu.a = val;
        break;
    default:
        UNREACHABLE_SWITCH(dst);
    }
}

static uint8_t r16mem_read(r16_mem_t src) {
    switch (src) {
    case BC_MEM:
        return read_8(r16_read(BC));
    case DE_MEM:
        return read_8(r16_read(DE));
    case HLINC_MEM: {
        uint8_t ret = read_8(r16_read(HL));
        if (++cpu.l == 0)
            cpu.h++;
        return ret;
    }
    case HLDEC_MEM: {
        uint8_t ret = read_8(r16_read(HL));
        if (--cpu.l == 0xff)
            cpu.h--;
        return ret;
    }
    default:
        UNREACHABLE_SWITCH(src);
    }
}

static void r16mem_write(r16_mem_t dst, uint8_t val) {
    switch (dst) {
    case BC_MEM:
        write_8(r16_read(BC), val);
        break;
    case DE_MEM:
        write_8(r16_read(DE), val);
        break;
    case HLINC_MEM:
        write_8(r16_read(HL), val);
        if (++cpu.l == 0)
            cpu.h++;
        break;
    case HLDEC_MEM:
        write_8(r16_read(HL), val);
        if (--cpu.l == 0xff)
            cpu.h--;
        break;
    default:
        UNREACHABLE_SWITCH(dst);
    }
}

void execute(void) {
    uint8_t opcode = next_8();
    if (opcode == 0xcb) {
        opcode = next_8();
        if ((opcode >> 6) == 0) {
            switch ((opcode >> 3) & 0b111) {
            case 0:
                TODO("rlc r8");
                break;
            case 1:
                TODO("rrc r8");
                break;
            case 2:
                TODO("rl r8");
                break;
            case 3:
                TODO("rr r8");
                break;
            case 4:
                TODO("sla r8");
                break;
            case 5:
                TODO("sra r8");
                break;
            case 6:
                TODO("swap r8");
                break;
            case 7:
                TODO("srl r8");
                break;
            }
        } else {
            switch (opcode >> 6) {
            case 1: {
                bool result =
                    r8_read(opcode & 0b111) & (1 << ((opcode >> 3) & 0b111));
                set_status_bit(Z_STATUS, !result);
                printf("0x%04x\n", TO_U16(cpu.l, cpu.h));
                set_status_bit(N_STATUS, false);
                set_status_bit(H_STATUS, true);
                break;
            }
            case 2:
                r8_write(opcode & 0b111, r8_read(opcode & 0b111) &
                                             ~(1 << ((opcode >> 3) & 0b111)));
                break;
            case 3:
                r8_write(opcode & 0b111, r8_read(opcode & 0b111) |
                                             (1 << ((opcode >> 3) & 0b111)));
                break;
            default:
                UNREACHABLE_SWITCH(opcode);
            }
        }
    } else
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
            else if ((opcode & 0b11100111) == 0b100000) {
                if (resolve_cond((opcode >> 3) & 0b11))
                    cpu.pc += (((int16_t)next_8()) - 256);
                else
                    next_8();
            } else if ((opcode & 0b1000) == 0) {
                switch (opcode & 0b1111) {
                case 1:
                    r16_write((opcode >> 4) & 0b11, next_16());
                    break;
                case 2:
                    r16mem_write((opcode >> 4) & 0b11, cpu.a);
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
            if (opcode == 0b11000110)
                TODO("add a, imm8");
            else if (opcode == 0b11001110)
                TODO("adc a, imm8");
            else if (opcode == 0b11010110)
                TODO("sub a, imm8");
            else if (opcode == 0b11011110)
                TODO("sbc a, imm8");
            else if (opcode == 0b1110010)
                TODO("and a, imm8");
            else if (opcode == 0b11101110)
                TODO("xor a, imm8");
            else if (opcode == 0b11110110)
                TODO("or a, imm8");
            else if (opcode == 0b11111110)
                TODO("cp a, imm8");
            else if (opcode == 0b11001001)
                TODO("ret");
            else if (opcode == 0b11011001)
                TODO("reti");
            else if (opcode == 0b11000011)
                TODO("jp imm16");
            else if (opcode == 0b11101001)
                TODO("jp hl");
            else if (opcode == 0b11001101)
                TODO("call imm16");
            else if (opcode == 0b11100010)
                TODO("ldh [c], a");
            else if (opcode == 0b11100000)
                TODO("ldh [imm8], a");
            else if (opcode == 0b11101010)
                TODO("ld [imm16], a");
            else if (opcode == 0b11110010)
                TODO("ldh a, [c]");
            else if (opcode == 0b11110000)
                TODO("ldh a, [imm8]");
            else if (opcode == 0b11111010)
                TODO("ld a, [imm16]");
            else if (opcode == 0b11101000)
                TODO("add sp, imm8");
            else if (opcode == 0b11111000)
                TODO("ld hl, sp + imm8");
            else if (opcode == 0b11111001)
                TODO("ld sp, hl");
            else if (opcode == 0b11110011)
                TODO("di");
            else if (opcode == 0b11111011)
                TODO("ei");
            else
                switch (opcode & 0b111) {
                case 0:
                    TODO("ret cond");
                    break;
                case 1:
                    TODO("pop r16stk");
                    break;
                case 2:
                    TODO("jp cond, imm16");
                    break;
                case 4:
                    TODO("call cond, imm16");
                    break;
                case 5:
                    TODO("push r16stk");
                    break;
                case 7:
                    TODO("rst tgt3");
                    break;
                default:
                    UNREACHABLE_SWITCH(opcode);
                }
            break;
        default:
            UNREACHABLE_SWITCH(opcode >> 6);
        }
}
