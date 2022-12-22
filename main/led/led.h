
#ifndef __LED_H
#define __LED_H
#include "led_strip.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdint.h>
#include <stdint.h>
#include <math.h>

/* Use project configuration menu (idf.py menuconfig) to choose the GPIO to
   blink, or you can edit the following line and set a number here.
*/
#define BLINK_GPIO CONFIG_BLINK_GPIO
#define BLINK_POWER CONFIG_BLINK_POWER

void set_pixel(uint32_t red, uint32_t green, uint32_t blue);

void blink_led(void);

void configure_led(void);

void rainbow_task(void * pvParameters);

typedef struct {
    double r;       // ∈ [0, 1]
    double g;       // ∈ [0, 1]
    double b;       // ∈ [0, 1]
} rgb;

typedef struct {
    double h;       // ∈ [0, 360]
    double s;       // ∈ [0, 1]
    double v;       // ∈ [0, 1]
} hsv;

rgb hsv2rgb(hsv HSV);

#endif // __LED_H