#ifndef _GM_BASE_H
#define _GM_BASE_H

#include <string.h>
#include <stdlib.h>
#include "rom/ets_sys.h"
#include "esp_system.h"
#include "esp_log.h"

static const char* GM __attribute__((used)) = "gm";
#define GM_DBG(fmt, ...) \
        ESP_LOGE(GM, fmt, ##__VA_ARGS__)

typedef void (*pfunc_unref_t)(int);
typedef void (*pfunc_free_t)(void*);

#define SEC 1000000
#define WAIT (SEC/60) //60fps
#define DELTA(a, b) ((a)>=(b)?(a)-(b):SEC+(a)-(b))

#define GM_VERSION "dev"
#define GM_MAX_FPS 500 // 500 for test, the max fps is 100hz in fact, limited by oled fosc
#define GM_MAX_SPRITE_SPEED (1<<7)

#define GM_BMP_COPY(dest, src) \
	memcpy((dest), (src), sizeof(bitmap2_t)); \
	(dest)->data = (uint8_t*)malloc((src)->len); \
	if ((dest)->data == NULL) { \
		return NULL; \
	} \
	memcpy((dest)->data, (src)->data, (src)->len);

#endif