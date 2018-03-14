#include "gm_lcd.h"

DRAM_ATTR static const uint8_t lcdBootProgram[] = {
	// boot defaults are commented out but left here incase they
	// might prove useful for reference
	//
	// Further reading: https://www.adafruit.com/datasheets/SSD1306.pdf
	//
	// Display Off
	// 0xAE,

	// Set Display Clock Divisor v = 0xF0
	// default is 0x80
	0xD5, 0x00,

	// Set Multiplex Ratio v = 0x3F
	0xA8, 0x3F,

	// Set Display Offset v = 0
	// 0xD3, 0x00,

	// Set Start Line (0)
	// 0x40,

	// Charge Pump Setting v = enable (0x14)
	// default is disabled
	0x8D, 0x14,

	// Set Segment Re-map (A0) | (b0001)
	// default is (b0000)
	0xA0,// 0xA1,

	// Set COM Output Scan Direction
	0xC0,// 0xC8,

	// Set COM Pins v
	// 0xDA, 0x12,

	// Set Contrast v = 0xCF
	0x81, 0xFF,

	// Set Precharge = 0xF1
	0xD9, 0xF1,

	// Set VCom Detect
	// 0xDB, 0x40,

	// Entire Display ON
	// 0xA4,

	// Set normal/inverse display
	// 0xA6,

	// Display On
	0xAF,

	// set display mode = horizontal addressing mode (0x00)
	0x20, 0x00,

	// set col address range
	// 0x21, 0x00, COLUMN_ADDRESS_END,

	// set page address range
	// 0x22, 0x00, PAGE_ADDRESS_END
};

static const uint8_t lowbits[] = {
	0xff, 0xfe, 0xfc, 0xf8, 0xf0, 0xe0, 0xc0, 0x80, 0x00
};

static const uint8_t highbits[] = {
	0x00, 0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f, 0xff
};

#define SCREEN_BUFFER_SIZE 128*(64/8)
static uint8_t _screen_buffer[SCREEN_BUFFER_SIZE];
static gm_lcd_t s_gm_lcd = {
	.screen_info = {
		.width = 128,
		.height = 64,
		.screen = _screen_buffer,
		.screen_size = SCREEN_BUFFER_SIZE
	},
	.spi = NULL,
	.is_init = 0,
};

screen_info_t* gm_screen_info(void)
{
	return &s_gm_lcd.screen_info;
}

