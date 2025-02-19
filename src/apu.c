#include "apu.h"
#include "raylib.h"
#include "types.h"
#include <math.h>

extern apu_t apu;

static int8_t duty_cycles[4][8] = {{-1, -1, -1, -1, -1, -1, -1, 1},
                                    {-1, -1, -1, -1, -1, -1, 1, 1},
                                    {-1, -1, -1, -1, 1, 1, 1, 1},
                                    {-1, -1, 1, 1, 1, 1, 1, 1}};

static float square_wave(float x, duty_cycle_t dc) {
    ASSERT(x < 2 * PI, "Square wave call outside of allowed range, found %f",
           x);
    return duty_cycles[dc][((uint8_t)floor(x / (PI / 4)) % 8)];
}

static void ch1_cb(void *buffer, uint32_t sample_count) {
    static float square_idx = 0.f;
    static float envelope_timer = 0.f;
    int16_t *out = (int16_t *)buffer;

    ASSERT(apu.ch1.wave_duty < 4,
           "Pulse channel 1 duty cycle out of bounds, found %d",
           apu.ch1.wave_duty);
    for (uint32_t i = 0; i < sample_count; i++) {
        out[i] = (int16_t)(32000.f * 0.02 *
                           square_wave(2 * PI * square_idx, apu.ch1.wave_duty) *
                           (apu.ch1.enable ? 1 : 0) * (apu.ch1.volume / 15.f));
        square_idx += apu.ch1.frequency / (float)SAMPLE_RATE;
        while (square_idx > 1)
            square_idx -= 1;

        if (apu.ch1.envelope_pace != 0) {
            envelope_timer += 1.f / SAMPLE_RATE;
            while (envelope_timer > (1.f / 64.f) * apu.ch1.envelope_pace) {
                if (apu.ch1.envelope_dir) {
                    if (apu.ch1.volume < 15)
                        apu.ch1.volume++;
                } else {
                    if (apu.ch1.volume > 0)
                        apu.ch1.volume--;
                }
                envelope_timer -= (1.f / 64.f) * apu.ch1.envelope_pace;
            }
        }
    }
}

void apu_init(void) {
    SetTraceLogLevel(LOG_ERROR);
    InitAudioDevice();
    SetAudioStreamBufferSizeDefault(1024);
    apu.ch1.handle = LoadAudioStream(SAMPLE_RATE, 16, 1);
    SetAudioStreamCallback(apu.ch1.handle, ch1_cb);
    PlayAudioStream(apu.ch1.handle);
    apu.ch2.handle = LoadAudioStream(SAMPLE_RATE, 16, 1);
    apu.ch3.handle = LoadAudioStream(SAMPLE_RATE, 16, 1);
    apu.ch4.handle = LoadAudioStream(SAMPLE_RATE, 16, 1);
}

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

void ch1_period_low(uint8_t val) { apu.ch1.period_low = val; }

void ch1_period_high_control(uint8_t val) {
    apu.ch1.period_high = val & 0b111;
    apu.ch1.length_enable = val & 0x40;
    if (val & 0x80) {
        apu.ch1.enable = true;
        // TODO: reset length timer if expired
        apu.ch1.frequency =
            131072 / (2048 - TO_U16(apu.ch1.period_low, apu.ch1.period_high));
        // TODO: reset envelope timer
        apu.ch1.volume = apu.ch1.envelope_initial_volume;
    }
}
