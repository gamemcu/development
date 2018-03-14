#ifndef _GM_DIRCTOR_H
#define _GM_DIRCTOR_H

#include "gm_node.h"

void gm_director_show_fps(uint8_t en);
uint8_t gm_director_set_fps(uint16_t fps);
uint16_t gm_director_get_fps(void);
uint8_t gm_director_run_scene(gm_node_t *node);
uint8_t gm_director_replace_scene(gm_node_t *node);
void gm_director_remove_all_scene(void);
uint8_t gm_director_pre_scene(void);
uint8_t gm_director_next_scene(void);
void gm_director_main_loop(void);

#endif