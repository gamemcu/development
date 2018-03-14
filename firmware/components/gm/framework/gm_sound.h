#ifndef _GM_SOUND_H
#define _GM_SOUND_H

#include "gm_base.h"

typedef uint16_t gm_song_t;

void gm_sound_setup(int pin);
uint8_t gm_sound_tone(uint32_t freq, uint32_t dura);
#endif