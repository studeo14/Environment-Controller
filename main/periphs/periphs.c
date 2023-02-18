
#include "periphs.h"
#include "periphs/periphs.h"
#include "sht31.h"
#include "mcp960x.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "pcf.h"

#include "mqtt_client.h"
#include <string.h>

static const char *TAG = "PERIPHS";

static QueueHandle_t tempSetQueue = NULL;

void periphs_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    esp_mqtt_event_handle_t event = event_data;
    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        float number = atof(event->data);
        ESP_LOGI(TAG, "%f", number);
        if (number > 0.0) {
            if(xQueueSend(tempSetQueue, &number, 0) != pdPASS) {
                ESP_LOGE(TAG, "Unable to send to queue");
            }
        }
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}

void periphs_monitor_task(void * pvParameters) {
    PerifStructHandle_t params = (PerifStructHandle_t)pvParameters;
    // setup queue
    tempSetQueue = xQueueCreate(5, sizeof(float));
    if (tempSetQueue == NULL) {
        ESP_LOGE(TAG, "Unable to create queue");
    }
    float temp;
    float threshold = 27.0;
    esp_err_t res;
    res = pcf_init();
    if (res != ESP_OK) {
        ESP_LOGE(TAG, "Error init pcf %s", esp_err_to_name(res));
    }
    res = set_pin_mode(0, false);
    if (res != ESP_OK) {
        ESP_LOGE(TAG, "Error setting pin mode %s", esp_err_to_name(res));
    }
    bool pin_state = true;
    mcp960x_t temp_dev;
    res = mcp960x_init_desc(&temp_dev, MCP960X_ADDR_DEFAULT, 0, 41, 40);
    if (res != ESP_OK) {
        ESP_LOGE(TAG, "Error desc_init %s", esp_err_to_name(res));
    }
    ESP_LOGI(TAG, "i2c_dev ptr: %p", temp_dev.i2c_dev);
    res = mcp960x_init(&temp_dev);
    if (res != ESP_OK) {
        ESP_LOGE(TAG, "Error init %s, %d, %p", esp_err_to_name(res), res, res);
    }
    while(1) {
        res = mcp960x_get_thermocouple_temp(&temp_dev, &temp);
        if (res != ESP_OK) {
            ESP_LOGE(TAG, "Unable to read MCP: %d", res);
        } else {
            ESP_LOGI(TAG, "Temp (here): %f", temp);
            xTaskNotifyIndexed(params->screen_task, 1, *((uint32_t*)&temp), eSetValueWithOverwrite);
            if (params->mqtt_client != NULL) {
                char payload[50] = {0};
                sprintf(payload, "%f", temp);
                int res = esp_mqtt_client_publish(params->mqtt_client, "/topic/monitor_data", payload, 0, 0, 0);
                ESP_LOGI(TAG, "res: %d, %s", res, payload);
            } else {
                ESP_LOGE(TAG, "Mqtt client null");
            }
            if (temp >= threshold) {
                pin_state = false;
            } else {
                pin_state = true;
            }
            res = set_pin_value(0, pin_state);
            if (res != ESP_OK) {
                ESP_LOGE(TAG, "Error setting pin state %s", esp_err_to_name(res));
            }
        }
        if (xQueueReceive(tempSetQueue, &threshold, 30000 / portTICK_PERIOD_MS) == pdPASS) {
            ESP_LOGI(TAG, "Set new threshold value of: %f", threshold);
        }
    }
}

void periphs_monitor_task_no_mqtt(void * pvParameters) {
    TaskHandle_t screen_task = (TaskHandle_t) pvParameters;
    // setup queue
    tempSetQueue = xQueueCreate(5, sizeof(float));
    if (tempSetQueue == NULL) {
        ESP_LOGE(TAG, "Unable to create queue");
    }
    float temp;
    float threshold = 27.0;
    esp_err_t res;
    res = pcf_init();
    if (res != ESP_OK) {
        ESP_LOGE(TAG, "Error init pcf %s", esp_err_to_name(res));
    }
    res = set_pin_mode(0, false);
    if (res != ESP_OK) {
        ESP_LOGE(TAG, "Error setting pin mode %s", esp_err_to_name(res));
    }
    bool pin_state = true;
    mcp960x_t temp_dev;
    res = mcp960x_init_desc(&temp_dev, MCP960X_ADDR_DEFAULT, 0, 41, 40);
    if (res != ESP_OK) {
        ESP_LOGE(TAG, "Error desc_init %s", esp_err_to_name(res));
    }
    ESP_LOGI(TAG, "i2c_dev ptr: %p", temp_dev.i2c_dev);
    res = mcp960x_init(&temp_dev);
    if (res != ESP_OK) {
        ESP_LOGE(TAG, "Error init %s, %d, %p", esp_err_to_name(res), res, res);
    }
    while(1) {
        res = mcp960x_get_thermocouple_temp(&temp_dev, &temp);
        if (res != ESP_OK) {
            ESP_LOGE(TAG, "Unable to read MCP: %d", res);
        } else {
            ESP_LOGI(TAG, "Temp: %f", temp);
            xTaskNotifyIndexed(screen_task, 1, *((uint32_t*)&temp), eSetValueWithOverwrite);
            if (temp >= threshold) {
                pin_state = false;
            } else {
                pin_state = true;
            }
            res = set_pin_value(0, pin_state);
            if (res != ESP_OK) {
                ESP_LOGE(TAG, "Error setting pin state %s", esp_err_to_name(res));
            }
        }
        if (xQueueReceive(tempSetQueue, &threshold, 30000 / portTICK_PERIOD_MS) == pdPASS) {
            ESP_LOGI(TAG, "Set new threshold value of: %f", threshold);
        }
    }
}