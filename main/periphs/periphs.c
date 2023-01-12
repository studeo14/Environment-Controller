
#include "periphs.h"
#include "sht31.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "pcf8574.h"

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
    // setup queue
    tempSetQueue = xQueueCreate(5, sizeof(float));
    if (tempSetQueue == NULL) {
        ESP_LOGE(TAG, "Unable to create queue");
    }
    float temp, humidity;
    float threshold = 32.0;
    esp_err_t res;
    res = set_pin_mode(0, false);
    if (res != ESP_OK) {
        ESP_LOGE(TAG, "Error setting pin mode %s", esp_err_to_name(res));
    }
    bool pin_state = true;
    esp_mqtt_client_handle_t client = (esp_mqtt_client_handle_t)pvParameters;
    while(1) {
        res = sht31_read_temp_humi(&temp, &humidity);
        if (res != ESP_OK) {
            ESP_LOGE(TAG, "Unable to read SHT31: %d", res);
        } else {
            ESP_LOGI(TAG, "Temp: %f, Humidity: %f", temp, humidity);
            if (client != NULL) {
                char payload[50] = {0};
                sprintf(payload, "%f|%f", temp, humidity);
                esp_mqtt_client_publish(client, "/topic/monitor_data", payload, 0, 1, 0);
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
        if (xQueueReceive(tempSetQueue, &threshold, 60000 / portTICK_PERIOD_MS) == pdPASS) {
            ESP_LOGI(TAG, "Set new threshold value of: %f", threshold);
        }
    }
}