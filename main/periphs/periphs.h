
#ifndef __PERIPHS_H
#define __PERIPHS_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "mqtt_client.h"
#include "esp_event.h"
#include <stdint.h>

typedef struct {
    TaskHandle_t screen_task;
    esp_mqtt_client_handle_t mqtt_client;
} PerifStruct;

typedef PerifStruct* PerifStructHandle_t;

void periphs_monitor_task(void * pvParameters);
void periphs_monitor_task_no_mqtt(void * pvParameters);

void periphs_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);

#endif // __PERIPHS_H