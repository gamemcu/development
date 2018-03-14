#ifndef _GM_LABEL_H
#define _GM_LABEL_H

#include "gm_node.h"
#include "gm_physics.h"

typedef struct
{
	gm_physics_dir_t x, y;
	char *str;
	uint8_t is_visiable;
} gm_label_t;

gm_node_t* gm_label_create(int16_t x, int16_t y, int16_t z, const char* str);
uint8_t gm_label_set_visiable(gm_label_t *label, uint8_t v);
uint8_t gm_label_set_str(gm_label_t *label, const char* str);
uint8_t gm_label_get_pos(gm_label_t *label, int32_t *x, int32_t *y);
void gm_label_set_xspeed(gm_label_t *label, int16_t speed);
void gm_label_set_yspeed(gm_label_t *label, int16_t speed);
void gm_label_visit(gm_label_t *label, int16_t xoffset, int16_t yoffset);

#endif