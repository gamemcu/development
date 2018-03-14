#include "gm_led.h"
#include "platform.h"
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"

typedef struct
{
	TimerHandle_t tmr_blink, tmr_dura;
	uint8_t is_init, blink;
	uint8_t data[LED_DATA_LEN], len;
	int pin;
} gm_led_t;

gm_led_t led;
static const uint8_t *dark[LED_DATA_LEN] = {0};

void gm_led_setup(int pin)
{
	led.pin = pin;
	led.is_init = 1;
}

static uint8_t _led_write(uint8_t *data, uint8_t len)
{
	if (led.is_init) {
		if (platform_ws2812_setup( led.pin, 1, data, len) != PLATFORM_OK) {
			platform_ws2812_release();
			return 0;
		}
		if (platform_ws2812_send() != PLATFORM_OK) {
			platform_ws2812_release();
			return 0;
		}
		platform_ws2812_release();
		return 1;
	}
	return 0;
}

static void tmr_blink_callback(TimerHandle_t xTimer)
{
	led.blink = !led.blink;
	if (led.blink) {
		_led_write(led.data, led.len);
	}
	else {
		_led_write((uint8_t*)dark, led.len);
	}
}

static void tmr_dura_callback(TimerHandle_t xTimer)
{
	if (led.tmr_blink) {
		xTimerStop(led.tmr_blink, portMAX_DELAY);
		xTimerDelete(led.tmr_blink, portMAX_DELAY);
		led.tmr_blink = NULL;
	}
	_led_write((uint8_t*)dark, led.len);
}

uint8_t gm_led_run(gm_led_cfg_t *cfg)
{
	if (!led.is_init || !cfg || cfg->len > LED_DATA_LEN) {
		return 0;
	}
	if (led.tmr_dura) {
		xTimerStop(led.tmr_dura, portMAX_DELAY);
		xTimerDelete(led.tmr_dura, portMAX_DELAY);
		led.tmr_dura = NULL;
	}
	if (led.tmr_blink) {
		xTimerStop(led.tmr_blink, portMAX_DELAY);
		xTimerDelete(led.tmr_blink, portMAX_DELAY);
		led.tmr_blink = NULL;
	}
	_led_write((uint8_t*)dark, led.len);
	if (cfg->dura == 0) {
		return 1;
	}
	if (cfg->dura > 0) {
		led.tmr_dura = xTimerCreate("tmr_dura",
		                            pdMS_TO_TICKS(cfg->dura),
		                            pdFALSE, //once, pdTRUE continue
		                            NULL,
		                            tmr_dura_callback);
		if (!led.tmr_dura) {
			GMB_DBG("fail to create tmr_dura");
			return 0;
		}
	}
	memcpy(led.data, cfg->data, cfg->len);
	led.len = cfg->len;
	switch (cfg->mode) {
	case GM_LED_NORMAL:
		_led_write(led.data, led.len);
		break;
	case GM_LED_BLINK:
		if (cfg->freq) {
			led.tmr_blink = xTimerCreate("tmr_blink",
			                             pdMS_TO_TICKS(1000 / cfg->freq),
			                             pdTRUE, //once, pdTRUE continue
			                             NULL,
			                             tmr_blink_callback);
			if (!led.tmr_blink) {
				GMB_DBG("fail to create tmr_blink");
				return 0;
			}
		}
		break;
	default:
		return 0;
	}
	if (led.tmr_dura) {
		xTimerStart(led.tmr_dura, portMAX_DELAY);
	}
	if (led.tmr_blink) {
		xTimerStart(led.tmr_blink, portMAX_DELAY);
	}
	return 1;
}
