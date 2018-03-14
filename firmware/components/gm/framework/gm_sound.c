#include "gm_sound.h"
#include "driver/ledc.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

typedef struct $
{
	TaskHandle_t handle;
	uint32_t freq, dura;
	uint8_t type;
} gm_sound_t;

gm_sound_t sound;

void gm_sound_setup(int pin)
{
	ledc_timer_config_t timer_conf;
	timer_conf.speed_mode = LEDC_HIGH_SPEED_MODE;
	timer_conf.bit_num    = LEDC_TIMER_10_BIT;
	timer_conf.timer_num  = LEDC_TIMER_0;
	timer_conf.freq_hz    = 600;
	ledc_timer_config(&timer_conf);

	ledc_channel_config_t ledc_conf;
	ledc_conf.gpio_num   = pin;
	ledc_conf.speed_mode = LEDC_HIGH_SPEED_MODE;
	ledc_conf.channel    = LEDC_CHANNEL_0;
	ledc_conf.intr_type  = LEDC_INTR_DISABLE;
	ledc_conf.timer_sel  = LEDC_TIMER_0;
	ledc_conf.duty       = 0; // 50%=0x3FFF, 100%=0x7FFF for 15 Bit
	// 50%=0x01FF, 100%=0x03FF for 10 Bit
	ledc_channel_config(&ledc_conf);
}

void _tone(uint32_t freq, uint32_t duration)
{
	if (freq && duration) {
		ledc_set_freq(LEDC_HIGH_SPEED_MODE, LEDC_TIMER_0, freq);
		ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, 0x01FF); // 50% duty; 0x7F 12% duty - play here for your speaker or buzzer
		ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
		vTaskDelay(duration / portTICK_PERIOD_MS);
		ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, 0);
		ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
	}
}

#if 0
void gm_sound_task(void* arg)
{
	gm_song_t *song = (gm_song_t *)arg;
	for (uint16_t i = 0; song[i]; i += 3) {
		_tone(song[i], song[i + 1]);
		vTaskDelay(song[i + 2] / portTICK_PERIOD_MS);
	}
	sound_handle = NULL;
	vTaskDelete(sound_handle);
}

static const gm_song_t asset_song_demo[] = {
	660, 100, 150,
	660, 100, 300,
	660, 100, 300,
	510, 100, 100,
	660, 100, 330,
	770, 100, 550,
	380, 100, 575,
	510, 100, 450,
	380, 100, 400,
	320, 100, 500,
	440, 100, 300,
	480, 80 , 330,
	450, 100, 150,
	430, 100, 300,
	380, 100, 200,
	660, 80 , 200,
	760, 50 , 150,
	860, 100, 300,
	700, 80 , 150,
	760, 50 , 350,
	660, 80 , 300,
	520, 80 , 150,
	580, 80 , 150,
	480, 80 , 500,
	0
};

static const gm_song_t asset_song_1[] = {
	880, 20, 0,
	0
};

uint8_t gm_sound_play(uint16_t i)
{
	if (sound_handle) {
		vTaskDelete(sound_handle);
		sound_handle = NULL;
	}
	gm_song_t *song = asset_song_demo;
	switch (i) {
	case 0: song = asset_song_demo;
		break;
	case 1: song = asset_song_1;
		break;
	default:
		return 0;
	}
	if (pdTRUE == xTaskCreate(gm_sound_task, "gm_sound_task", 2048, (void*)song, 5, &sound_handle)) {
		return 1;
	}
	return 0;
}
#endif

void gm_sound_tone_task(void *arg)
{
	_tone(sound.freq, sound.dura);
	sound.handle = NULL;
	vTaskDelete(sound.handle);
}

uint8_t gm_sound_tone(uint32_t freq, uint32_t dura)
{
	if (sound.handle) {
		vTaskDelete(sound.handle);
		sound.handle = NULL;
	}
	sound.freq = freq;
	sound.dura = dura;
	if (pdTRUE == xTaskCreate(gm_sound_tone_task, "gm_sound_tone_task", 2048, NULL, 5, &sound.handle)) {
		return 1;
	}
	return 0;
}
