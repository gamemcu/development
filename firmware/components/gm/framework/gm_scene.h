#ifndef _GM_SCENE_H
#define _GM_SCENE_H

#include "gm_node.h"
#include "gm_listener.h"
#include "gm_schduler.h"


typedef struct gm_scene_s
{
	gm_node_mng_t view_mng;
	gm_node_mng_t listener_mng;
	gm_node_mng_t schduler_mng;
	uint32_t old;
	gm_event_t event;
} gm_scene_t;

gm_node_t* gm_scene_create(void);
void gm_scene_render(gm_scene_t* scene);
uint8_t gm_scene_add(gm_scene_t* scene, gm_node_t* node);
uint8_t gm_scene_remove(gm_scene_t* scene, gm_node_t* node);
uint8_t gm_scene_set_z(gm_scene_t* scene, gm_node_t* node, int16_t z);
void gm_scene_dispatch(gm_scene_t* scene, uint32_t delta);

#endif
