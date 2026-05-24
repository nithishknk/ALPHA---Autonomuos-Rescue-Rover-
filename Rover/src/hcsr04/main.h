/**
 * @file   hcsr04.h
 * @author saurash automations
 * @brief  radar firmware for stm32
 * @version 0.1
 * @date    2025-03-30
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#ifndef _HCSR04_H
#define _HCSR04_H

#include "../src/utils/main.h"
#include "../src/gpio/main.h"
#include "../src/extint/main.h"

void radar_initialize(gpio_pinmapping_t trigger, gpio_pinmapping_t echo);
uint16_t radar_fetch(void);
#endif 