#ifndef _GM_TITLEMAP_H
#define _GM_TITLEMAP_H

#include "gm_node.h"
#include "gm_lcd.h"

typedef struct
{
	bitmap2_t title, map;
} gm_titlemap_t;

gm_node_t* gm_titlemap_create(bitmap2_t *title, bitmap2_t *map, int16_t z);
uint8_t gm_titlemap_is_soild(gm_titlemap_t *titlemap, int16_t x, int16_t y);
void gm_titlemap_visit(gm_titlemap_t *titlemap, int16_t xoffset, int16_t yoffset);

#endif