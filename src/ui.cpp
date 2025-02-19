#define NO_FONT_AWESOME

#include "imgui.h"
#include "rlImGui.h"
#include "types.h"

extern cpu_t cpu;
extern ppu_t ppu;
extern apu_t apu;

extern "C" {
void cpp_init(void) { rlImGuiSetup(true); }

void cpp_imgui_render(void) {
    rlImGuiBegin();
    ImGui::Begin("debug", NULL, ImGuiWindowFlags_NoCollapse);
    ImGui::Text("PC: 0x%04x", cpu.pc);
    ImGui::SameLine();
    if(ImGui::Button("Start")) cpu.state = RUNNING;
    ImGui::SameLine();
    if(ImGui::Button("Stop")) cpu.state = STOPPED;
    ImGui::SameLine();
    if(ImGui::Button("Step")) cpu.state = STEPPED;
    ImGui::End();
    rlImGuiEnd();
}

void cpp_end(void) { rlImGuiShutdown(); }
}
