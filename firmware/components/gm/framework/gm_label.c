#include "gm_label.h"
#include "gm_lcd.h"
#include "gm_camera.h"

static void _free(gm_node_t* node)
{
	GM_DBG("label free");
	gm_label_t* label = (gm_label_t*)node->p;
	free(label->str);
	gm_node_free(node);
}

gm_node_t* gm_label_create(int16_t x, int16_t y, int16_t z, const char* str)
{
	gm_label_t* label = (gm_label_t*)malloc(sizeof(gm_label_t));
	if (label == NULL) {
		return NULL;
	}
	memset(label, 0, sizeof(gm_label_t));
	label->is_visiable = 1;
	label->x.maxspeed = label->y.maxspeed = GM_MAX_SPRITE_SPEED;
	label->x.maxpos = label->y.maxpos = 65536 << 7;
	label->x.minpos = label->y.minpos = -65536 << 7;
	label->x.pos = x << 7;
	label->y.pos = y << 7;
	label->str = (char*)malloc(strlen(str) + 1);
	if (label->str == NULL) {
		return NULL;
	}
	strcpy(label->str, str);
	return gm_node_create(label, GM_NODE_LABEL, z, _free);
}

uint8_t gm_label_set_visiable(gm_label_t *label, uint8_t v)
{
	if (label) {
		label->is_visiable = v > 0;
		return 1;
	}
	return 0;
}

uint8_t gm_label_get_pos(gm_label_t *label, int32_t *x, int32_t *y)
{
	if (label) {
		*x = label->x.pos >> 7;
		*y = label->y.pos >> 7;
		return 1;
	}
	return 0;
}

uint8_t gm_label_set_str(gm_label_t *label, const char* str)
{
	if (label && str) {
		free(label->str);
		label->str = (char*)malloc(strlen(str) + 1);
		if (label->str == NULL) {
			return NULL;
		}
		strcpy(label->str, str);
		return 1;
	}
	return 0;
}

void gm_label_set_xspeed(gm_label_t* label, int16_t speed)
{
	if (label) {
		label->x.speed = gm_physics_limit_speed(&label->x, speed);
	}
}

void gm_label_set_yspeed(gm_label_t* label, int16_t speed)
{
	if (label) {
		label->y.speed = gm_physics_limit_speed(&label->y, speed);
	}
}


void gm_label_visit(gm_label_t *label, int16_t xoffset, int16_t yoffset)
{
	if (label) {
		gm_physics_change_pos(&label->x);
		gm_physics_change_pos(&label->y);
		int16_t x = (label->x.pos >> 7) + xoffset;
		int16_t y = (label->y.pos >> 7) + yoffset;
		gm_camera_update(label, x, y);
		if (label->is_visiable) {
			gm_lcd_write_str(x - gm_camera()->x, y - gm_camera()->y, label->str);
		}
	}
}
