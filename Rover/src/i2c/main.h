/**
 * @file    i2c driver.h
 * @author  saurash automations
 * @brief   i2c master for stm32fxx
 * @version 0.1
 * @date    2024-12-04
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef _I2C_H
#define _I2C_H

#include "../src/utils/main.h"
#include "../src/gpio/main.h"
#include <stdbool.h>

typedef I2C_TypeDef i2c_inst_t;
#define i2c1 (I2C1)
#define i2c2 (I2C2)

extern void i2c_initialize(i2c_inst_t *i2c, gpio_pinmapping_t scl, gpio_pinmapping_t sda, uint16_t frequency);
extern void i2c_start(i2c_inst_t *i2c);
extern void i2c_stop(i2c_inst_t *i2c);
extern void i2c_write_sequence(i2c_inst_t *i2c, uint8_t sad, uint8_t *buf, size_t length, bool stop);
extern void i2c_read_sequence(i2c_inst_t *i2c, uint8_t sad, uint8_t *buf, size_t length);
#endif 