#include "gm_layer.h"
#include "gm_label.h"
#include "gm_sprite.h"
#include "gm_titlemap.h"
#include "gm_starsky.h"

static void _free(gm_node_t *node)
{
	GM_DBG("layer free");
	gm_layer_t* layer = (gm_layer_t *)node->p;
	gm_node_mng_free(&layer->view_mng);
	gm_node_free(node);
}

gm_node_t* gm_layer_create(int16_t x, int16_t y, int16_t z)
{
	gm_layer_t* layer = (gm_layer_t*)malloc(sizeof(gm_layer_t));
	if (layer == NULL) {
		return NULL;
	}
	memset(layer, 0, sizeof(gm_layer_t));
	layer->x = x;
	layer->y = y;
	return gm_node_create(layer, GM_NODE_LAYER, z, _free);
}

uint8_t gm_layer_add(gm_layer_t *layer, gm_node_t* node)
{
	if (layer && node) {
		switch (node->type) {
		case GM_NODE_SPRITE:
		case GM_NODE_LABEL:
		case GM_NODE_TITLEMAP:
		case GM_NODE_STARSKY:
			gm_node_add(&layer->view_mng, node);
			break;
		default:
			GM_DBG("layer not support node type:%x", node->type);
			return 0;
		}
		return 1;
	}
	return 0;
}

uint8_t gm_layer_remove(gm_layer_t *layer, gm_node_t* node)
{
	if (layer && node) {
		switch (node->type) {
		case GM_NODE_SPRITE:
		case GM_NODE_LABEL:
		case GM_NODE_TITLEMAP:
		case GM_NODE_STARSKY:
			gm_node_remove(&layer->view_mng, node);
			break;
		default:
			GM_DBG("layer not support node type:%x", node->type);
			return 0;
		}
		return 1;
	}
	return 0;
}

uint8_t gm_layer_set_z(gm_layer_t *layer, gm_node_t* node, int16_t z)
{
	if (layer && node) {
		switch (node->type) {
		case GM_NODE_SPRITE:
		case GM_NODE_LABEL:
		case GM_NODE_TITLEMAP:
		case GM_NODE_STARSKY:
			gm_node_z(&layer->view_mng, node, z);
			return 1;
		default:
			GM_DBG("layer not support node type:%x", node->type);
			return 0;
		}
		return 1;
	}
	return 0;
}

void gm_layer_visit(gm_layer_t *layer)
{
	if (layer) {
		for (gm_node_t * node = layer->view_mng.start; node; node = node->child) {
			switch (node->type) {
			case GM_NODE_SPRITE:
				gm_sprite_visit(node->p, layer->x, layer->y);
				break;
			case GM_NODE_LABEL:
				gm_label_visit(node->p, layer->x, layer->y);
				break;
			case GM_NODE_TITLEMAP:
				gm_titlemap_visit(node->p, layer->x, layer->y);
				break;
			case GM_NODE_STARSKY:
				gm_starsky_visit(node->p, layer->x, layer->y);
				break;
			default:;
			}
		}
	}
}