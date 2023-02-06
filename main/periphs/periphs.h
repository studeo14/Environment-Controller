
#ifndef __PERIPHS_H
#define __PERIPHS_H

#include "esp_event.h"
#include <stdint.h>

void periphs_monitor_task(void * pvParameters);
void periphs_monitor_task_no_mqtt(void * pvParameters);

void periphs_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);

#endif // __PERIPHS_H