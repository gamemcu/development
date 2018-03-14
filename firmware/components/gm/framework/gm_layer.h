#ifndef _GM_LAYER_H
#define _GM_LAYER_H

#include "gm_node.h"
#include "gm_physics.h"

typedef struct
{
	gm_node_mng_t view_mng;
	int16_t x, y;
} gm_layer_t;

gm_node_t* gm_layer_create(int16_t x, int16_t y, int16_t z);
uint8_t gm_layer_add(gm_layer_t *layer, gm_node_t* node);
uint8_t gm_layer_remove(gm_layer_t *layer, gm_node_t* node);
uint8_t gm_layer_set_z(gm_layer_t *layer, gm_node_t* node, int16_t z);
void gm_layer_visit(gm_layer_t *layer);

#endif