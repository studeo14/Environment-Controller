#ifndef __SCREEN_H__
#define __SCREEN_H__

#include "esp_err.h"
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

TaskHandle_t screen_init(int8_t rst, int8_t cs, int8_t dc);

#endif // __SCREEN_H__
