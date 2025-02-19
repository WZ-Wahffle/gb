#ifndef APU_H_
#define APU_H_

#include "types.h"

void audio_master_control(uint8_t val);
void sound_panning(uint8_t val);
void master_volume_vin_panning(uint8_t val);
void ch1_sweep(uint8_t val);
void ch1_length_timer_duty_cycle(uint8_t val);
void ch1_volume_envelope(uint8_t val);
void ch1_period_low(uint8_t val);
void ch1_period_high_control(uint8_t val);

#endif
