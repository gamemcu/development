#include "gm_listener.h"
#include "gm_input.h"

static void _free(gm_node_t* node)
{
	GM_DBG("listener free");
	gm_listener_t* listener = (gm_listener_t*)node->p;
	listener->task.unref(listener->task.cb_ref);
	gm_node_free(node);
}

gm_node_t* gm_listener_create(int16_t prio, gm_task_t *task)
{
	if (task->type < GM_EVENT_COUNT) {
		gm_listener_t* listener = (gm_listener_t*)malloc(sizeof(gm_listener_t));
		if (listener == NULL) {
			return NULL;
		}
		memset(listener, 0, sizeof(gm_listener_t));
		memcpy(&listener->task, task, sizeof(gm_task_t));
		return gm_node_create(listener, GM_NODE_LISTENER, prio, _free);
	}
	return NULL;
}

uint8_t gm_listener_get_event(gm_event_t* event)
{
	int32_t v = gm_input_read();
	if ((v > -1) && (v != event->input)) {
		event->input = v;
		return 1;
	}
	return 0;
}

void gm_listener_visit(gm_listener_t* listener, gm_event_t* event)
{
	if (listener) {
		listener->task.info = event;
		task_post_medium (listener->task.handle, (task_param_t)&listener->task);
	}
}