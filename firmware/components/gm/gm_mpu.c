#include "gm_mpu.h"

#define GM_MPU_CHECK(a, err_msg) \
	if(!(a)){ \
		GMB_DBG(err_msg); \
		return 0; \
	}

typedef struct
{
	uint8_t is_init;
	mpud_result2_t result;
	task_handle_t handle;
} gm_mpu_t;

static gm_mpu_t mpu;

static void IRAM_ATTR gm_mpu_isr_cb(void* p)
{
	gm_mpu_t* mpu = (gm_mpu_t*)p;
	task_post_high(mpu->handle, (task_param_t)&mpu->result);
}

uint8_t gm_mpu_setup(gm_mpu_cfg_t* cfg)
{
	if (mpu.is_init && !cfg->reinit) {
		return 1;
	}
	mpu.handle = cfg->handle;
	i2c_config_t conf = {
		.mode = I2C_MODE_MASTER,
		.sda_io_num = cfg->sda,
		.sda_pullup_en = 1,
		.scl_io_num = cfg->sclk,
		.scl_pullup_en = 1,
		.master.clk_speed = cfg->speed
	};
	GM_MPU_CHECK(i2c_param_config(cfg->i2c, &conf) == ESP_OK, "fail to set i2c param\n");
	GM_MPU_CHECK(i2c_driver_install(cfg->i2c, conf.mode, 0, 0, 0) == ESP_OK, "fail to install i2c drive\n");
	GM_MPU_CHECK(mpud_init(cfg->i2c) == 0, "fail to init mpu6050\n");
	gpio_config_t io_conf = {
		.intr_type = GPIO_PIN_INTR_NEGEDGE,
		.mode = GPIO_MODE_INPUT,
		.pull_down_en = 0,
		.pull_up_en = 1
	};
	io_conf.pin_bit_mask = 1ULL << cfg->in;
	GM_MPU_CHECK(gpio_config(&io_conf) == ESP_OK, "fail to config gpio interrupt\n");
	gpio_install_isr_service(0);
	gpio_isr_handler_add(cfg->in, gm_mpu_isr_cb, (void*)&mpu);
	mpud_enable(1);
	mpu.is_init = 1;
	return 1;
}

mpud_result2_t* gm_mpu_read(void)
{
	if (mpu.result.is_valid) {
		return &mpu.result; 
	}
	return NULL;
}

mpud_result2_t* gm_mpu_read_angle(void)
{
	if (mpu.result.is_valid) {
		float q0 = mpu.result.quat[0] / q30;
		float q1 = mpu.result.quat[1] / q30;
		float q2 = mpu.result.quat[2] / q30;
		float q3 = mpu.result.quat[3] / q30;
		mpu.result.pitch = asin(-2 * q1 * q3 + 2 * q0 * q2) * 57.3;	// pitch
		mpu.result.roll = atan2(2 * q2 * q3 + 2 * q0 * q1, -2 * q1 * q1 - 2 * q2 * q2 + 1) * 57.3;	// roll
		mpu.result.yaw = atan2(2 * (q1 * q2 + q0 * q3), q0 * q0 + q1 * q1 - q2 * q2 - q3 * q3) * 57.3;	//yaw
		return &mpu.result;
	}
	return NULL;
}

