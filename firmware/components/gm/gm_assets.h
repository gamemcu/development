#ifndef _GM_ASSETS_H
#define _GM_ASSETS_H

#include "rom/ets_sys.h"

#define _ASSET_SIZE(w,h) ((((h)+7)>>3)*(w))

enum {
	DRAW_NORMAL = 1 << 0,
	DRAW_MASK = 1 << 1,
	DRAW_REVERSE = 1 << 2,
	DRAW_TRANSPARENCY = 1 << 3,
	DRAW_GRAY = 1 << 4
};

enum {
	BITMAP_WIDTH = 0,
	BITMAP_HEIGHT,
	BITMAP_FRAMES,
	BITMAP_COLOR,
	BITMAP_DATA
};

typedef struct
{
	uint8_t *data;
	uint16_t len;
	uint16_t framesize;
	uint8_t rendermode;
} bitmap2_t;

extern bitmap2_t asset_abc;
extern bitmap2_t asset_num;
extern bitmap2_t asset_ascii;
#endif