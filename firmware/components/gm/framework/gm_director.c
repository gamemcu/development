#include <time.h>
#include <sys/time.h>

#include "gm_director.h"
#include "gm_lcd.h"
#include "gm_scene.h"
#include "gm_camera.h"

typedef struct
{
	uint32_t now_s, now, old, delta, wait;
	uint8_t in_wait, pause, show_fps;
	uint16_t fps, fps_old;
	gm_node_mng_t scene_mng;
	gm_node_t* scene_node;
} gm_director_t;

static gm_director_t director = {
	.pause = 0,
	.wait = WAIT
};

#define _NOW \
	struct timeval tv; \
	gettimeofday(&tv, NULL); \
	if (director.now == tv.tv_usec) { \
		return; \
	} \
	director.now = tv.tv_usec; \

static void _show_fps(int16_t x, int16_t y, uint16_t fps)
{
	gm_lcd_write_abc(x, y, "fps");
	gm_lcd_write_num(x + 20, y, fps, 3);
}

static void _draw_scene(gm_node_t* scene_node)
{
	_NOW;
	gm_scene_dispatch(scene_node->p, director.now);
	uint32_t delta = DELTA(director.now, director.old);
	if ( delta >= director.wait) {
		director.old = director.now;
		director.fps = SEC / delta;

		gm_scene_render(scene_node->p);

		if (director.show_fps) {
			if (tv.tv_sec != director.now_s) {
				director.now_s = tv.tv_sec;
				director.fps_old = director.fps;
			}
			_show_fps(88, 0, director.fps_old);
		}

		gm_lcd_send_buffer();
		gm_lcd_clear_buffer();
	}
}

uint8_t gm_director_set_fps(uint16_t fps)
{
	if (fps && (fps <= GM_MAX_FPS)) {
		director.wait = SEC / fps;
		return 1;
	}
	return 0;
}

uint16_t gm_director_get_fps(void)
{
	return director.fps;
}

void gm_director_show_fps(uint8_t en)
{
	director.show_fps = en > 0;
}

uint8_t gm_director_run_scene(gm_node_t *node)
{
	if (node) {
		switch (node->type) {
		case GM_NODE_SCENE:
			gm_node_add(&director.scene_mng, node);
			director.scene_node = node;
			return 1;
		default:
			return 0;
		}
	}
	return 0;
}

uint8_t gm_director_replace_scene(gm_node_t *node)
{
	if (node) {
		switch (node->type) {
		case GM_NODE_SCENE:
			gm_node_remove(&director.scene_mng, director.scene_node);
			gm_node_add(&director.scene_mng, node);
			director.scene_node = node;
			gm_camera_reset();
			return 1;
		default:
			return 0;
		}
	}
	return 0;
}

void gm_director_remove_all_scene(void)
{
	gm_node_mng_free(&director.scene_mng);
}

uint8_t gm_director_pre_scene(void)
{
	if (director.scene_node->parent) {
		director.scene_node = director.scene_node->parent;
		return 1;
	}
	return 0;
}

uint8_t gm_director_next_scene(void)
{
	if (director.scene_node->child) {
		director.scene_node = director.scene_node->child;
		return 1;
	}
	return 0;
}

void gm_director_main_loop(void)
{
	if (!director.pause && director.scene_node) {
		_draw_scene(director.scene_node);
	}
}