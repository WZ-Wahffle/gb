#include "apu.h"
#include "raylib.h"
#include "types.h"
#include <math.h>

extern cpu_t cpu;
extern apu_t apu;

static int8_t duty_cycles[4][8] = {{-1, -1, -1, -1, -1, -1, -1, 1},
                                   {-1, -1, -1, -1, -1, -1, 1, 1},
                                   {-1, -1, -1, -1, 1, 1, 1, 1},
                                   {-1, -1, 1, 1, 1, 1, 1, 1}};

static float square_wave(float x, duty_cycle_t dc) {
    ASSERT(x <= 2 * PI, "Square wave call outside of allowed range, found %f",
           x);
    return duty_cycles[dc][((uint8_t)floor(x / (PI / 4)) % 8)];
}

static void ch1_cb(void *buffer, uint32_t sample_count) {
    static float square_idx = 0.f;
    static float envelope_timer = 0.f;
    static float length_timer = 0.f;
    static float sweep_timer = 0.f;

    int16_t *out = (int16_t *)buffer;

    ASSERT(apu.ch1.wave_duty < 4,
           "Pulse channel 1 duty cycle out of bounds, found %d",
           apu.ch1.wave_duty);
    for (uint32_t i = 0; i < sample_count; i++) {
        out[i] = !apu.muted *
                 (int16_t)(32000.f * 0.02 *
                           square_wave(2 * PI * square_idx, apu.ch1.wave_duty) *
                           (apu.audio_enable ? 1 : 0) *
                           (apu.ch1.enable ? 1 : 0) * (apu.ch1.volume / 15.f));
        if (apu.ch1.enable) {
            square_idx += apu.ch1.frequency / (float)SAMPLE_RATE;
            while (square_idx > 1)
                square_idx -= 1;
        }

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

        if (apu.ch1.length_enable) {
            length_timer += 1.f / SAMPLE_RATE;
            if (length_timer > 1.f / 256.f) {
                apu.ch1.current_length_timer++;
                if (apu.ch1.current_length_timer >= 64) {
                    apu.ch1.enable = false;
                    apu.ch1.length_enable = false;
                }
                length_timer -= 1.f / 256.f;
            }
        }

        if (apu.ch1.sweep_step != 0) {
            sweep_timer += 1.f / SAMPLE_RATE;
            if (sweep_timer > apu.ch1.sweep_pace / 128.f) {
                uint16_t new_period =
                    TO_U16(apu.ch1.period_low, apu.ch1.period_high);
                if (apu.ch1.sweep_direction) {
                    new_period -= new_period / (1 << apu.ch1.sweep_step);
                } else {
                    new_period += new_period / (1 << apu.ch1.sweep_step);
                }
                if (!apu.ch1.sweep_direction && new_period > 0x7ff)
                    apu.ch1.enable = false;
                else {
                    apu.ch1.period_low = LOBYTE(new_period);
                    apu.ch1.period_high = HIBYTE(new_period);
                    apu.ch1.frequency = 131072 / (2048 - new_period);
                }

                sweep_timer -= apu.ch1.sweep_pace / 128.f;
            }
        }
    }
}

static void ch2_cb(void *buffer, uint32_t sample_count) {
    static float square_idx = 0.f;
    static float envelope_timer = 0.f;
    static float length_timer = 0.f;
    int16_t *out = (int16_t *)buffer;

    ASSERT(apu.ch2.wave_duty < 4,
           "Pulse channel 2 duty cycle out of bounds, found %d",
           apu.ch2.wave_duty);
    for (uint32_t i = 0; i < sample_count; i++) {
        out[i] = !apu.muted *
                 (int16_t)(32000.f * 0.02 *
                           square_wave(2 * PI * square_idx, apu.ch2.wave_duty) *
                           (apu.audio_enable ? 1 : 0) *
                           (apu.ch2.enable ? 1 : 0) * (apu.ch2.volume / 15.f));
        if (apu.ch2.enable) {
            square_idx += apu.ch2.frequency / (float)SAMPLE_RATE;
            while (square_idx > 1)
                square_idx -= 1;
        }

        if (apu.ch2.envelope_pace != 0) {
            envelope_timer += 1.f / SAMPLE_RATE;
            while (envelope_timer > (1.f / 64.f) * apu.ch2.envelope_pace) {
                if (apu.ch2.envelope_dir) {
                    if (apu.ch2.volume < 15)
                        apu.ch2.volume++;
                } else {
                    if (apu.ch2.volume > 0)
                        apu.ch2.volume--;
                }
                envelope_timer -= (1.f / 64.f) * apu.ch2.envelope_pace;
            }
        }

        if (apu.ch2.length_enable) {
            length_timer += 1.f / SAMPLE_RATE;
            if (length_timer > 1.f / 256.f) {
                apu.ch2.current_length_timer++;
                if (apu.ch2.current_length_timer >= 64) {
                    apu.ch2.enable = false;
                    apu.ch2.length_enable = false;
                }
                length_timer -= 1.f / 256.f;
            }
        }
    }
}

