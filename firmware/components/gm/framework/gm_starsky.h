#ifndef _GM_STARSKY_H
#define _GM_STARSKY_H

#include "gm_node.h"
#include "gm_physics.h"

typedef struct
{
	int32_t x, y;
} gm_star_t;

typedef struct
{
	uint8_t n, color;
	uint8_t width, height;
	int16_t xspeed, yspeed;
	gm_star_t *stars;
} gm_starsky_t;

gm_node_t* gm_starsky_create(uint8_t n, int16_t z, uint8_t color);
void gm_starsky_set_xspeed(gm_starsky_t *starsky, int16_t speed);
void gm_starsky_set_yspeed(gm_starsky_t *starsky, int16_t speed);
void gm_starsky_visit(gm_starsky_t *starsky, int16_t xoffset, int16_t yoffset);

#endif