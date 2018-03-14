#ifndef _GM_MPU_H
#define _GM_MPU_H

#include "gm_board.h"
#include "task/task.h"
#include "mpu.h"

typedef struct
{
	uint8_t reinit;
	int i2c, in, sclk, sda, speed;
	task_handle_t handle;
} gm_mpu_cfg_t;

uint8_t gm_mpu_setup(gm_mpu_cfg_t* cfg);
mpud_result2_t* gm_mpu_read(void);
mpud_result2_t* gm_mpu_read_angle(void);

#endif