
#include "pcf8574.h"

// default all outputs
static uint8_t PIN_MODES = 0x0;
static uint8_t DATA = 0x0;
static i2c_port_t i2c_port = I2C_NUM_0;

static i2c_dev_t pcf_dev;
esp_err_t pcf_init(void) {
    pcf_dev.cfg.clk_flags = 0;
    pcf_dev.cfg.sda_pullup_en = 1;
    pcf_dev.cfg.scl_pullup_en = 1;
    return pcf8574_init_desc(&pcf_dev, 0x20, 0, 41, 40);
}

esp_err_t set_pin_mode(uint8_t pin, bool mode) {
    if (pin > 7) {
        return ESP_ERR_INVALID_ARG;
    }
    if (mode) {
        PIN_MODES |= 1 << pin;
    } else {
        PIN_MODES &= ~(1 << pin);
    }
    return pcf8574_port_write(&pcf_dev, DATA);
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
    return pcf8574_port_write(&pcf_dev, DATA);
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
    uint8_t port_val;
    if (pcf8574_port_read(&pcf_dev, &port_val) != ESP_OK) {
        return ESP_FAIL;
    }
    *value = port_val & (1 << pin);
    return ESP_OK;
}
