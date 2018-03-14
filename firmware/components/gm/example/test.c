#include "stdio.h"
#include "stdint.h"

typedef struct
{
	uint8_t wcells, hcells;
	uint8_t screen_wcells, screen_hcells;
	uint8_t data[10];
} gm_titlemap_t;

gm_titlemap_t titlemap = {
	/*.wcells = */3,
	/*.hcells = */8,
	/*.screen_wcells = */8,
	/*.screen_hcells = */4,
	/*.data = */{0xfe, 0xef, 0xff}
};

uint8_t gm_titlemap_is_soild(gm_titlemap_t *titlemap, int16_t x, int16_t y)
{
	if ((x < 0) || (x > titlemap->wcells - 1)) {
		return 1;
	}
	if ((y < 0) || (y > titlemap->hcells - 1)) {
		return 1;
	}
	uint16_t i = x + (y >> 3) * (titlemap->wcells);
	uint8_t b = titlemap->data[i];
	b = b >> (y & 7);
	printf("%x\n", b);
	return b & 1;
}

const uint8_t str[100] = 
	"hello"
	"world";

int main(int argc, char const *argv[])
{
	uint64_t apb = 80 * 1000000;
	uint64_t ref = 1 * 1000000;
	uint8_t a = 12;
	printf("%d\n", gm_titlemap_is_soild(&titlemap, 1, 4));
	printf("%d, %d\n", 101 % 10, 201 % 10);
	printf("%lu\n", (apb << 8)/660/(1<<13));
	printf("%ld\n", (ref << 8)/660/(1<<13));
	printf("%d\n", '~'-' ');
	return 0;
}