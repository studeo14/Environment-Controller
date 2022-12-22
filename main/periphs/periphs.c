
#include "periphs.h"
#include "sht31.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "pcf8574.h"

#include "mqtt_client.h"
#include <string.h>

static const char *TAG = "PERIPHS";

void periphs_monitor_task(void * pvParameters) {
    float temp, humidity;
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
            if (temp >= 34.0) {
                pin_state = false;
            } else {
                pin_state = true;
            }
            res = set_pin_value(0, pin_state);
            if (res != ESP_OK) {
                ESP_LOGE(TAG, "Error setting pin state %s", esp_err_to_name(res));
            }
        }
        vTaskDelay(60000 / portTICK_PERIOD_MS);
    }
}