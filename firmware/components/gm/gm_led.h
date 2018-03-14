#ifndef _GM_LED_H
#define _GM_LED_H

#include "gm_board.h"

#define LED_DATA_LEN 6
enum
{
	GM_LED_NORMAL = 0,
	GM_LED_BLINK,
	GM_LED_BREATH
};

typedef struct
{
	uint8_t mode, freq;
	int dura;
	uint8_t data[LED_DATA_LEN], len;
}gm_led_cfg_t;

void gm_led_setup(int pin);
uint8_t gm_led_run(gm_led_cfg_t *cfg);

#endif