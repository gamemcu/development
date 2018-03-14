#include "gm_schduler.h"

static void _free(gm_node_t* node)
{
	gm_schduler_t* schduler = (gm_schduler_t*)node->p;
	GM_DBG("gm_schduler_free:%d", schduler->task.cb_ref);
	schduler->task.unref(schduler->task.cb_ref);
	gm_node_free(node);
}

gm_node_t* gm_schduler_create(int16_t prio, uint32_t interval, gm_task_t *task)
{
	if ((task->type < GM_SCH_COUNT) && (interval < GM_SCH_MAX_INTERVAL)) {
		gm_schduler_t* sch = (gm_schduler_t*)malloc(sizeof(gm_schduler_t));
		if (sch == NULL) {
			return NULL;
		}
		memset(sch, 0, sizeof(gm_schduler_t));
		memcpy(&sch->task, task, sizeof(gm_task_t));
		sch->wait = interval * 1000;
		return gm_node_create(sch, GM_NODE_SCHDULER, prio, _free);
	}
	return NULL;
}

uint8_t gm_schduler_visit(gm_schduler_t* schduler, uint32_t delta)
{
	if (schduler) {
		if (schduler->task.type & 0x80) {
			return 1;
		}
		schduler->delta += delta;
		if (schduler->delta >= schduler->wait) {
			schduler->delta = 0;
			task_post_high(schduler->task.handle, (task_param_t)&schduler->task);
			if (schduler->task.type == GM_SCH_ONCE) {
				schduler->task.type |= 0x80;
			}
		}
	}
	return 0;
}




