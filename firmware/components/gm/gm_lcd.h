#ifndef _GM_LCD_H
#define _GM_LCD_H

#include "gm_board.h"
#include "gm_assets.h"

typedef struct {
	uint8_t width;
	uint8_t height;
	uint8_t* screen;
	uint16_t screen_size;
} screen_info_t;

typedef struct
{
	uint8_t reinit;
	int host, sclk, mosi, cs, dc, rst;
} gm_lcd_cfg_t;

typedef struct
{
	screen_info_t screen_info;
	spi_device_handle_t spi;
	uint8_t is_init;
} gm_lcd_t;

screen_info_t* gm_screen_info(void);
esp_err_t gm_lcd_setup(gm_lcd_cfg_t* cfg);
void gm_lcd_clear_buffer(void);
esp_err_t gm_lcd_send_buffer(void);
void gm_lcd_write_ascii(int16_t x, int16_t y, const char* str);
void gm_lcd_write_str(int16_t x, int16_t y, const char* str);
void gm_lcd_write_abc(int16_t x, int16_t y, const char* abc);
void gm_lcd_write_num(int16_t x, int16_t y, uint16_t num, uint8_t limit);
void gm_lcd_draw_bmp2(bitmap2_t *bmp, uint16_t x, uint16_t y, uint8_t i);
void gm_lcd_draw_point(int16_t x, int16_t y, uint8_t color);

#endif