static void ch3_cb(void *buffer, uint32_t sample_count) {
    static float table_idx = 1.f;
    static float length_timer = 0.f;
    int16_t *out = (int16_t *)buffer;

    for (uint32_t i = 0; i < sample_count; i++) {
        out[i] =
            !apu.muted *
            (int16_t)(32000.f * 0.02 *
                      ((((uint8_t)table_idx % 2)
                            ? (apu.ch3.wave_ram[(uint8_t)table_idx / 2] & 0xf)
                            : (apu.ch3.wave_ram[(uint8_t)table_idx / 2] >> 4)) /
                           8.f -
                       1.f) *
                      (apu.audio_enable ? 1 : 0) * (apu.ch3.enable ? 1 : 0) *
                      (apu.ch3.dac_on ? 1 : 0) * (apu.ch3.volume / 4.f));
        if (apu.ch3.enable) {
            table_idx += (32 * apu.ch3.frequency / (float)SAMPLE_RATE);
            if (table_idx >= 32)
                table_idx = 0;
        }

        if (apu.ch3.length_enable) {
            length_timer += 1.f / SAMPLE_RATE;
            if (length_timer > 1.f / 256.f) {
                apu.ch3.current_length_timer++;
                if (apu.ch3.current_length_timer >= 256) {
                    apu.ch3.enable = false;
                    apu.ch3.length_enable = false;
                }
                length_timer -= 1.f / 256.f;
            }
        }
    }
}

static void ch4_cb(void *buffer, uint32_t sample_count) {
    static float lfsr_timer = 0.f;
    static float envelope_timer = 0.f;
    static float length_timer = 0.f;
    int16_t *out = (int16_t *)buffer;

    for (uint32_t i = 0; i < sample_count; i++) {
        out[i] =
            !apu.muted *
            (int16_t)(32000.f * 0.02 * (apu.ch4.lfsr & 1) * (apu.audio_enable) *
                      (apu.ch4.enable ? 1 : 0) * (apu.ch4.volume / 15.f));

        if (apu.ch4.envelope_pace != 0) {
            envelope_timer += 1.f / SAMPLE_RATE;
            while (envelope_timer > (1.f / 64.f) * apu.ch4.envelope_pace) {
                if (apu.ch4.envelope_dir) {
                    if (apu.ch4.volume < 15)
                        apu.ch4.volume++;
                } else {
                    if (apu.ch4.volume > 0)
                        apu.ch4.volume--;
                }
                envelope_timer -= (1.f / 64.f) * apu.ch4.envelope_pace;
            }
        }

        if (apu.ch4.enable) {
            lfsr_timer += 1.f / SAMPLE_RATE;
            while (lfsr_timer >
                   1.f / (262144.f / (apu.ch4.clock_divider *
                                      pow(2, apu.ch4.clock_shift)))) {
                bool result =
                    (apu.ch4.lfsr & 0b1) == ((apu.ch4.lfsr >> 1) & 0b1);
                apu.ch4.lfsr &= 0x7fff;
                apu.ch4.lfsr |= result << 15;
                if (apu.ch4.narrow_lfsr) {
                    apu.ch4.lfsr &= 0xff7f;
                    apu.ch4.lfsr |= result << 7;
                }
                apu.ch4.lfsr >>= 1;
                lfsr_timer -= 1.f / (262144.f / (apu.ch4.clock_divider *
                                                 pow(2, apu.ch4.clock_shift)));
            }
        }

        if (apu.ch4.length_enable) {
            length_timer += 1.f / SAMPLE_RATE;
            if (length_timer > 1.f / 256.f) {
                apu.ch4.current_length_timer++;
                if (apu.ch4.current_length_timer >= 64) {
                    apu.ch4.enable = false;
                    apu.ch4.length_enable = false;
                }
                length_timer -= 1.f / 256.f;
            }
        }
    }
}

