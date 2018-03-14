#ifndef _GM_CAMERA_H
#define _GM_CAMERA_H

#include "gm_node.h"

typedef struct
{
	int16_t x, y, xoffset, yoffset;
	void *focus_obj;
} gm_camera_t;

gm_camera_t* gm_camera(void);
uint8_t gm_camera_focus(gm_node_t *node, int16_t xoffset, int16_t yoffset);
uint8_t gm_camera_unfocus(gm_node_t *node);
void gm_camera_reset(void);
void gm_camera_update(void *obj, int16_t x, int16_t y);

#endif