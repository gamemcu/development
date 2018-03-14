#ifndef _GM_LISTENER_H
#define _GM_LISTENER_H

#include "gm_node.h"

enum
{
	GM_EVENT_INPUT = 0,
	GM_EVENT_NET,
	GM_EVENT_COUNT
};

typedef struct
{
	uint16_t input;
} gm_event_t;

typedef struct
{
	gm_task_t task;
} gm_listener_t;

gm_node_t* gm_listener_create(int16_t prio, gm_task_t *task);
uint8_t gm_listener_get_event(gm_event_t* event);
void gm_listener_visit(gm_listener_t* listener, gm_event_t* event);

#endif