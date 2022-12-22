
#ifndef __SHT31_H
#define __SHT31_H

#include "esp_log.h"
#include "driver/i2c.h"

#define WRITE_BIT I2C_MASTER_WRITE            
#define READ_BIT I2C_MASTER_READ
#define ACK_CHECK_EN 0x1
#define ACK_CHECK_DIS 0x0
#define ACK_VAL 0x0
#define NACK_VAL 0x1

void sht31_set_port(i2c_port_t port);

esp_err_t sht31_read_temp_humi(float *temp, float *humi);

#endif // __SHT31_H