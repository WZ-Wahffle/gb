#include "cpu.h"
#include "cpu_mmu.h"
#include "types.h"

uint8_t cycle_lookup[] = {
    1, 3, 2, 2, 1, 1, 2, 1, 5, 2, 2, 2, 1, 1, 2, 1, 1, 3, 2, 2, 1, 1, 2, 1,
    3, 2, 2, 2, 1, 1, 2, 1, 2, 3, 2, 2, 1, 1, 2, 1, 2, 2, 2, 2, 1, 1, 2, 1,
    2, 3, 2, 2, 3, 3, 3, 1, 2, 2, 2, 2, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
    1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
    1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, 2, 2, 2, 2, 2, 2, 1, 2,
    1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
    1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
    1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
    2, 3, 3, 4, 3, 4, 2, 4, 2, 4, 3, 1, 3, 6, 2, 4, 2, 3, 3, 0, 3, 4, 2, 4,
    2, 4, 3, 0, 3, 0, 2, 4, 3, 3, 2, 0, 0, 4, 2, 4, 4, 1, 4, 0, 0, 0, 2, 4,
    3, 3, 2, 1, 0, 4, 2, 4, 3, 2, 4, 1, 0, 0, 2, 4,
};

uint8_t extended_lookup[] = {2, 2, 2, 2, 2, 2, 4, 2};

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

uint8_t read_8(uint16_t addr) { return mmu_read(addr); }

static void write_8(uint16_t addr, uint8_t val) { return mmu_write(addr, val); }

uint16_t read_16(uint16_t addr) {
    return TO_U16(read_8(addr), read_8(addr + 1));
}

static uint8_t next_8(void) { return read_8(cpu.pc++); }

static uint16_t next_16(void) {
    uint16_t ret = read_16(cpu.pc);
    cpu.pc += 2;
    return ret;
}

static void push_8(uint8_t val) { write_8(cpu.sp--, val); }

static void push_16(uint16_t val) {
    push_8(HIBYTE(val));
    push_8(LOBYTE(val));
}

static uint8_t pop_8(void) { return read_8(++cpu.sp); }

static uint16_t pop_16(void) {
    uint8_t lsb = pop_8();
    uint8_t msb = pop_8();
    return TO_U16(lsb, msb);
}

static void call(uint16_t addr) {
    push_16(cpu.pc);
    cpu.pc = addr;
}

static void ret(void) { cpu.pc = pop_16(); }

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

static uint16_t r16stk_read(r16_stk_t src) {
    switch (src) {
    case BC_STK:
        return TO_U16(cpu.c, cpu.b);
    case DE_STK:
        return TO_U16(cpu.e, cpu.d);
    case HL_STK:
        return TO_U16(cpu.l, cpu.h);
    case AF_STK:
        return TO_U16(cpu.f, cpu.a);
    default:
        UNREACHABLE_SWITCH(src);
    }
}

static void r16stk_write(r16_stk_t dst, uint16_t val) {
    switch (dst) {
    case BC_STK:
        r16_write(BC, val);
        break;
    case DE_STK:
        r16_write(DE, val);
        break;
    case HL_STK:
        r16_write(HL, val);
        break;
    case AF_STK:
        cpu.a = HIBYTE(val);
        cpu.f = LOBYTE(val);
        break;
    default:
        UNREACHABLE_SWITCH(dst);
    }
}

