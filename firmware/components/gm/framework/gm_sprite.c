
#include "gm_sprite.h"
#include "gm_camera.h"

static void _free(gm_node_t* node)
{
	GM_DBG("sprite free");
	// gm_sprite_t *sprite = (gm_sprite_t*)node->p;
	// free(sprite->bmp.data);
	gm_node_free(node);
}

gm_node_t* gm_sprite_create(int16_t x, int16_t y, int16_t z, bitmap2_t *bmp)
{
	gm_sprite_t* sprite = (gm_sprite_t*)malloc(sizeof(gm_sprite_t));
	if (sprite == NULL) {
		return NULL;
	}
	memset(sprite, 0, sizeof(gm_sprite_t));
	memcpy(&sprite->bmp, bmp, sizeof(bitmap2_t));
	// sprite->bmp.data = (uint8_t*)malloc(sprite->bmp.len);
	// if (sprite->bmp.data == NULL) {
	// 	return NULL;
	// }
	// memcpy(sprite->bmp.data, bmp->data, bmp->len);
	sprite->is_visiable = 1;
	sprite->x.pos = x << 7;
	sprite->y.pos = y << 7;
	sprite->x.maxspeed = sprite->y.maxspeed = GM_MAX_SPRITE_SPEED;
	sprite->x.maxpos = sprite->y.maxpos = 65536 << 7;
	sprite->x.minpos = sprite->y.minpos = -65536 << 7;
	sprite->width = bmp->data[BITMAP_WIDTH];
	sprite->height = bmp->data[BITMAP_HEIGHT];
	return gm_node_create(sprite, GM_NODE_SPRITE, z, _free);
}

uint8_t gm_sprite_set_visiable(gm_sprite_t* sprite, uint8_t v)
{
	if (sprite) {
		sprite->is_visiable = v > 0;
		return 1;
	}
	return 0;
}

uint8_t gm_sprite_set_xmax(gm_sprite_t* sprite, int32_t max)
{
	if (sprite) {
		sprite->x.maxpos = max << 7;
		return 1;
	}
	return 0;
}

uint8_t gm_sprite_set_xmin(gm_sprite_t* sprite, int32_t min)
{
	if (sprite) {
		sprite->x.minpos = min << 7;
		return 1;
	}
	return 0;
}

uint8_t gm_sprite_set_ymax(gm_sprite_t* sprite, int32_t max)
{
	if (sprite) {
		sprite->y.maxpos = max << 7;
		return 1;
	}
	return 0;
}

uint8_t gm_sprite_set_ymin(gm_sprite_t* sprite, int32_t min)
{
	if (sprite) {
		sprite->y.minpos = min << 7;
		return 1;
	}
	return 0;
}

uint8_t gm_sprite_get_pos(gm_sprite_t* sprite, int32_t *x, int32_t *y)
{
	if (sprite) {
		*x = sprite->x.pos >> 7;
		*y = sprite->y.pos >> 7;
		return 1;
	}
	return 0;
}

uint8_t gm_sprite_set_xpos(gm_sprite_t* sprite, int32_t pos)
{
	if (sprite) {
		sprite->x.pos = pos << 7;
		return 1;
	}
	return 0;
}

uint8_t gm_sprite_set_ypos(gm_sprite_t* sprite, int32_t pos)
{
	if (sprite) {
		sprite->y.pos = pos << 7;
		return 1;
	}
	return 0;
}

uint8_t gm_sprite_is_moving(gm_sprite_t* sprite)
{
	if (sprite) {
		return sprite->x.speed != 0 || sprite->y.speed != 0;
	}
	return 0;
}

uint8_t gm_sprite_set_frame(gm_sprite_t* sprite, uint8_t frame)
{
	if (sprite) {
		if (frame < sprite->bmp.data[BITMAP_FRAMES]) {
			sprite->frame = frame;
		}
		return 1;
	}
	return 0;
}

uint8_t gm_sprite_get_frame(gm_sprite_t* sprite)
{
	return sprite->frame;
}

uint8_t gm_sprite_get_speed(gm_sprite_t* sprite, int16_t *xspeed, int16_t *yspeed)
{
	if (sprite) {
		*xspeed = sprite->x.speed;
		*yspeed = sprite->y.speed;
		return 1;
	}
	return 0;
}

uint8_t gm_sprite_set_max_xspeed(gm_sprite_t* sprite, int16_t max)
{
	if (sprite) {
		sprite->x.maxspeed = max;
		return 1;
	}
	return 0;
}

uint8_t gm_sprite_set_max_yspeed(gm_sprite_t* sprite, int16_t max)
{
	if (sprite) {
		sprite->y.maxspeed = max;
		return 1;
	}
	return 0;
}

void gm_sprite_set_xspeed(gm_sprite_t* sprite, int16_t speed)
{
	if (sprite) {
		sprite->x.speed = gm_physics_limit_speed(&sprite->x, speed);
	}
}

void gm_sprite_set_yspeed(gm_sprite_t* sprite, int16_t speed)
{
	if (sprite) {
		sprite->y.speed = gm_physics_limit_speed(&sprite->y, speed);
	}
}

void gm_sprite_set_xacc(gm_sprite_t* sprite, int16_t acc)
{
	if (sprite) {
		sprite->x.acc = acc;
	}
}

void gm_sprite_set_yacc(gm_sprite_t* sprite, int16_t acc)
{
	if (sprite) {
		sprite->y.acc = acc;
	}
}

void gm_sprite_set_xfriction(gm_sprite_t* sprite, int16_t frict)
{
	if (sprite) {
		sprite->x.frict = frict;
	}
}

void gm_sprite_set_yfriction(gm_sprite_t* sprite, int16_t frict)
{
	if (sprite) {
		sprite->y.frict = frict;
	}
}

void gm_sprite_visit(gm_sprite_t* sprite, int16_t xoffset, int16_t yoffset)
{
	if (sprite) {
		gm_physics_change_pos(&sprite->x);
		gm_physics_change_pos(&sprite->y);
		int16_t x = (sprite->x.pos >> 7) + xoffset;
		int16_t y = (sprite->y.pos >> 7) + yoffset;
		gm_camera_update(sprite, x, y);
		if (sprite->is_visiable) {
			gm_lcd_draw_bmp2(&sprite->bmp, x - gm_camera()->x, y - gm_camera()->y, sprite->frame);
		}
	}
}