/**
 * @file    alarm.h
 * @author  saurash automations
 * @brief   timer function based alarm 
 * @version 0.1
 * @date    2024-12-11
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#ifndef _ALARM_H
#define _ALARM_H

#include "../src/utils/main.h"
#include "../src/gpio/main.h"

#define getalarm()\
(!(RCC->APB2ENR & (1 << 11)) ? 1 :\
(!(RCC->APB1ENR & (1 << 0))  ? 2 :\
(!(RCC->APB1ENR & (1 << 1))  ? 3 :\
0)))

#define getregister(x) (x == 1 ? TIM1 : x == 2 ? TIM2 : TIM3)
#define us * 0.000001F
#define ms * 0.001F

uint8_t add_repeating_alarm_us(uint32_t firing);
uint8_t add_repeating_alarm_ms(uint32_t firing);
void start_repeating_alarm(uint8_t alarmid, void *fun);
void reset_repeating_alarm(uint8_t alarmid);
void stop_repeating_alarm(uint8_t alarmid);
void remove_repeating_alarm(uint8_t alarmid);
#endif 