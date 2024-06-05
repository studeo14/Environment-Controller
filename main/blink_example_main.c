/* Blink Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "hal/gpio_types.h"
#include "nvs_flash.h"

#include "driver/i2c.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "screen.h"
#include "sdkconfig.h"

#include <stdio.h>

#include "mqtt_client.h"
#include "esp_tls.h"
#include "esp_ota_ops.h"
#include <sys/param.h>

#include "utils.h"
#include "wifi/wifi.h"
#include "led/led.h"
#include "periphs/periphs.h"
#include "mqtt.h"
#include "i2cdev.h"

static const char *TAG = "blinky";

#define I2C_MASTER_SCL_IO                                                      \
  CONFIG_I2C_MASTER_SCL /*!< GPIO number used for I2C master clock */
#define I2C_MASTER_SDA_IO                                                      \
  CONFIG_I2C_MASTER_SDA /*!< GPIO number used for I2C master data  */
#define I2C_MASTER_NUM                                                         \
  0 /*!< I2C master i2c port number, the number of i2c peripheral interfaces   \
       available will depend on the chip */
#define I2C_MASTER_FREQ_HZ 10000   /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE 0 /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE 0 /*!< I2C master doesn't need buffer */
#define I2C_MASTER_TIMEOUT_MS 1000




/**
 * @brief i2c master initialization
 */
static esp_err_t i2c_master_init(void) {
  int i2c_master_port = I2C_MASTER_NUM;

  i2c_config_t conf = {
      .mode = I2C_MODE_MASTER,
      .sda_io_num = I2C_MASTER_SDA_IO,
      .sda_pullup_en = GPIO_PULLUP_ENABLE,
      .scl_io_num = I2C_MASTER_SCL_IO,
      .scl_pullup_en = GPIO_PULLUP_ENABLE,
      .master.clk_speed = I2C_MASTER_FREQ_HZ,
      .clk_flags = 0,
  };

  i2c_param_config(i2c_master_port, &conf);

  return i2c_driver_install(i2c_master_port, conf.mode,
                            I2C_MASTER_RX_BUF_DISABLE,
                            I2C_MASTER_TX_BUF_DISABLE, 0);
}

void app_main(void) {
  esp_err_t ret;
  ret = i2cdev_init();
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Unable to init i2cdev");
  }
  /* Configure the peripheral according to the LED type */
  configure_led();
  // ESP_ERROR_CHECK(i2c_master_init());

  // // do i2c scan
  // ESP_LOGI(TAG, "i2c scan:");
  // for (uint8_t i = 1; i < 127; i++) {
  //   i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  //   i2c_master_start(cmd);
  //   i2c_master_write_byte(cmd, (i << 1) | I2C_MASTER_WRITE, 1);
  //   i2c_master_stop(cmd);
  //   ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_PERIOD_MS);
  //   i2c_cmd_link_delete(cmd);

  //   if (ret == ESP_OK) {
  //     ESP_LOGI(TAG, "Found device at: 0x%2x", i);
  //   }
  // }
  // i2c_driver_delete(0);

  //Initialize NVS
  ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);
  ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
  wifi_init_sta();

  // spawn rainbow task
  ret = xTaskCreate(rainbow_task, "Rainbow", 2048, NULL, 5, NULL);
  if (ret != pdPASS) {
    ESP_LOGE(TAG, "Unable to initialize rainbow task: %d", ret);
  }


  // spawn screen task
  TaskHandle_t screen_task = screen_init(7, 5, 6);
  // spawn mqtt (once connected with spawn perih task)
  mqtt_app_start(screen_task);

}
