#ifndef APU_H_
#define APU_H_

#include "types.h"

#ifndef __cplusplus
#define EXTERNC
#else
#define EXTERNC extern "C"
#endif

EXTERNC void apu_init(void);
void audio_master_control(uint8_t val);
void sound_panning(uint8_t val);
void master_volume_vin_panning(uint8_t val);

void ch1_sweep(uint8_t val);
void ch1_length_timer_duty_cycle(uint8_t val);
void ch1_volume_envelope(uint8_t val);
void ch1_period_low(uint8_t val);
void ch1_period_high_control(uint8_t val);

void ch2_length_timer_duty_cycle(uint8_t val);
void ch2_volume_envelope(uint8_t val);
void ch2_period_low(uint8_t val);
void ch2_period_high_control(uint8_t val);

void ch3_period_low(uint8_t val);
void ch3_period_high_control(uint8_t val);

void ch4_length_timer(uint8_t val);
void ch4_volume_envelope(uint8_t val);
void ch4_frequency_randomness(uint8_t val);
void ch4_control(uint8_t val);

#endif
