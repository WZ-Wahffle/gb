#include <vector>
#define NO_FONT_AWESOME
#include "cpu.h"
#include "imgui.h"
#include "rlImGui.h"
#include "types.h"
#include <string>

std::string opcode_strings[] = {"NOP",
                                "LD BC,0x%04x",
                                "LD (BC),A",
                                "INC BC",
                                "INC B",
                                "DEC B",
                                "LD B,0x%02x",
                                "RLCA",
                                "LD (0x%04x),SP",
                                "ADD HL,BC",
                                "LD A,(BC)",
                                "DEC BC",
                                "INC C",
                                "DEC C",
                                "LD C,0x%02x",
                                "RRCA",
                                "STOP",
                                "LD DE,0x%04x",
                                "LD (DE),A",
                                "INC DE",
                                "INC D",
                                "DEC D",
                                "LD D,0x%02x",
                                "RLA",
                                "JR %d",
                                "ADD HL,DE",
                                "LD A,(DE)",
                                "DEC DE",
                                "INC E",
                                "DEC E",
                                "LD E,0x%02x",
                                "RRA",
                                "JR NZ,%d",
                                "LD HL,0x%04x",
                                "LD (HL+),A",
                                "INC HL",
                                "INC H",
                                "DEC H",
                                "LD H,0x%02x",
                                "DAA",
                                "JR Z,%d",
                                "ADD HL,HL",
                                "LD A,(HL+)",
                                "DEC HL",
                                "INC L",
                                "DEC L",
                                "LD L,0x%02x",
                                "CPL",
                                "JR NC,%d",
                                "LD SP,0x%04x",
                                "LD (HL-),A",
                                "INC SP",
                                "INC (HL)",
                                "DEC (HL)",
                                "LD (HL),0x%02x",
                                "SCF",
                                "JR C,%d",
                                "ADD HL,SP",
                                "LD A,(HL-)",
                                "DEC SP",
                                "INC A",
                                "DEC A",
                                "LD A,0x%02x",
                                "CCF",
                                "LD B,B",
                                "LD B,C",
                                "LD B,D",
                                "LD B,E",
                                "LD B,H",
                                "LD B,L",
                                "LD B,(HL)",
                                "LD B,A",
                                "LD C,B",
                                "LD C,C",
                                "LD C,D",
                                "LD C,E",
                                "LD C,H",
                                "LD C,L",
                                "LD C,(HL)",
                                "LD C,A",
                                "LD D,B",
                                "LD D,C",
                                "LD D,D",
                                "LD D,E",
                                "LD D,H",
                                "LD D,L",
                                "LD D,(HL)",
                                "LD D,A",
                                "LD E,B",
                                "LD E,C",
                                "LD E,D",
                                "LD E,E",
                                "LD E,H",
                                "LD E,L",
                                "LD E,(HL)",
                                "LD E,A",
                                "LD H,B",
                                "LD H,C",
                                "LD H,D",
                                "LD H,E",
                                "LD H,H",
                                "LD H,L",
                                "LD H,(HL)",
                                "LD H,A",
                                "LD L,B",
                                "LD L,C",
                                "LD L,D",
                                "LD L,E",
                                "LD L,H",
                                "LD L,L",
                                "LD L,(HL)",
                                "LD L,A",
                                "LD (HL),B",
                                "LD (HL),C",
                                "LD (HL),D",
                                "LD (HL),E",
                                "LD (HL),H",
                                "LD (HL),L",
                                "HALT",
                                "LD (HL),A",
                                "LD A,B",
                                "LD A,C",
                                "LD A,D",
                                "LD A,E",
                                "LD A,H",
                                "LD A,L",
                                "LD A,(HL)",
                                "LD A,A",
                                "ADD A,B",
                                "ADD A,C",
                                "ADD A,D",
                                "ADD A,E",
                                "ADD A,H",
                                "ADD A,L",
                                "ADD A,(HL)",
                                "ADD A,A",
                                "ADC A,B",
                                "ADC A,C",
                                "ADC A,D",
                                "ADC A,E",
                                "ADC A,H",
                                "ADC A,L",
                                "ADC A,(HL)",
                                "ADC A,A",
                                "SUB B",
                                "SUB C",
                                "SUB D",
                                "SUB E",
                                "SUB H",
                                "SUB L",
                                "SUB (HL)",
                                "SUB A",
                                "SBC A,B",
                                "SBC A,C",
                                "SBC A,D",
                                "SBC A,E",
                                "SBC A,H",
                                "SBC A,L",
                                "SBC A,(HL)",
                                "SBC A,A",
                                "AND B",
                                "AND C",
                                "AND D",
                                "AND E",
                                "AND H",
                                "AND L",
                                "AND (HL)",
                                "AND A",
                                "XOR B",
                                "XOR C",
                                "XOR D",
                                "XOR E",
                                "XOR H",
                                "XOR L",
                                "XOR (HL)",
                                "XOR A",
                                "OR B",
                                "OR C",
                                "OR D",
                                "OR E",
                                "OR H",
                                "OR L",
                                "OR (HL)",
                                "OR A",
                                "CP B",
                                "CP C",
                                "CP D",
                                "CP E",
                                "CP H",
                                "CP L",
                                "CP (HL)",
                                "CP A",
                                "RET NZ",
                                "POP BC",
                                "JP NZ,0x%04x",
                                "JP 0x%04x",
                                "CALL NZ,0x%04x",
                                "PUSH BC",
                                "ADD A,0x%02x",
                                "RST 00H",
                                "RET Z",
                                "RET",
                                "JP Z,0x%04x",
                                "PREFIX CB",
                                "CALL Z,0x%04x",
                                "CALL 0x%04x",
                                "ADC A,0x%02x",
                                "RST 08H",
                                "RET NC",
                                "POP DE",
                                "JP NC,0x%04x",
                                "???",
                                "CALL NC,0x%04x",
                                "PUSH DE",
                                "SUB 0x%02x",
                                "RST 10H",
                                "RET C",
                                "RETI",
                                "JP C,0x%04x",
                                "???",
                                "CALL C,0x%04x",
                                "???",
                                "SBC A,0x%02x",
                                "RST 18H",
                                "LDH (a8),A",
                                "POP HL",
                                "LD (C),A",
                                "???",
                                "???",
                                "PUSH HL",
                                "AND 0x%02x",
                                "RST 20H",
                                "ADD SP,%d",
                                "JP (HL)",
                                "LD (0x%04x),A",
                                "???",
                                "???",
                                "???",
                                "XOR 0x%02x",
                                "RST 28H",
                                "LDH A,(a8)",
                                "POP AF",
                                "LD A,(C)",
                                "DI",
                                "???",
                                "PUSH AF",
                                "OR 0x%02x",
                                "RST 30H",
                                "LD HL,SP+%d",
                                "LD SP,HL",
                                "LD A,(0x%04x)",
                                "EI",
                                "???",
                                "???",
                                "CP 0x%02x",
                                "RST 38H"};

