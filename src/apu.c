#include "apu.h"
#include "types.h"

extern apu_t apu;

void audio_master_control(uint8_t val) {
    apu.ch1.enable = val & 0x1;
    apu.ch2.enable = val & 0x2;
    apu.ch3.enable = val & 0x4;
    apu.ch4.enable = val & 0x8;
    apu.audio_enable = val & 0x80;
}

void sound_panning(uint8_t val) {
    apu.ch1.pan_right = val & 0x1;
    apu.ch2.pan_right = val & 0x2;
    apu.ch3.pan_right = val & 0x4;
    apu.ch4.pan_right = val & 0x8;
    apu.ch1.pan_left = val & 0x10;
    apu.ch2.pan_left = val & 0x20;
    apu.ch3.pan_left = val & 0x40;
    apu.ch4.pan_left = val & 0x80;
}

void master_volume_vin_panning(uint8_t val) {
    apu.vol_right = val & 0b111;
    apu.vin_right = val & 0x8;
    apu.vol_left = (val >> 4) & 0b111;
    apu.vin_left = val & 0x80;
}

void ch1_sweep(uint8_t val) {
    apu.ch1.sweep_step = val & 0b111;
    apu.ch1.sweep_direction = val & 0x8;
    apu.ch1.sweep_pace = (val >> 4) & 0b111;
}

void ch1_length_timer_duty_cycle(uint8_t val) {
    apu.ch1.initial_length_timer = val & 0x3f;
    apu.ch1.wave_duty = val >> 6;
}

void ch1_volume_envelope(uint8_t val) {
    apu.ch1.envelope_pace = val & 0b111;
    apu.ch1.envelope_dir = val & 0x8;
    apu.ch1.envelope_initial_volume = val >> 4;
}

void ch1_period_low(uint8_t val) {
    apu.ch1.period_low = val;
}

void ch1_period_high_control(uint8_t val) {
    apu.ch1.period_high = val & 0b111;
    apu.ch1.length_enable = val & 0x40;
    apu.ch1.trigger = val & 0x80;
}
