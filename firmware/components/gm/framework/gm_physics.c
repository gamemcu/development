#include "gm_physics.h"

int16_t gm_physics_limit_speed(gm_physics_dir_t *dir, int16_t speed)
{
	int16_t maxspeed = GM_MAX_SPRITE_SPEED;
	if (dir) {
		maxspeed = dir->maxspeed;
	}
	if (speed > maxspeed) {
		return maxspeed;
	}
	if (speed < -maxspeed) {
		return -maxspeed;
	}
	return speed;
}

void gm_physics_change_pos(gm_physics_dir_t *dir)
{
	int16_t frict;
	if (dir->speed == 0) {
		frict = 0;
	}
	else {
		frict = dir->speed > 0 ? -dir->frict : dir->frict;
	}
	dir->speed += dir->acc + frict;
	uint8_t same_dir = ((frict > 0) && (dir->speed > 0)) ||
	                   ((frict < 0) && (dir->speed < 0));
	if (same_dir) {
		dir->speed = 0;
	}
	dir->speed = gm_physics_limit_speed(dir, dir->speed);
	dir->pos += dir->speed;
	if (dir->pos >= dir->maxpos) {
		dir->pos = dir->maxpos;
	}
	if (dir->pos <= dir->minpos) {
		dir->pos = dir->minpos;
	}

}


void gm_physics_collide(void)
{

}

void _emiter_free(gm_node_t *node)
{

}

gm_node_t* gm_emiter_create(bitmap2_t* bmp, uint8_t n, int16_t x, int16_t y, int16_t z)
{
	gm_emiter_t *emiter = (gm_emiter_t *)malloc(sizeof(gm_emiter_t));
	if (emiter == NULL) {
		return NULL;
	}
	memset(emiter, 0, sizeof(gm_emiter_t));

	return gm_node_create(emiter, GM_NODE_EMITER, z, _emiter_free);
}

void gm_emiter_visit(gm_emiter_t* emiter, int16_t xoffset, int16_t yoffset)
{
	if (emiter) {
		for (gm_node_t* node = emiter->particle_mng.start; node; node = node->child) {
			gm_particle_t *particle = (gm_particle_t *)node->p;
			particle->x += emiter->xspeed;
			particle->y += emiter->yspeed;
			gm_lcd_draw_bmp2(&emiter->bmp, particle->x, particle->y, 0);
			if (particle->life) {
				particle->life--;
				if (!particle->life) {
					gm_node_remove(&emiter->particle_mng, node);
				}
			}
		}
	}
}
