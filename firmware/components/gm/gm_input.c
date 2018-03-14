#include "gm_input.h"

#define _INVALID_RANGE 500
#define _HYS_RANGE 50
#define _HYS_0 (_INVALID_RANGE/2)
#define _HYS_1 (_INVALID_RANGE/2+_HYS_RANGE)

#define GM_INPUT_SET(b) input.v|=1<<(b)
#define GM_INPUT_CLR(b) input.v&=~(1<<(b))
#define GM_INPUT_INIT_COUNT 10
typedef struct
{
	gm_input_cfg_t cfg;
	uint8_t is_init;
	uint16_t xthr, ythr, v;
} gm_input_t;

static gm_input_t input;

static void gm_input_init(void)
{
	uint8_t i;
	uint32_t xthr_sum = 0, ythr_sum = 0;
	for (i = 0; i < GM_INPUT_INIT_COUNT; i++) {
		int x = adc1_get_voltage(input.cfg.x);
		int y = adc1_get_voltage(input.cfg.y);
		xthr_sum += x;
		ythr_sum += y;
	}
	input.xthr = xthr_sum / i;
	input.ythr = ythr_sum / i;
	input.is_init = 1;
}

uint8_t gm_input_setup(gm_input_cfg_t* cfg)
{
	if (input.is_init && !cfg->reinit){
		return 1;
	}
	// adc1_config_width(ADC_WIDTH_12Bit);
	if (adc1_config_channel_atten(cfg->x, ADC_ATTEN_11db) != ESP_OK) {
		return 0;
	}
	if (adc1_config_channel_atten(cfg->y, ADC_ATTEN_11db) != ESP_OK) {
		return 0;
	}
	gpio_config_t io_conf = {
		.intr_type = GPIO_PIN_INTR_DISABLE,
		.mode = GPIO_MODE_INPUT,
		.pull_down_en = 0,
		.pull_up_en = 1
	};
	io_conf.pin_bit_mask = (1ULL << cfg->a) | (1ULL << cfg->b) | (1ULL << cfg->c) | (1ULL << cfg->d);
	if (gpio_config(&io_conf) != ESP_OK) {
		return 0;
	}
	memcpy(&input.cfg, cfg, sizeof(gm_input_cfg_t));
	gm_input_init();
	return 1;
}

int32_t gm_input_read(void)
{
	if (!input.is_init) {
		return -1;
	}
	int x = adc1_get_voltage(input.cfg.x);
	int y = adc1_get_voltage(input.cfg.y);
	if (x < input.xthr - _HYS_0) {
		GM_INPUT_SET(GM_INPUT_LEFT);
	}
	if (x > input.xthr + _HYS_0) {
		GM_INPUT_SET(GM_INPUT_RIGHT);
	}
	if (x > input.xthr - _HYS_1) {
		GM_INPUT_CLR(GM_INPUT_LEFT);
	}
	if (x < input.xthr + _HYS_1) {
		GM_INPUT_CLR(GM_INPUT_RIGHT);
	}
	if (y < input.ythr - _HYS_0) {
		GM_INPUT_SET(GM_INPUT_UP);
	}
	if (y > input.ythr + _HYS_0) {
		GM_INPUT_SET(GM_INPUT_DOWN);
	}
	if (y > input.ythr - _HYS_1) {
		GM_INPUT_CLR(GM_INPUT_UP);
	}
	if (y < input.ythr + _HYS_1) {
		GM_INPUT_CLR(GM_INPUT_DOWN);
	}
	if (gpio_get_level (input.cfg.a)) {
		GM_INPUT_SET(GM_INPUT_A);
	}
	else {
		GM_INPUT_CLR(GM_INPUT_A);
	}
	if (gpio_get_level (input.cfg.b)) {
		GM_INPUT_SET(GM_INPUT_B);
	}
	else {
		GM_INPUT_CLR(GM_INPUT_B);
	}
	if (gpio_get_level (input.cfg.c)) {
		GM_INPUT_SET(GM_INPUT_C);
	}
	else {
		GM_INPUT_CLR(GM_INPUT_C);
	}
	if (gpio_get_level (input.cfg.d)) {
		GM_INPUT_SET(GM_INPUT_D);
	}
	else {
		GM_INPUT_CLR(GM_INPUT_D);
	}
	return input.v;
}
