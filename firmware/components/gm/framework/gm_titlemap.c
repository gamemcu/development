#include "gm_titlemap.h"
#include "gm_camera.h"

static void _free(gm_node_t* node)
{
	GM_DBG("titlemap free");
	gm_titlemap_t *titlemap = (gm_titlemap_t*)node->p;
	free(titlemap->title.data);
	free(titlemap->map.data);
	gm_node_free(node);
}

gm_node_t* gm_titlemap_create(bitmap2_t *title, bitmap2_t *map, int16_t z)
{
	gm_titlemap_t* titlemap = (gm_titlemap_t*)malloc(sizeof(gm_titlemap_t));
	if (titlemap == NULL) {
		return NULL;
	}
	memset(titlemap, 0, sizeof(gm_titlemap_t));

	GM_BMP_COPY(&titlemap->title, title);
	GM_BMP_COPY(&titlemap->map, map);

	return gm_node_create(titlemap, GM_NODE_TITLEMAP, z, _free);
}

#define _is_solid(x, y) \
	gm_titlemap_is_soild(titlemap, x, y)

static uint8_t _get_title_idx(gm_titlemap_t *titlemap, int16_t x, int16_t y)
{
	if (_is_solid(x, y) == 0) {
		return 0;
	}
	uint8_t left = _is_solid(x - 1, y);
	uint8_t up = _is_solid(x, y - 1);
	uint8_t right = _is_solid(x + 1, y);
	uint8_t down = _is_solid(x, y + 1);
	uint8_t n = (left << 3) | (up << 2) | (right << 1) | down;
	switch (titlemap->title.data[BITMAP_FRAMES]) {
	case 2:
		switch (n) {
		case 15: return 0;
		default: return 1;
		}
	case 11:
		switch (n) {
		case 3: return 1;
		case 11: return 2;
		case 9: return 3;
		case 7: return 4;
		case 15: return 5;
		case 13: return 6;
		case 6: return 7;
		case 14: return 8;
		case 12: return 9;
		default: return 10;
		}
	default: return 0;
	}
}

uint8_t gm_titlemap_is_soild(gm_titlemap_t *titlemap, int16_t x, int16_t y)
{
	uint8_t *data = titlemap->map.data;
	uint8_t len = titlemap->map.len;
	uint8_t width = data[BITMAP_WIDTH];
	uint8_t height = data[BITMAP_HEIGHT];
	if ((x < 0) || (x > width - 1) ||
	        (y < 0) || (y > height - 1)) {
		return 1;
	}
	uint16_t i = x + (y >> 3) * width + BITMAP_DATA;
	if (i < len) {
		uint8_t b = data[i];
		b = b >> (y & 7);
		return b & 1;
	}
	return 1;
}

void gm_titlemap_visit(gm_titlemap_t *titlemap, int16_t xoffset, int16_t yoffset)
{
	if (titlemap) {
		uint8_t offset = 4;
		switch (titlemap->title.data[BITMAP_WIDTH]) {
		case 8: offset = 3; break;
		case 16: offset = 4; break;
		case 32: offset = 5; break;
		default: return;
		}
		//here could be optimized, when xoffset and yoffset not equal 0
		int16_t xstart = gm_camera()->x >> offset;
		int16_t ystart = gm_camera()->y >> offset;
		int16_t xend = xstart + (gm_screen_info()->width >> offset);
		int16_t yend = ystart + (gm_screen_info()->height >> offset);
		for (int16_t y = ystart; y <= yend; y++) {
			int16_t yy = (y << offset) - gm_camera()->y;
			for (int16_t x = xstart; x <= xend; x++) {
				int16_t xx = (x << offset) - gm_camera()->x;
				uint8_t i = _get_title_idx(titlemap, x, y);
				gm_lcd_draw_bmp2(&titlemap->title, xx + xoffset, yy + yoffset, i);
			}
		}
	}
}