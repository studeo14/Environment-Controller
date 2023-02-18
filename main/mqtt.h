#ifndef __MQTT_H__
#define __MQTT_H__
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "mqtt_client.h"

void mqtt_app_start(TaskHandle_t screen_task);

esp_mqtt_client_handle_t mqtt_get_client(void);

#endif // __MQTT_H__
