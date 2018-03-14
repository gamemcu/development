#include "gm_camera.h"
#include "gm_label.h"
#include "gm_sprite.h"

gm_camera_t camera;
gm_camera_t* gm_camera(void)
{
	return &camera;
}

uint8_t gm_camera_focus(gm_node_t *node, int16_t xoffset, int16_t yoffset)
{
	if (node) {
		switch (node->type) {
		case GM_NODE_SPRITE:
		case GM_NODE_LABEL: {
			camera.focus_obj = node->p;
			camera.xoffset = xoffset;
			camera.yoffset = yoffset;
			return 1;
		}
		default:
			GM_DBG("camera not support node type");
			return 0;
		}
	}
	return 0;
}


uint8_t gm_camera_unfocus(gm_node_t *node)
{
	if (node) {
		switch (node->type) {
		case GM_NODE_SPRITE:
		case GM_NODE_LABEL: {
			if (camera.focus_obj == node->p) {
				camera.focus_obj = NULL;
			}
			return 1;
		}
		default:
			GM_DBG("camera not support node type");
			return 0;
		}
	}
	return 0;
}

void gm_camera_reset(void)
{
	camera.y = camera.x = 0;
}

void gm_camera_update(void *obj, int16_t x, int16_t y)
{
	if (camera.focus_obj && (camera.focus_obj == obj)) {
		camera.x = x - camera.xoffset;
		camera.y = y - camera.yoffset;
	}
}
