/**
 * @file    servo.h
 * @author  saurash automations
 * @brief   servo firmware for stm32
 * @version 0.1
 * @date    2025-04-08
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#ifndef _SERVO_H
#define _SERVO_H

#include "../src/utils/main.h"
#include "../src/gpio/main.h"
#include "../src/alarm/main.h"

#define SERVO_MIN_PULSE 544
#define SERVO_MAX_PULSE 2400

#define SERVO_MIN_ANGLE 0
#define SERVO_MAX_ANGLE 180

typedef struct
{
    gpio_pinmapping_t pin;
    uint8_t position;
    bool isactive;
    uint16_t pulseperiod;
    uint64_t activatedtime;
}
servo_pin_config_t;

extern servo_pin_config_t servo[MAX_NO_SERVO];
extern void servo_attach(servo_pin_config_t *servo, gpio_pinmapping_t pin);
extern void servo_initialize(void);
extern void servo_write(servo_pin_config_t *servo, uint8_t pos);
extern void servo_dettach(servo_pin_config_t *servo);
#endif 