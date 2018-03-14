#ifndef _GM_NODE_H
#define _GM_NODE_H

#include "gm_base.h"
#include "task/task.h"

typedef struct
{
	void *info;
	uint8_t type;
	int cb_ref;
	pfunc_unref_t unref;
	task_handle_t handle;
} gm_task_t;

enum {
	GM_NODE_SPRITE = 1,
	GM_NODE_LABEL,
	GM_NODE_TITLEMAP,
	GM_NODE_STARSKY,
	GM_NODE_PARTICLE,
	GM_NODE_EMITER,
	GM_NODE_LAYER,
	GM_NODE_LISTENER,
	GM_NODE_SCHDULER,
	GM_NODE_SCENE
};

typedef struct gm_node_s
{
	void *p;
	uint8_t type;
	uint8_t ref;
	int16_t z;
	struct gm_node_s *parent, *child;
	void (*free)(struct gm_node_s* node);
} gm_node_t;

typedef struct
{
	gm_node_t *start, *end;
} gm_node_mng_t;

typedef void (*pfunc_node_free_t)(gm_node_t* node);

#define GM_REF(a) gm_node_ref(a)
#define GM_UNREF(a) gm_node_unref(a)

void gm_node_ref(gm_node_t *node);
void gm_node_unref(gm_node_t *node);
void gm_node_free(gm_node_t* node);
void gm_node_mng_free(gm_node_mng_t* mng);
gm_node_t* gm_node_create(void *p, uint8_t type, int16_t z, pfunc_node_free_t free);
void gm_node_add(gm_node_mng_t *mng, gm_node_t *node);
void gm_node_remove(gm_node_mng_t *mng, gm_node_t *node);
void gm_node_z(gm_node_mng_t *mng, gm_node_t *node, int16_t z);

#endif