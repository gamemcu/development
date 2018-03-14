#include "gm_scene.h"
#include "gm_label.h"
#include "gm_sprite.h"
#include "gm_titlemap.h"
#include "gm_layer.h"

static void _free(gm_node_t* node)
{
	GM_DBG("gm_scene_free");
	gm_scene_t *scene = (gm_scene_t *)node->p;
	gm_node_mng_free(&scene->view_mng);
	gm_node_mng_free(&scene->listener_mng);
	gm_node_mng_free(&scene->schduler_mng);
	gm_node_free(node);
}

gm_node_t* gm_scene_create(void)
{
	gm_scene_t* scene = (gm_scene_t*)malloc(sizeof(gm_scene_t));
	if (scene == NULL) {
		return NULL;
	}
	memset(scene, 0, sizeof(gm_scene_t));
	return gm_node_create(scene, GM_NODE_SCENE, 0, _free);
}

uint8_t gm_scene_add(gm_scene_t* scene, gm_node_t* node)
{
	if (scene && node) {
		switch (node->type) {
		case GM_NODE_SPRITE:
		case GM_NODE_LABEL:
		case GM_NODE_TITLEMAP:
		case GM_NODE_LAYER:
			gm_node_add(&scene->view_mng, node);
			return 1;
		case GM_NODE_LISTENER:
			gm_node_add(&scene->listener_mng, node);
			return 1;
		case GM_NODE_SCHDULER:
			gm_node_add(&scene->schduler_mng, node);
			return 1;
		default:
			GM_DBG("not support node type");
			return 0;
		}
	}
	return 0;

}

uint8_t gm_scene_remove(gm_scene_t* scene, gm_node_t* node)
{
	if (scene && node) {
		switch (node->type) {
		case GM_NODE_SPRITE:
		case GM_NODE_LABEL:
		case GM_NODE_TITLEMAP:
		case GM_NODE_LAYER:
			gm_node_remove(&scene->view_mng, node);
			return 1;
		case GM_NODE_LISTENER:
			gm_node_remove(&scene->listener_mng, node);
			return 1;
		case GM_NODE_SCHDULER:
			gm_node_remove(&scene->schduler_mng, node);
			return 1;
		default:
			GM_DBG("not support node type");
			return 0;
		}
	}
	return 0;
}

uint8_t gm_scene_set_z(gm_scene_t* scene, gm_node_t* node, int16_t z)
{
	if (scene && node) {
		switch (node->type) {
		case GM_NODE_SPRITE:
		case GM_NODE_LABEL:
		case GM_NODE_TITLEMAP:
		case GM_NODE_LAYER:
			gm_node_z(&scene->view_mng, node, z);
			return 1;
		case GM_NODE_LISTENER:
			gm_node_z(&scene->listener_mng, node, z);
			return 1;
		case GM_NODE_SCHDULER:
			gm_node_z(&scene->schduler_mng, node, z);
			return 1;
		default:
			GM_DBG("not support node type");
			return 0;
		}
		return 1;
	}
	return 0;
}

void gm_scene_dispatch(gm_scene_t* scene, uint32_t now)
{
	if (scene) {
		uint32_t delta = DELTA(now, scene->old);
		if (delta >= 500) { // >= 500us
			scene->old = now;
			for (gm_node_t *node = scene->schduler_mng.start; node; node = node->child) { //err
				if (gm_schduler_visit(node->p, delta)) {
					gm_scene_remove(scene, node);
				}
			}
		}
	}
}

static void _listen(gm_scene_t* scene)
{
	if (gm_listener_get_event(&scene->event)) {
		for (gm_node_t *node = scene->listener_mng.start; node; node = node->child) { //err
			gm_listener_visit(node->p, &scene->event);
		}
	}
}

void gm_scene_render(gm_scene_t* scene)
{
	if (scene) {
		for (gm_node_t *node = scene->view_mng.start; node; node = node->child) {
			switch (node->type) {
			case GM_NODE_SPRITE:
				gm_sprite_visit(node->p, 0, 0);
				break;
			case GM_NODE_LABEL:
				gm_label_visit(node->p, 0, 0);
				break;
			case GM_NODE_TITLEMAP:
				gm_titlemap_visit(node->p, 0, 0);
				break;
			case GM_NODE_LAYER:
				gm_layer_visit(node->p);
				break;
			default:;
			}
		}
		// for (gm_node_t *node = scene->view_mng.start; node; node = node->child) {
		// 	if (GM_NODE_SPRITE == node->type) {
		// 		if (gm_sprite_is_moving(node->p)) {
		// 			for (gm_node_t *parent = node->parent; parent; parent = node->parent) {
						
		// 			}
		// 			for (gm_node_t *child = node->child; child; child = node->child ) {

		// 			}
		// 		}
		// 	}
		// }
		_listen(scene);
	}
}