int32_t apu_init(void *_) {
    (void)_;
    SetTraceLogLevel(LOG_ERROR);
    InitAudioDevice();
    SetAudioStreamBufferSizeDefault(1024);

    apu.ch1.handle = LoadAudioStream(SAMPLE_RATE, 16, 1);
    SetAudioStreamCallback(apu.ch1.handle, ch1_cb);
    PlayAudioStream(apu.ch1.handle);

    apu.ch2.handle = LoadAudioStream(SAMPLE_RATE, 16, 1);
    SetAudioStreamCallback(apu.ch2.handle, ch2_cb);
    PlayAudioStream(apu.ch2.handle);

    apu.ch3.handle = LoadAudioStream(SAMPLE_RATE, 16, 1);
    SetAudioStreamCallback(apu.ch3.handle, ch3_cb);
    PlayAudioStream(apu.ch3.handle);

    apu.ch4.clock_divider = 0.5;
    apu.ch4.handle = LoadAudioStream(SAMPLE_RATE, 16, 1);
    SetAudioStreamCallback(apu.ch4.handle, ch4_cb);
    PlayAudioStream(apu.ch4.handle);

    return 0;
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

void ch1_period_low(uint8_t val) {
    apu.ch1.period_low = val;
    apu.ch1.frequency =
        131072 / (2048 - TO_U16(apu.ch1.period_low, apu.ch1.period_high));
}

void ch1_period_high_control(uint8_t val) {
    apu.ch1.period_high = val & 0b111;
    apu.ch1.length_enable = val & 0x40;
    apu.ch1.frequency =
        131072 / (2048 - TO_U16(apu.ch1.period_low, apu.ch1.period_high));
    if (val & 0x80) {
        apu.ch1.enable = true;
        if (apu.ch1.current_length_timer >= 64)
            apu.ch1.current_length_timer = apu.ch1.initial_length_timer;
        // TODO: reset envelope timer
        apu.ch1.volume = apu.ch1.envelope_initial_volume;
    }
}

void ch2_length_timer_duty_cycle(uint8_t val) {
    apu.ch2.initial_length_timer = val & 0x3f;
    apu.ch2.wave_duty = val >> 6;
}

void ch2_volume_envelope(uint8_t val) {
    apu.ch2.envelope_pace = val & 0b111;
    apu.ch2.envelope_dir = val & 0x8;
    apu.ch2.envelope_initial_volume = val >> 4;
}

void ch2_period_low(uint8_t val) {
    apu.ch2.period_low = val;
    apu.ch2.frequency =
        131072 / (2048 - TO_U16(apu.ch2.period_low, apu.ch2.period_high));
}

void ch2_period_high_control(uint8_t val) {
    apu.ch2.period_high = val & 0b111;
    apu.ch2.length_enable = val & 0x40;
    apu.ch2.frequency =
        131072 / (2048 - TO_U16(apu.ch2.period_low, apu.ch2.period_high));
    if (val & 0x80) {
        apu.ch2.enable = true;
        if (apu.ch2.current_length_timer >= 64)
            apu.ch2.current_length_timer = apu.ch2.initial_length_timer;
        // TODO: reset envelope timer
        apu.ch2.volume = apu.ch2.envelope_initial_volume;
    }
}

void ch3_period_low(uint8_t val) {
    apu.ch3.period_low = val;
    apu.ch3.frequency =
        65536 / (2048 - TO_U16(apu.ch3.period_low, apu.ch3.period_high));
}

void ch3_period_high_control(uint8_t val) {
    apu.ch3.period_high = val & 0b111;
    apu.ch3.length_enable = val & 0x40;
    apu.ch3.frequency =
        65536 / (2048 - TO_U16(apu.ch3.period_low, apu.ch3.period_high));
    if (val & 0x80) {
        apu.ch3.enable = true;
        if (apu.ch3.current_length_timer >= 256)
            apu.ch3.current_length_timer = apu.ch3.initial_length_timer;
    }
}

void ch4_length_timer(uint8_t val) {
    apu.ch4.initial_length_timer = val & 0x3f;
}

void ch4_volume_envelope(uint8_t val) {
    apu.ch4.envelope_pace = val & 0b111;
    apu.ch4.envelope_dir = val & 0x8;
    apu.ch4.envelope_initial_volume = val >> 4;
}

void ch4_frequency_randomness(uint8_t val) {
    apu.ch4.clock_divider = val & 0b111;
    if (apu.ch4.clock_divider == 0)
        apu.ch4.clock_divider = 0.5f;
    apu.ch4.narrow_lfsr = val & 0x8;
    apu.ch4.clock_shift = val >> 4;
}

void ch4_control(uint8_t val) {
    apu.ch4.length_enable = val & 0x40;
    if (val & 0x80) {
        apu.ch4.enable = true;
        if (apu.ch4.current_length_timer >= 64)
            apu.ch4.current_length_timer = apu.ch4.initial_length_timer;
        // TODO: reset envelope timer
        apu.ch4.volume = apu.ch4.envelope_initial_volume;
        apu.ch4.lfsr = 0;
    }
}
