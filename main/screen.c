#include "screen.h"

#include "esp_log.h"
#include "freertos/portmacro.h"
#include "freertos/projdefs.h"
#include "freertos/timers.h"
#include "nano_gfx_types.h"
#include "sdkconfig.h"
#include "ssd1306_8bit.h"
#include "touch_element/touch_button.h"
#include "ssd1306.h"
#include <stdint.h>
#include <stdio.h>

static touch_button_handle_t button_handle;
static touch_pad_t button_channel = CONFIG_TOUCH_BUTTON;
static const float channel_sense = 0.1F;

static const char *TAG = "screen";

/* Button event handler task */
static void button_handler_task(void *arg) {
    TaskHandle_t screen_task = (TaskHandle_t) arg; //Unused
    touch_elem_message_t element_message;
    while (1) {
        /* Waiting for touch element messages */
        touch_element_message_receive(&element_message, portMAX_DELAY);
        if (element_message.element_type != TOUCH_ELEM_TYPE_BUTTON) {
            continue;
        }
        /* Decode message */
        const touch_button_message_t *button_message = touch_button_get_message(&element_message);
        if (button_message->event == TOUCH_BUTTON_EVT_ON_PRESS) {
            ESP_LOGI(TAG, "Button[%ld] Press", (uint32_t)element_message.arg);
        } else if (button_message->event == TOUCH_BUTTON_EVT_ON_RELEASE) {
            ESP_LOGI(TAG, "Button[%ld] Release", (uint32_t)element_message.arg);
        } else if (button_message->event == TOUCH_BUTTON_EVT_ON_LONGPRESS) {
            ESP_LOGI(TAG, "Button[%ld] LongPress", (uint32_t)element_message.arg);
            xTaskNotify(screen_task, 0x1, eSetBits);
        }
    }
}

void screen_timer_callback(TimerHandle_t xTimer) {
    // get task to send "turn off" to
    TaskHandle_t task = (TaskHandle_t)pvTimerGetTimerID(xTimer);
    xTaskNotify(task, 0x2, eSetBits);
}

static void screen_task(void *arg) {
    (void) arg;
    bool on = true;
    uint32_t control;
    float temp = 0;
    TimerHandle_t turn_off_timer = xTimerCreate("turn off", pdMS_TO_TICKS(5 * 1000), pdFALSE, xTaskGetCurrentTaskHandle(), screen_timer_callback);
    xTimerStart(turn_off_timer, 0);
    ssd1306_printFixed16(0, 0, "Temp:", STYLE_NORMAL);
    ssd1306_printFixed16(0, 16, " 0.00 C", STYLE_NORMAL);
    char temp_string[20];
    while (1) {
        if (on) {
            // if temp update
            uint32_t tmp;
            if (xTaskNotifyWaitIndexed(1, 0x0, 0xFFFFFFFF, &tmp, 50) == pdTRUE) {
                temp = *((float*)&tmp);
                ESP_LOGI(TAG, "Got new temp: %f, %5.2f", temp, temp);
                snprintf(temp_string, 8, "%5.2f C", temp);
                ssd1306_printFixed16(0, 16, temp_string, STYLE_NORMAL);
            } else {
                ESP_LOGI(TAG, "Display old temp: %f, %5.2f", temp, temp);
                snprintf(temp_string, 8, "%5.2f C", temp);
                ssd1306_printFixed16(0, 16, temp_string, STYLE_NORMAL);
            }
            if (xTaskNotifyWait(0x0, 0x3, &control, 0) == pdTRUE) {
                // if another button press
                if (control & 0x1) {
                    xTimerReset(turn_off_timer, 0);
                }
                // if timer expires
                if (control & 0x2) {
                    on = false;
                    ssd1306_clearScreen16();
                }
            }
        } else {
            // wait for button press
            if (xTaskNotifyWait(0x0, 0x1, &control, pdMS_TO_TICKS(1000)) == pdTRUE) {
                if (control & 0x1) {
                    on = true;
                    ssd1306_printFixed16(0, 0, "Temp:", STYLE_NORMAL);
                    xTimerReset(turn_off_timer, 0);
                }
            }
        }
    }
}

TaskHandle_t screen_init(int8_t rst, int8_t cs, int8_t dc) {
    ssd1351_128x128_spi_init(rst, cs, dc);
    ssd1306_setFixedFont(ssd1306xled_font6x8);
    ssd1306_fillScreen(0x00);
    ssd1306_setMode(LCD_MODE_NORMAL);
    ssd1306_clearScreen16();
    ssd1306_setColor(RGB_COLOR16(255, 255, 0));
    touch_elem_global_config_t global_config = TOUCH_ELEM_GLOBAL_DEFAULT_CONFIG();
    ESP_ERROR_CHECK(touch_element_install(&global_config));
    ESP_LOGI(TAG, "Touch element lib installed");
    touch_button_global_config_t button_global_config = TOUCH_BUTTON_GLOBAL_DEFAULT_CONFIG();
    ESP_ERROR_CHECK(touch_button_install(&button_global_config));
    ESP_LOGI(TAG, "Touch button installed");

    touch_button_config_t button_config = {
        .channel_num = button_channel,
        .channel_sens = channel_sense
    };

    /* Create Touch buttons */
    ESP_ERROR_CHECK(touch_button_create(&button_config, &button_handle));
    /* Subscribe touch button events (On Press, On Release, On LongPress) */
    ESP_ERROR_CHECK(touch_button_subscribe_event(button_handle, TOUCH_ELEM_EVENT_ON_PRESS | TOUCH_ELEM_EVENT_ON_RELEASE | TOUCH_ELEM_EVENT_ON_LONGPRESS,
                                                    (void *)button_channel));
    /* Set EVENT as the dispatch method */
    ESP_ERROR_CHECK(touch_button_set_dispatch_method(button_handle, TOUCH_ELEM_DISP_EVENT));
    /* Set LongPress event trigger threshold time */
    ESP_ERROR_CHECK(touch_button_set_longpress(button_handle, 2000));
    /* Create a handler task to handle event messages */
    TaskHandle_t retVal;
    xTaskCreate(screen_task, "screen_task", 4 * 1024, NULL, 5, &retVal);
    xTaskCreate(button_handler_task, "button_handler_task", 4 * 1024, retVal, 5, NULL);
    touch_element_start();
    ESP_LOGI(TAG, "Touch buttons created");
    return retVal;
}