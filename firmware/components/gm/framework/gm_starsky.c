#include "gm_starsky.h"
#include "gm_lcd.h"
#include "gm_camera.h"

#define _ODD(a) (((((a)+1)>>1)<<1)-1)
#define _ABS(a) ((a)?a:-a)

static void _free(gm_node_t *node)
{
	GM_DBG("starsky free");
	gm_starsky_t *starsky = (gm_starsky_t *)node->p;
	free(starsky->stars);
	gm_node_free(node);
}

gm_node_t* gm_starsky_create(uint8_t n, int16_t z, uint8_t color)
{
	gm_starsky_t *starsky = (gm_starsky_t *)malloc(sizeof(gm_starsky_t));
	if (starsky == NULL) {
		return NULL;
	}
	memset(starsky, 0, sizeof(gm_starsky_t));
	starsky->color = color;
	starsky->n = n;
	starsky->stars = (gm_star_t*)malloc(sizeof(gm_star_t) * n);
	if (starsky->stars == NULL) {
		return NULL;
	}
	memset(starsky->stars, 0, sizeof(gm_star_t) * n);
	starsky->width = _ODD(gm_screen_info()->width);
	starsky->height = _ODD(gm_screen_info()->height);
	for (uint8_t i = 0; i < n; i++) {
		gm_star_t * star = starsky->stars + i;
		star->x = (esp_random() & starsky->width) << 7;
		star->y = (esp_random() & starsky->height) << 7;
	}
	return gm_node_create(starsky, GM_NODE_STARSKY, z, _free);
}

void gm_starsky_set_xspeed(gm_starsky_t *starsky, int16_t speed)
{
	if (starsky) {
		starsky->xspeed = gm_physics_limit_speed(NULL, speed);
	}
}

void gm_starsky_set_yspeed(gm_starsky_t *starsky, int16_t speed)
{
	if (starsky) {
		starsky->yspeed = gm_physics_limit_speed(NULL, speed);
	}
}

void gm_starsky_visit(gm_starsky_t *starsky, int16_t xoffset, int16_t yoffset)
{
	if (starsky) {
		for (uint8_t i = 0; i < starsky->n; i++) {
			gm_star_t * star = starsky->stars + i;
			star->x += starsky->xspeed;
			star->y += starsky->yspeed;
			int16_t x = (star->x >> 7) - gm_camera()->x;
			int16_t y = (star->y >> 7) - gm_camera()->y;
			if (x < 0) {
				star->x = (starsky->width + gm_camera()->x) << 7;
				star->y = ((esp_random() & starsky->height) + gm_camera()->y) << 7;
			}
			else if (x > starsky->width) {
				star->x = gm_camera()->x << 7;
				star->y = ((esp_random() & starsky->height) + gm_camera()->y) << 7;
			}
			else if (y < 0) {
				star->x = ((esp_random() & starsky->width) + gm_camera()->x) << 7;
				star->y = (starsky->height + gm_camera()->y) << 7;
			}
			else if (y > starsky->height) {
				star->x = ((esp_random() & starsky->width) + gm_camera()->x) << 7;
				star->y = gm_camera()->y << 7;
			}
			gm_lcd_draw_point(x + xoffset, y + yoffset, starsky->color);
		}
	}
}