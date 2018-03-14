#ifndef _GM_INPUT_H
#define _GM_INPUT_H

#include "gm_board.h"

enum
{
	GM_INPUT_UP = 0,
	GM_INPUT_DOWN,
	GM_INPUT_LEFT,
	GM_INPUT_RIGHT,
	GM_INPUT_A,
	GM_INPUT_B,
	GM_INPUT_C,
	GM_INPUT_D
};

typedef struct
{
	uint8_t reinit;
	int x, y, a, b, c, d;
} gm_input_cfg_t;

uint8_t gm_input_setup(gm_input_cfg_t* cfg);
int32_t gm_input_read(void);

#endif