static void _draw_h8(screen_info_t *screen_info, int16_t x0, int16_t y0, uint16_t w, uint16_t h, uint8_t *d, uint16_t l, uint8_t rendermode)
{
	int16_t x, pg, i, k, screen_start_index, pic_start_index;
	uint8_t offset;
	uint16_t pic_x, pic_xstart, pic_xend, pic_pg, pic_pgstart, pic_pgend;
	if ((x0 + w < 0) || (x0 > screen_info->width - 1) ||
	        (y0 + h < 0) || (y0 > screen_info->height - 1) ||
	        (w == 0) || (h == 0) || (l == 0)) {
		return;
	}
	x = x0;
	pg = y0 >> 3;
	offset = y0 & 7;
	pic_xstart = 0;
	pic_xend = w;
	pic_pgstart = 0;
	pic_pgend = (h + 7) >> 3;
	if (x0 < 0) {
		pic_xstart = -x0;
		x = 0;
	}
	if (x0 + w > screen_info->width) {
		pic_xend = screen_info->width - x0;
	}
	if (y0 < 0) {
		pic_pgstart = (-y0) >> 3;
		offset = (8 - (-y0)) & 7;
		pg = offset ? -1 : 0;
	}
	if (y0 + h > screen_info->height) {
		pic_pgend = (screen_info->height - y0 + 7) >> 3;
		offset = (8 - (screen_info->height - y0)) & 7;
	}
	screen_start_index = pg * screen_info->width;
	uint16_t ww = w;
	if (rendermode & DRAW_GRAY) {
		ww += w;
	}
	if (rendermode & DRAW_MASK) {
		ww += w;
	}
	pic_start_index = pic_pgstart * ww;
	uint8_t highbitsmask = highbits[offset];
	uint8_t lowbitsmask = lowbits[offset];
	if (rendermode & DRAW_TRANSPARENCY) {
		highbitsmask = 0xff;
		lowbitsmask = 0xff;
	}
	for (pic_x = pic_xstart; pic_x < pic_xend; pic_x++, x++) {
		uint16_t pic_xx = pic_x;
		i = screen_start_index + x;
		if (rendermode & DRAW_GRAY) {
			pic_xx += pic_x;
		}
		if (rendermode & DRAW_MASK) {
			pic_xx += pic_x;
		}
		k = pic_start_index + pic_xx;
		if (k < l) {
			for (pic_pg = pic_pgstart; pic_pg < pic_pgend; pic_pg++) {
				uint8_t v = d[k];
				uint8_t kk = k;
				if (rendermode & DRAW_GRAY) {
					if (++kk < l) {
						d[k] ^= d[kk];
					}
				}
				if (rendermode & DRAW_REVERSE) {
					v = ~v;
				}
				uint8_t v1 = v << offset;
				uint8_t v2 = v >> (8 - offset);
				if (rendermode & DRAW_MASK) {
					if (++kk < l) {
						highbitsmask = highbits[offset] | (d[kk] << offset);
						lowbitsmask = lowbits[offset] | (d[kk] >> (8 - offset));
						v1 &= ~highbitsmask;
						v2 &= ~lowbitsmask;
					}
				}
				if (i >= 0) {
					screen_info->screen[i] = (screen_info->screen[i] & highbitsmask) | v1;
				}
				if (i + screen_info->width < screen_info->screen_size && offset) {
					screen_info->screen[i + screen_info->width] = (screen_info->screen[i + screen_info->width] & lowbitsmask) | v2;
				}
				i += screen_info->width;
				k += ww;
			}
		}
	}
}

static esp_err_t _spi_transmit(spi_device_handle_t spi, uint8_t* data, size_t len)
{
	if (len == 0) {
		return ESP_OK;
	}
	spi_transaction_t trans;
	memset( &trans, 0, sizeof( trans ) );
	trans.length = len << 3; //trans to bits
	trans.tx_buffer = data;
	return spi_device_transmit(spi, &trans);
}

static esp_err_t _spi_setup(gm_lcd_cfg_t* cfg)
{
	spi_bus_config_t buscfg = {
		.miso_io_num = -1,
		.mosi_io_num = cfg->mosi,
		.sclk_io_num = cfg->sclk,
		.quadwp_io_num = -1,
		.quadhd_io_num = -1
	};
	if (spi_bus_initialize(cfg->host, &buscfg, 1) == ESP_FAIL) {
		return ESP_FAIL;
	}
	// set up the spi device
	spi_device_interface_config_t devcfg = {
		.clock_speed_hz = 10000000UL,
		.mode = 0,
		.spics_io_num = cfg->cs,
		.queue_size = 1
	};
	return spi_bus_add_device( cfg->host, &devcfg, &s_gm_lcd.spi );
}

esp_err_t gm_lcd_setup(gm_lcd_cfg_t* cfg)
{
	if (s_gm_lcd.is_init && !cfg->reinit) {
		return ESP_OK;
	}
	if (cfg == NULL) {
		return ESP_FAIL;
	}

	if (_spi_setup(cfg) == ESP_FAIL) {
		return ESP_FAIL;
	}
	gpio_config_t io_conf = {
		.intr_type = GPIO_PIN_INTR_DISABLE,
		.mode = GPIO_MODE_OUTPUT,
		.pin_bit_mask = (1 << cfg->rst) | (1 << cfg->dc)
	};
	gpio_config(&io_conf);
	gpio_set_level(cfg->rst, 0); //reset chip
	ets_delay_us(5000); //delay 5ms
	gpio_set_level(cfg->rst, 1);
	ets_delay_us(5000);
	gpio_set_level(cfg->dc, 0); //cmd mode
	esp_err_t ret = _spi_transmit(s_gm_lcd.spi, (uint8_t*)lcdBootProgram, sizeof(lcdBootProgram));
	gpio_set_level(cfg->dc, 1); //data mode
	s_gm_lcd.is_init = 1;
	gm_lcd_clear_buffer();
	gm_lcd_send_buffer();
	return ret;
}

