
#ifndef __PCF8574_H
#define __PCF8574_H

#include "esp_log.h"
#include "driver/i2c.h"
#include <stdint.h>

#define WRITE_BIT I2C_MASTER_WRITE            
#define READ_BIT I2C_MASTER_READ
#define ADDR 0x20
#define ACK_CHECK_EN 0x1
#define ACK_CHECK_DIS 0x0
#define ACK_VAL 0x0
#define NACK_VAL 0x1

/**
 * Set pin to WB/R (false is output, true is input);
 */
esp_err_t set_pin_mode(uint8_t pin, bool mode);

/**
 * Set pin to value where true = "light pull-up input"
 *                        false = "strong ground output"
 */
esp_err_t set_pin_value(uint8_t pin, bool value);
esp_err_t read_pin_value(uint8_t pin, bool *value);

#endif // __PCF8574_H