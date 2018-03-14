#ifndef _GM_SCHDULER_H
#define _GM_SCHDULER_H

#include "gm_node.h"
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"

#define GM_SCH_MAX_INTERVAL 6870947 //SDK 1.5.3 limit (0x68D7A3)
enum
{
	GM_SCH_CONTINUE = 0,
	GM_SCH_ONCE,
	GM_SCH_COUNT
};

typedef struct
{
	gm_task_t task;
	uint8_t in_wait;
	uint32_t delta, wait;
} gm_schduler_t;

gm_node_t* gm_schduler_create(int16_t prio, uint32_t interval, gm_task_t *task);
uint8_t gm_schduler_visit(gm_schduler_t* schduler, uint32_t delta);
#endif