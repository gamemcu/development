#ifndef _GM_PHYSICS_H
#define _GM_PHYSICS_H

#include "gm_node.h"
#include "gm_lcd.h"

typedef struct
{
	int32_t pos, maxpos, minpos;
	int16_t speed, acc, frict;
	uint16_t maxspeed;
} gm_physics_dir_t;

typedef struct
{
	int32_t x, y;
	uint16_t life; 	
}gm_particle_t;

typedef struct
{
	uint8_t n;
	int16_t xspeed, yspeed;
	bitmap2_t bmp;
	gm_node_mng_t particle_mng;
}gm_emiter_t;

int16_t gm_physics_limit_speed(gm_physics_dir_t *dir, int16_t speed);
void gm_physics_change_pos(gm_physics_dir_t *dir);

#endif