#ifndef _GM_SPRITE_H
#define _GM_SPRITE_H

#include "gm_node.h"
#include "gm_lcd.h"
#include "gm_physics.h"

typedef struct
{
	gm_physics_dir_t x, y;
	uint8_t is_visiable;
	uint16_t width, height;
	bitmap2_t bmp;
	uint8_t frame;
} gm_sprite_t;

gm_node_t* gm_sprite_create(int16_t x, int16_t y, int16_t z, bitmap2_t *bmp);
uint8_t gm_sprite_set_visiable(gm_sprite_t* sprite, uint8_t v);
uint8_t gm_sprite_set_xmax(gm_sprite_t* sprite, int32_t max);
uint8_t gm_sprite_set_xmin(gm_sprite_t* sprite, int32_t min);
uint8_t gm_sprite_set_ymax(gm_sprite_t* sprite, int32_t max);
uint8_t gm_sprite_set_ymin(gm_sprite_t* sprite, int32_t min);
uint8_t gm_sprite_get_pos(gm_sprite_t* sprite, int32_t *x, int32_t *y);
uint8_t gm_sprite_set_xpos(gm_sprite_t* sprite, int32_t pos);
uint8_t gm_sprite_set_ypos(gm_sprite_t* sprite, int32_t pos);
uint8_t gm_sprite_is_moving(gm_sprite_t* sprite);
uint8_t gm_sprite_set_frame(gm_sprite_t* sprite, uint8_t frame);
uint8_t gm_sprite_get_frame(gm_sprite_t* sprite);
uint8_t gm_sprite_get_speed(gm_sprite_t* sprite, int16_t *xspeed, int16_t *yspeed);
uint8_t gm_sprite_set_max_xspeed(gm_sprite_t* sprite, int16_t max);
uint8_t gm_sprite_set_max_yspeed(gm_sprite_t* sprite, int16_t max);
void gm_sprite_set_xspeed(gm_sprite_t* sprite, int16_t speed);
void gm_sprite_set_yspeed(gm_sprite_t* sprite, int16_t speed);
void gm_sprite_set_xacc(gm_sprite_t* sprite, int16_t acc);
void gm_sprite_set_yacc(gm_sprite_t* sprite, int16_t acc);
void gm_sprite_set_xfriction(gm_sprite_t* sprite, int16_t frict);
void gm_sprite_set_yfriction(gm_sprite_t* sprite, int16_t frict);
void gm_sprite_visit(gm_sprite_t* sprite, int16_t xoffset, int16_t yoffset);
#endif