std::string prefixed_opcode_strings[] = {
    "RLC B",      "RLC C",   "RLC D",      "RLC E",   "RLC H",      "RLC L",
    "RLC (HL)",   "RLC A",   "RRC B",      "RRC C",   "RRC D",      "RRC E",
    "RRC H",      "RRC L",   "RRC (HL)",   "RRC A",   "RL B",       "RL C",
    "RL D",       "RL E",    "RL H",       "RL L",    "RL (HL)",    "RL A",
    "RR B",       "RR C",    "RR D",       "RR E",    "RR H",       "RR L",
    "RR (HL)",    "RR A",    "SLA B",      "SLA C",   "SLA D",      "SLA E",
    "SLA H",      "SLA L",   "SLA (HL)",   "SLA A",   "SRA B",      "SRA C",
    "SRA D",      "SRA E",   "SRA H",      "SRA L",   "SRA (HL)",   "SRA A",
    "SWAP B",     "SWAP C",  "SWAP D",     "SWAP E",  "SWAP H",     "SWAP L",
    "SWAP (HL)",  "SWAP A",  "SRL B",      "SRL C",   "SRL D",      "SRL E",
    "SRL H",      "SRL L",   "SRL (HL)",   "SRL A",   "BIT 0,B",    "BIT 0,C",
    "BIT 0,D",    "BIT 0,E", "BIT 0,H",    "BIT 0,L", "BIT 0,(HL)", "BIT 0,A",
    "BIT 1,B",    "BIT 1,C", "BIT 1,D",    "BIT 1,E", "BIT 1,H",    "BIT 1,L",
    "BIT 1,(HL)", "BIT 1,A", "BIT 2,B",    "BIT 2,C", "BIT 2,D",    "BIT 2,E",
    "BIT 2,H",    "BIT 2,L", "BIT 2,(HL)", "BIT 2,A", "BIT 3,B",    "BIT 3,C",
    "BIT 3,D",    "BIT 3,E", "BIT 3,H",    "BIT 3,L", "BIT 3,(HL)", "BIT 3,A",
    "BIT 4,B",    "BIT 4,C", "BIT 4,D",    "BIT 4,E", "BIT 4,H",    "BIT 4,L",
    "BIT 4,(HL)", "BIT 4,A", "BIT 5,B",    "BIT 5,C", "BIT 5,D",    "BIT 5,E",
    "BIT 5,H",    "BIT 5,L", "BIT 5,(HL)", "BIT 5,A", "BIT 6,B",    "BIT 6,C",
    "BIT 6,D",    "BIT 6,E", "BIT 6,H",    "BIT 6,L", "BIT 6,(HL)", "BIT 6,A",
    "BIT 7,B",    "BIT 7,C", "BIT 7,D",    "BIT 7,E", "BIT 7,H",    "BIT 7,L",
    "BIT 7,(HL)", "BIT 7,A", "RES 0,B",    "RES 0,C", "RES 0,D",    "RES 0,E",
    "RES 0,H",    "RES 0,L", "RES 0,(HL)", "RES 0,A", "RES 1,B",    "RES 1,C",
    "RES 1,D",    "RES 1,E", "RES 1,H",    "RES 1,L", "RES 1,(HL)", "RES 1,A",
    "RES 2,B",    "RES 2,C", "RES 2,D",    "RES 2,E", "RES 2,H",    "RES 2,L",
    "RES 2,(HL)", "RES 2,A", "RES 3,B",    "RES 3,C", "RES 3,D",    "RES 3,E",
    "RES 3,H",    "RES 3,L", "RES 3,(HL)", "RES 3,A", "RES 4,B",    "RES 4,C",
    "RES 4,D",    "RES 4,E", "RES 4,H",    "RES 4,L", "RES 4,(HL)", "RES 4,A",
    "RES 5,B",    "RES 5,C", "RES 5,D",    "RES 5,E", "RES 5,H",    "RES 5,L",
    "RES 5,(HL)", "RES 5,A", "RES 6,B",    "RES 6,C", "RES 6,D",    "RES 6,E",
    "RES 6,H",    "RES 6,L", "RES 6,(HL)", "RES 6,A", "RES 7,B",    "RES 7,C",
    "RES 7,D",    "RES 7,E", "RES 7,H",    "RES 7,L", "RES 7,(HL)", "RES 7,A",
    "SET 0,B",    "SET 0,C", "SET 0,D",    "SET 0,E", "SET 0,H",    "SET 0,L",
    "SET 0,(HL)", "SET 0,A", "SET 1,B",    "SET 1,C", "SET 1,D",    "SET 1,E",
    "SET 1,H",    "SET 1,L", "SET 1,(HL)", "SET 1,A", "SET 2,B",    "SET 2,C",
    "SET 2,D",    "SET 2,E", "SET 2,H",    "SET 2,L", "SET 2,(HL)", "SET 2,A",
    "SET 3,B",    "SET 3,C", "SET 3,D",    "SET 3,E", "SET 3,H",    "SET 3,L",
    "SET 3,(HL)", "SET 3,A", "SET 4,B",    "SET 4,C", "SET 4,D",    "SET 4,E",
    "SET 4,H",    "SET 4,L", "SET 4,(HL)", "SET 4,A", "SET 5,B",    "SET 5,C",
    "SET 5,D",    "SET 5,E", "SET 5,H",    "SET 5,L", "SET 5,(HL)", "SET 5,A",
    "SET 6,B",    "SET 6,C", "SET 6,D",    "SET 6,E", "SET 6,H",    "SET 6,L",
    "SET 6,(HL)", "SET 6,A", "SET 7,B",    "SET 7,C", "SET 7,D",    "SET 7,E",
    "SET 7,H",    "SET 7,L", "SET 7,(HL)", "SET 7,A"};

