/**
 * @file    extint.h
 * @author  saurash automations
 * @brief   external interrupt enabled header
 * @version 0.1
 * @date    2024-12-21
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef _EXT_INT_H
#define _EXT_INT_H

#include "../src/utils/main.h"
#include "../src/gpio/main.h"

#define GPIO_ISR_RISING     (1 << 0)
#define GPIO_ISR_FALLING    (1 << 1)
#define GPIO_ISR_CHANGE     (GPIO_ISR_RISING | GPIO_ISR_FALLING)

void gpio_isr_initialize(gpio_pinmapping_t pin, uint8_t state, uint8_t edge);
void gpio_isr_enable(gpio_pinmapping_t pin, void *fun);
void gpio_isr_disable(gpio_pinmapping_t pin);

#endif 