void gm_lcd_draw_bmp2(bitmap2_t *bmp, uint16_t x, uint16_t y, uint8_t i)
{
	uint16_t offset = 4;
	uint8_t width = bmp->data[0];
	uint8_t height = bmp->data[1];
	uint8_t frames = bmp->data[2];
	if (i < frames) {
		uint16_t tmp = i * bmp->framesize;
		offset += tmp;
		if (bmp->rendermode & DRAW_GRAY) {
			offset += tmp;
		}
		if (bmp->rendermode & DRAW_MASK) {
			offset += tmp;
		}
		if (offset < bmp->len) {
			_draw_h8(&s_gm_lcd.screen_info, x, y, width, height, (uint8_t*)bmp->data + offset, bmp->len - offset, bmp->rendermode);
		}
	}
}

void gm_lcd_draw_point(int16_t x, int16_t y, uint8_t color)
{
	if ( (x < 0) || (x > gm_screen_info()->width - 1) ||
	        (y < 0) || (y > gm_screen_info()->height - 1)) {
		return;
	}
	uint16_t i = x + (y >> 3) * gm_screen_info()->width;
	if (i < gm_screen_info()->screen_size) {
		uint8_t *b = gm_screen_info()->screen + i;
		if (color > 0) {
			*b |= 1 << (y & 7);
		}
		else {
			*b &= ~(1 << (y & 7));
		}
	}
}

void gm_lcd_write_ascii(int16_t x, int16_t y, const char* str)
{
	for (uint8_t i = 0; str[i] && i < 255; i++) {
		gm_lcd_draw_bmp2(&asset_ascii, x + i * 5, y, str[i] - ' ');
	}
}

void gm_lcd_write_str(int16_t x, int16_t y, const char* str)
{
	for (uint8_t i = 0; str[i] && i < 255; i++) {
		if ((str[i] >= '0') && (str[i] <= '9')) {
			gm_lcd_draw_bmp2(&asset_num, x + i * 6, y, str[i] - '0');
		}
		else if ((str[i] >= 'a') && (str[i] <= 'z')) {
			gm_lcd_draw_bmp2(&asset_abc, x + i * 6, y, str[i] - 'a');
		}
	}
}

void gm_lcd_write_abc(int16_t x, int16_t y, const char* abc)
{
	for (uint8_t i = 0; abc[i] && i < 255; i++) {
		gm_lcd_draw_bmp2(&asset_abc, x + i * 6, y, abc[i] - 'a');
	}
}

void gm_lcd_write_num(int16_t x, int16_t y, uint16_t num, uint8_t limit)
{
	x += (limit - 1) * 6;
	for (uint8_t i = 0; i < limit; i++) {
		gm_lcd_draw_bmp2(&asset_num, x - i * 6, y, num % 10);
		num /= 10;
	}
}

void gm_lcd_clear_buffer(void)
{
	memset(s_gm_lcd.screen_info.screen, 0, s_gm_lcd.screen_info.screen_size);
}

esp_err_t gm_lcd_send_buffer(void)
{
	if (!s_gm_lcd.is_init) {
		return ESP_FAIL;
	}
	return _spi_transmit(s_gm_lcd.spi, s_gm_lcd.screen_info.screen, s_gm_lcd.screen_info.screen_size);
}