void execute(void) {
    cpu.opcode = next_8();
    uint8_t opcode = cpu.opcode;
    cpu.remaining_cycles -= cycle_lookup[opcode] * 4;
    if (cpu.opcode == 0xcb) {
        cpu.opcode = next_8();
        opcode = cpu.opcode;
        cpu.remaining_cycles -= extended_lookup[opcode % 8] * 4;
        if ((opcode >> 6) == 0) {
            switch ((opcode >> 3) & 0b111) {
            case 0:
                TODO("rlc r8");
                break;
            case 1:
                TODO("rrc r8");
                break;
            case 2: {
                uint8_t result = r8_read(opcode & 0b111) * 2 +
                                 ((get_status_bit(C_STATUS) != 0) ? 1 : 0);

                set_status_bit(Z_STATUS, result == 0);
                set_status_bit(N_STATUS, false);
                set_status_bit(H_STATUS, false);
                set_status_bit(C_STATUS, r8_read(opcode & 0b111) & 0x80);
                r8_write(opcode & 0b111, result);
                break;
            }
            case 3:
                TODO("rr r8");
                break;
            case 4:
                TODO("sla r8");
                break;
            case 5:
                TODO("sra r8");
                break;
            case 6: {
                uint8_t operand = r8_read(opcode & 0b111);
                uint8_t result =
                    ((operand & 0xf0) >> 4) | ((operand & 0x0f) << 4);
                set_status_bit(Z_STATUS, result == 0);
                set_status_bit(N_STATUS, false);
                set_status_bit(H_STATUS, false);
                set_status_bit(C_STATUS, false);
                r8_write(opcode & 0b111, result);
                break;
            }
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
            if (opcode == 0b00000000) {
                // this page intentionally left blank
            } else if (opcode == 0b00000111)
                TODO("rlca");
            else if (opcode == 0b00001000)
                TODO("ld [imm16], sp");
            else if (opcode == 0b00001111)
                TODO("rrca");
            else if (opcode == 0b00010000)
                TODO("stop");
            else if (opcode == 0b00010111) {
                uint8_t result =
                    cpu.a * 2 + ((get_status_bit(C_STATUS) != 0) ? 1 : 0);
                set_status_bit(Z_STATUS, false);
                set_status_bit(N_STATUS, false);
                set_status_bit(H_STATUS, false);
                set_status_bit(C_STATUS, cpu.a & 0x80);
                cpu.a = result;
            } else if (opcode == 0b00011000) {
                cpu.pc += (int8_t)next_8();
            } else if (opcode == 0b00011111)
                TODO("rra");
            else if (opcode == 0b00100111)
                TODO("daa");
            else if (opcode == 0b00101111) {
                cpu.a = ~cpu.a;
                set_status_bit(N_STATUS, true);
                set_status_bit(H_STATUS, true);
            } else if (opcode == 0b00110111)
                TODO("scf");
            else if (opcode == 0b00111111)
                TODO("ccf");
            else if ((opcode & 0b11100111) == 0b100000) {
                if (resolve_cond((opcode >> 3) & 0b11))
                    cpu.pc += (int8_t)next_8();
                else
                    next_8();
            } else if ((opcode & 0b100) == 0) {
                switch (opcode & 0b1111) {
                case 1:
                    r16_write((opcode >> 4) & 0b11, next_16());
                    break;
                case 2:
                    r16mem_write((opcode >> 4) & 0b11, cpu.a);
                    break;
                case 3:
                    r16_write((opcode >> 4) & 0b11,
                              r16_read((opcode >> 4) & 0b11) + 1);
                    break;
                case 9: {
                    uint16_t operand = r16_read((opcode >> 4) & 0b11);
                    uint16_t result = r16_read(HL) + operand;
                    set_status_bit(N_STATUS, false);
                    set_status_bit(H_STATUS,
                                   (r16_read(HL) & 0xfff) + (operand & 0xfff) >
                                       0xfff);
                    set_status_bit(C_STATUS, r16_read(HL) + operand > 0xffff);
                    r16_write(HL, result);
                    break;
                }
                case 10:
                    cpu.a = r16mem_read((opcode >> 4) & 0b11);
                    break;
                case 11:
                    r16_write((opcode >> 4) & 0b11,
                              r16_read((opcode >> 4) & 0b11) - 1);
                    break;
                }
            } else {
                switch (opcode & 0b11) {
                case 0: {
                    uint8_t result = r8_read((opcode >> 3) & 0b111) + 1;
                    set_status_bit(Z_STATUS, result == 0);
                    set_status_bit(N_STATUS, false);
                    set_status_bit(H_STATUS, (result & 0xf) == 0);
                    r8_write((opcode >> 3) & 0b111, result);
                    break;
                }
                case 1: {
                    uint8_t result = r8_read((opcode >> 3) & 0b111) - 1;
                    set_status_bit(Z_STATUS, result == 0);
                    set_status_bit(N_STATUS, true);
                    set_status_bit(H_STATUS, (result & 0xf) == 0xf);
                    r8_write((opcode >> 3) & 0b111, result);
                    break;
                }
                case 2:
                    r8_write((opcode >> 3) & 0b111, next_8());
                    break;
                default:
                    UNREACHABLE_SWITCH(opcode & 0b11);
                }
            }
            break;
        case 1:
            if (opcode == 0b01110110)
                TODO("halt");
            else {
                r8_write((opcode >> 3) & 0b111, r8_read(opcode & 0b111));
            }
            break;
        case 2:
            switch ((opcode >> 3) & 0b111) {
            case 0: {
                uint8_t operand = r8_read(opcode & 0b111);
                uint8_t result = cpu.a + operand;
                set_status_bit(Z_STATUS, result == 0);
                set_status_bit(N_STATUS, true);
                set_status_bit(H_STATUS, (operand & 0xf) + (cpu.a & 0xf) > 0xf);
                set_status_bit(C_STATUS, operand + cpu.a > 0xff);
                cpu.a = result;
                break;
            }
            case 1:
                TODO("adc a, r8");
                break;
            case 2: {
                uint8_t operand = r8_read(opcode & 0b111);
                uint8_t result = cpu.a - operand;
                set_status_bit(Z_STATUS, result == 0);
                set_status_bit(N_STATUS, true);
                set_status_bit(H_STATUS, (operand & 0xf) > (cpu.a & 0xf));
                set_status_bit(C_STATUS, operand > cpu.a);
                cpu.a = result;
                break;
            }
            case 3:
                TODO("sbc a, r8");
                break;
            case 4: {
                cpu.a &= r8_read(opcode & 0b111);
                set_status_bit(Z_STATUS, cpu.a == 0);
                set_status_bit(N_STATUS, false);
                set_status_bit(H_STATUS, true);
                set_status_bit(C_STATUS, false);
                break;
            }
            case 5: {
                cpu.a ^= r8_read(opcode & 0b111);
                set_status_bit(Z_STATUS, cpu.a == 0);
                set_status_bit(N_STATUS, false);
                set_status_bit(H_STATUS, false);
                set_status_bit(C_STATUS, false);
                break;
            }
            case 6:
                cpu.a |= r8_read(opcode & 0b111);
                set_status_bit(Z_STATUS, cpu.a == 0);
                set_status_bit(N_STATUS, false);
                set_status_bit(H_STATUS, false);
                set_status_bit(C_STATUS, false);
                break;
            case 7: {
                uint8_t operand = r8_read(opcode & 0b111);
                uint8_t result = cpu.a - operand;
                set_status_bit(Z_STATUS, result == 0);
                set_status_bit(N_STATUS, true);
                set_status_bit(H_STATUS, (operand & 0xf) > (cpu.a & 0xf));
                set_status_bit(C_STATUS, operand > cpu.a);
                break;
            }
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
            else if (opcode == 0b11100110) {
                cpu.a &= next_8();
                set_status_bit(Z_STATUS, cpu.a == 0);
                set_status_bit(N_STATUS, false);
                set_status_bit(H_STATUS, true);
                set_status_bit(C_STATUS, false);
            } else if (opcode == 0b11101110)
                TODO("xor a, imm8");
            else if (opcode == 0b11110110)
                TODO("or a, imm8");
            else if (opcode == 0b11111110) {
                uint8_t operand = next_8();
                uint8_t result = cpu.a - operand;
                set_status_bit(Z_STATUS, result == 0);
                set_status_bit(N_STATUS, true);
                set_status_bit(H_STATUS, (operand & 0xf) > (cpu.a & 0xf));
                set_status_bit(C_STATUS, cpu.a > result);
            } else if (opcode == 0b11001001)
                ret();
            else if (opcode == 0b11011001)
                TODO("reti");
            else if (opcode == 0b11000011)
                cpu.pc = next_16();
            else if (opcode == 0b11101001)
                cpu.pc = r16_read(HL);
            else if (opcode == 0b11001101)
                call(next_16());
            else if (opcode == 0b11100010)
                write_8(0xff00 + cpu.c, cpu.a);
            else if (opcode == 0b11100000)
                write_8(0xff00 + next_8(), cpu.a);
            else if (opcode == 0b11101010)
                write_8(next_16(), cpu.a);
            else if (opcode == 0b11110010)
                TODO("ldh a, [c]");
            else if (opcode == 0b11110000)
                cpu.a = read_8(0xff00 + next_8());
            else if (opcode == 0b11111010)
                cpu.a = read_8(read_16(next_16()));
            else if (opcode == 0b11101000)
                TODO("add sp, imm8");
            else if (opcode == 0b11111000)
                TODO("ld hl, sp + imm8");
            else if (opcode == 0b11111001)
                TODO("ld sp, hl");
            else if (opcode == 0b11110011)
                cpu.ime = false;
            else if (opcode == 0b11111011)
                cpu.ime = true;
            else
                switch (opcode & 0b111) {
                case 0:
                    TODO("ret cond");
                    break;
                case 1:
                    r16stk_write((opcode >> 4) & 0b11, pop_16());
                    break;
                case 2: {
                    if(resolve_cond((opcode >> 3) & 0b11)) {
                        cpu.pc = next_16();
                    } else {
                        next_16();
                    }
                    break;
                }
                case 4:
                    TODO("call cond, imm16");
                    break;
                case 5:
                    push_16(r16stk_read((opcode >> 4) & 0b11));
                    break;
                case 7:
                    call(8 * ((opcode >> 3) & 0b111));
                    break;
                default:
                    UNREACHABLE_SWITCH(opcode);
                }
            break;
        default:
            UNREACHABLE_SWITCH(opcode >> 6);
        }
}

void check_interrupts(void) {
    if (cpu.ime) {
        uint8_t vector = 0;
        if (cpu.memory.vblank_ie && cpu.memory.vblank_if) {
            cpu.memory.vblank_if = false;
            vector = 0x40;
        } else if (cpu.memory.lcd_ie && cpu.memory.lcd_if) {
            cpu.memory.lcd_if = false;
            vector = 0x48;
        } else if (cpu.memory.timer_ie && cpu.memory.timer_if) {
            cpu.memory.timer_if = false;
            vector = 0x50;
        } else if (cpu.memory.serial_ie && cpu.memory.serial_if) {
            cpu.memory.serial_if = false;
            vector = 0x58;
        } else if (cpu.memory.joypad_ie && cpu.memory.joypad_if) {
            cpu.memory.joypad_if = false;
            vector = 0x60;
        }

        if (vector != 0) {
            cpu.ime = false;
            push_16(cpu.pc);
            cpu.pc = vector;
            cpu.remaining_cycles -= 20;
        }
    }
}
