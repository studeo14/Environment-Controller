
#include "pcf8574.h"

// default all outputs
static uint8_t PIN_MODES = 0x0;
static uint8_t DATA = 0x0;
static i2c_port_t i2c_port = I2C_NUM_0;

esp_err_t set_pin_mode(uint8_t pin, bool mode) {
    if (pin > 7) {
        return ESP_ERR_INVALID_ARG;
    }
    if (mode) {
        PIN_MODES |= 1 << pin;
    } else {
        PIN_MODES &= ~(1 << pin);
    }
    uint8_t data[] = {PIN_MODES};
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (ADDR << 1) | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write(cmd, data, sizeof(data), ACK_CHECK_EN);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(i2c_port, cmd, 1000 / portTICK_PERIOD_MS);
    if (ret != ESP_OK) {
        return ret;
    }
    i2c_cmd_link_delete(cmd);
    return ESP_OK;
}

esp_err_t set_pin_value(uint8_t pin, bool value) {
    if (pin > 7) {
        return ESP_ERR_INVALID_ARG;
    }
    // if input
    if (PIN_MODES & (1 << pin)) {
        return ESP_ERR_INVALID_ARG;
    }
    // else
    if (value) { // set to light pull up
        DATA |= (1 << pin);
    } else { // set to ground
        DATA &= ~(1 << pin);
    }
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (ADDR << 1) | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, DATA, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(i2c_port, cmd, 1000 / portTICK_PERIOD_MS);
    if (ret != ESP_OK) {
        return ret;
    }
    i2c_cmd_link_delete(cmd);
    return ESP_OK;
}

esp_err_t read_pin_value(uint8_t pin, bool *value) {
    if (pin > 7) {
        return ESP_ERR_INVALID_ARG;
    }
    // if output
    if (!(PIN_MODES & (1 << pin))) {
        return ESP_ERR_INVALID_ARG;
    }
    // else
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (ADDR << 1) | READ_BIT, ACK_CHECK_EN);
    uint8_t values;
    i2c_master_read_byte(cmd, &values, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(i2c_port, cmd, 1000 / portTICK_PERIOD_MS);
    if (ret != ESP_OK) {
        return ret;
    }
    i2c_cmd_link_delete(cmd);
    *value = values & (1 << pin);
    return ESP_OK;
}
