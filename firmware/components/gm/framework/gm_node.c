#include "gm_node.h"

void gm_node_ref(gm_node_t *node)
{
	if (node) {
		node->ref++;
	}
}

void gm_node_unref(gm_node_t *node)
{
	if (node) {
		if (node->ref) {
			node->ref--;
			if (!node->ref) {
				node->free(node);
			}
		}
	}
}

void gm_node_free(gm_node_t* node)
{
	if (node) {
		free(node->p);
		free(node);
	}
}

void gm_node_mng_free(gm_node_mng_t* mng)
{
	if (mng) {
		for (gm_node_t *node = mng->start; node; node = node->child) {
			gm_node_unref(node);
		}
	}
}

gm_node_t* gm_node_create(void *p, uint8_t type, int16_t z, pfunc_node_free_t free)
{
	gm_node_t* node = (gm_node_t*)malloc(sizeof(gm_node_t));
	if (node == NULL) {
		return NULL;
	}
	memset(node, 0, sizeof(gm_node_t));
	node->p = p;
	node->type = type;
	node->z = z;
	node->free = free;
	return node;
}

void gm_node_add(gm_node_mng_t *mng, gm_node_t *node)
{
	if (mng && node) {
		gm_node_ref(node);
		if ((mng->start == NULL) || (mng->end == NULL)) {
			mng->start = mng->end = node;
		}
		else {
			if (node->z <= mng->start->z) {
				node->child = mng->start;
				mng->start->parent = node;
				mng->start = node;
				mng->start->parent = NULL;
			}
			else if (node->z >= mng->end->z) {
				mng->end->child = node;
				node->parent = mng->end;
				mng->end = node;
				mng->end->child = NULL;
			}
			else {
				for (gm_node_t *child = mng->start->child; child; child = child->child) {
					if (node->z <= child->z) {
						node->child = child;
						node->parent = child->parent;
						child->parent->child = node;
						child->parent = node;
						break;
					}
				}
			}
		}
	}
}

void gm_node_remove(gm_node_mng_t *mng, gm_node_t *node)
{
	if (mng && node) {
		if (node == mng->start) {
			mng->start = mng->start->child;
		}
		if (node == mng->end) {
			mng->end = mng->end->parent;
		}
		if (node->parent) {
			node->parent->child = node->child;
		}
		if (node->child) {
			node->child->parent = node->parent;
		}
		gm_node_unref(node);
	}
}

void gm_node_z(gm_node_mng_t *mng, gm_node_t *node, int16_t z)
{
	if (mng && node) {
		if (z != node->z) {
			node->z = z;
			if (node->ref > 1) { //if node is added, lua create node will do ref++, so ref is 1 at least
				gm_node_ref(node); //refer node avoid auto gc, when do remove next
				gm_node_remove(mng, node);
				gm_node_add(mng, node); //add node, and node will be added with z
			}
		}
	}
}