#ifndef _GM_BOARD_H
#define _GM_BOARD_H

#include <math.h>
#include <string.h>

#include "driver/gpio.h"
#include "driver/adc.h"
#include "driver/spi_master.h"
#include "driver/i2c.h"

#include "rom/ets_sys.h"
#include "esp_log.h"

static const char* GMB __attribute__((used)) = "gm_board";
#define GMB_DBG(fmt, ...) \
        ESP_LOGE(GMB, fmt, ##__VA_ARGS__)

#endif