extern cpu_t cpu;
extern ppu_t ppu;
extern apu_t apu;

extern "C" {

static char bp_inter[5] = {0};
static int32_t wram_page = 0;
static int32_t rom_page = 0;
static uint16_t poke_location = 0;
static uint8_t poke_value = 0;

void cpp_init(void) {
    rlImGuiSetup(true);
}

void cpp_imgui_render(void) {
    rlImGuiBegin();
    ImGui::Begin("debug", NULL, ImGuiWindowFlags_NoCollapse);
    ImGui::SetWindowFontScale(2);

    if (read_8(cpu.pc) == 0xcb) {
        uint8_t opcode = read_8(cpu.pc + 1);
        ImGui::Text("%s", prefixed_opcode_strings[opcode].c_str());
    } else {
        uint8_t opcode = read_8(cpu.pc);
        if (opcode_strings[opcode].find("%04x") != std::string::npos) {
            ImGui::Text(opcode_strings[opcode].c_str(), read_16(cpu.pc + 1));

        } else if (opcode_strings[opcode].find("%d") != std::string::npos) {
            ImGui::Text(opcode_strings[opcode].c_str(),
                        (int8_t)read_8(cpu.pc + 1));
        } else if (opcode_strings[opcode].find("%") != std::string::npos) {
            ImGui::Text(opcode_strings[opcode].c_str(), read_8(cpu.pc + 1));
        } else {
            ImGui::Text("%s", opcode_strings[opcode].c_str());
        }
        ImGui::SameLine();
        ImGui::Text(", opcode: 0x%02x", opcode);
    }

    ImGui::Text("PC: 0x%04x", cpu.pc);
    ImGui::SameLine();
    if (ImGui::Button("Start"))
        cpu.state = RUNNING;
    ImGui::SameLine();
    if (ImGui::Button("Stop"))
        cpu.state = STOPPED;
    ImGui::SameLine();
    if (ImGui::Button("Step"))
        cpu.state = STEPPED;

    ImGui::Text("Breakpoint: 0x");
    ImGui::SameLine();
    ImGui::PushItemWidth(4 * ImGui::GetFontSize());
    ImGui::InputText("##bpin", bp_inter, 5);
    ImGui::PopItemWidth();
    bool valid = true;
    for (char &c : bp_inter) {
        if ((c < '0' || c > '9') && (c < 'a' || c > 'f') &&
            (c < 'A' || c > 'F'))
            valid = false;
        break;
    }
    if (!valid) {
        ImGui::SameLine();
        ImGui::TextColored(ImVec4{0xff, 0, 0, 0xff}, "Invalid!");
    } else {
        cpu.breakpoint = strtoul(bp_inter, NULL, 16);
    }

    ImGui::Text("Location: 0x");
    ImGui::SameLine();
    ImGui::PushItemWidth(2.5 * ImGui::GetFontSize());
    ImGui::InputScalar("##pokeloc", ImGuiDataType_U16, &poke_location, NULL, NULL,
                       "%04x");
    ImGui::PopItemWidth();
    ImGui::SameLine();
    ImGui::Text("Value: 0x");
    ImGui::PushItemWidth(1.5 * ImGui::GetFontSize());
    ImGui::SameLine();
    ImGui::InputScalar("##pokeval", ImGuiDataType_U8, &poke_value, NULL, NULL, "%02x");
    ImGui::PopItemWidth();
    ImGui::SameLine();
    if(ImGui::Button("Poke")) {
        write_8(poke_location, poke_value);
    }

    ImGui::Text("Memory watchpoint: 0x");
    ImGui::SameLine();
    ImGui::PushItemWidth(2.5 * ImGui::GetFontSize());
    ImGui::InputScalar("##watchloc", ImGuiDataType_U16, &cpu.watch_addr, NULL, NULL, "%04x");
    ImGui::PopItemWidth();
    ImGui::SameLine();
    if(ImGui::Button(cpu.watching ? "Unwatch" : "Watch")) {
        cpu.watching = !cpu.watching;
    }

    if (ImGui::CollapsingHeader("CPU")) {
        ImGui::Text("A: 0x%02x", cpu.a);
        ImGui::Text("B: 0x%02x", cpu.b);
        ImGui::SameLine();
        ImGui::Text("C: 0x%02x", cpu.c);
        ImGui::Text("D: 0x%02x", cpu.d);
        ImGui::SameLine();
        ImGui::Text("E: 0x%02x", cpu.e);
        ImGui::Text("H: 0x%02x", cpu.h);
        ImGui::SameLine();
        ImGui::Text("L: 0x%02x", cpu.l);
        ImGui::Text("F: ZNHC");
        ImGui::Text("   %d%d%d%d", (cpu.f & 0x80) != 0, (cpu.f & 0x40) != 0,
                    (cpu.f & 0x20) != 0, (cpu.f & 0x10) != 0);
        ImGui::Text("SP: 0x%04x", cpu.sp);
        ImGui::NewLine();
        ImGui::Text("IME %d  EF", cpu.ime);
        ImGui::Text("VBlank %d%d", cpu.memory.vblank_ie, cpu.memory.vblank_if);
        ImGui::Text("LCD    %d%d", cpu.memory.lcd_ie, cpu.memory.lcd_if);
        ImGui::Text("Timer  %d%d", cpu.memory.timer_ie, cpu.memory.timer_if);
        ImGui::Text("Serial %d%d", cpu.memory.serial_ie, cpu.memory.serial_if);
        ImGui::Text("Joypad %d%d", cpu.memory.joypad_ie, cpu.memory.joypad_if);
    }

    if (ImGui::CollapsingHeader("PPU")) {
        ImGui::Text("X position: %d", ppu.drawing_x);
        ImGui::Text("Y position: %d", ppu.drawing_y);
    }

    if (ImGui::CollapsingHeader("APU")) {
        ImGui::BeginTabBar("Channels");
        if (ImGui::BeginTabItem("1")) {
            ImGui::Text("Enabled: %s", apu.ch1.enable ? "true" : "false");
            ImGui::Text("Period low: %d", apu.ch1.period_low);
            ImGui::Text("Period high: %d", apu.ch1.period_high);
            ImGui::Text("Frequency: %d Hz", apu.ch1.frequency);
            ImGui::Text("Volume: %0.4f", apu.ch1.volume / 15.f);
            ImGui::Text("Envelope initial: %d",
                        apu.ch1.envelope_initial_volume);
            ImGui::Text("Envelope pace: %d", apu.ch1.envelope_pace);
            ImGui::Text("Envelope direction: %s",
                        apu.ch1.envelope_dir ? "INC" : "DEC");
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("2")) {
            ImGui::Text("Enabled: %s", apu.ch2.enable ? "true" : "false");
            ImGui::Text("Period low: %d", apu.ch2.period_low);
            ImGui::Text("Period high: %d", apu.ch2.period_high);
            ImGui::Text("Frequency: %d Hz", apu.ch2.frequency);
            ImGui::Text("Volume: %0.4f", apu.ch2.volume / 15.f);
            ImGui::Text("Envelope initial: %d",
                        apu.ch2.envelope_initial_volume);
            ImGui::Text("Envelope pace: %d", apu.ch2.envelope_pace);
            ImGui::Text("Envelope direction: %s",
                        apu.ch2.envelope_dir ? "INC" : "DEC");
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    if (ImGui::CollapsingHeader("Input")) {
        ImGui::Text("UDLR ABSs");
        ImGui::Text("%d%d%d%d %d%d%d%d", ppu.up, ppu.down, ppu.left, ppu.right,
                    ppu.a, ppu.b, ppu.start, ppu.select);
    }

    if (ImGui::CollapsingHeader("ROM")) {
        ImGui::InputInt("Page", &rom_page);
        if (rom_page < 0)
            rom_page = 0;
        if (rom_page > 127)
            rom_page = 127;
        ImGui::BeginTable("##rompages", 8,
                          ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg);
        for (uint8_t i = 0; i < 32; i++) {
            for (uint8_t j = 0; j < 8; j++) {
                ImGui::TableNextColumn();
                ImGui::Text("0x%02x", read_8(rom_page * 256 + i * 8 + j));
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("0x%02x", rom_page * 256 + i * 8 + j);
                }
            }
            if (i != 31)
                ImGui::TableNextRow();
        }
        ImGui::EndTable();
    }

    if (ImGui::CollapsingHeader("WRAM")) {
        ImGui::InputInt("Page", &wram_page);
        if (wram_page < 0)
            wram_page = 0;
        if (wram_page > 31)
            wram_page = 31;
        ImGui::BeginTable("##wrampages", 8,
                          ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg);
        for (uint8_t i = 0; i < 32; i++) {
            for (uint8_t j = 0; j < 8; j++) {
                ImGui::TableNextColumn();
                ImGui::Text("0x%02x",
                            read_8(0xc000 + wram_page * 256 + i * 8 + j));
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("0x%02x",
                                      0xc000 + wram_page * 256 + i * 8 + j);
                }
            }
            if (i != 31)
                ImGui::TableNextRow();
        }
        ImGui::EndTable();
    }

    if (ImGui::CollapsingHeader("HRAM")) {
        ImGui::BeginTable("##pages", 8,
                          ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg);
        for (uint8_t i = 0; i < 16; i++) {
            for (uint8_t j = 0; j < 8; j++) {
                if (i * 8 + j == 127)
                    break;
                ImGui::TableNextColumn();
                ImGui::Text("0x%02x", read_8(0xff80 + i * 8 + j));
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("0x%02x", 0xff80 + i * 8 + j);
                }
            }
            if (i != 31)
                ImGui::TableNextRow();
        }
        ImGui::EndTable();
    }

    if(ImGui::Button("Quit & Dump")) {
        exit(0);
    }

    ImGui::End();
    rlImGuiEnd();
}

void cpp_end(void) { rlImGuiShutdown(); }
}
