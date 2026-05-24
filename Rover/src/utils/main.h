/**
 * @file    stm32xx utils function
 * @author  saurash automations
 * @brief   utils functions
 * @version 0.1
 * @date    2024-12-02
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef _UTILS_H
#define _UTILS_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "stm32f103xb.h"

#define SYSCLOCK    72000000UL
#define APB1CLOCK   (SYSCLOCK / 2)
#define APB2CLOCK   (SYSCLOCK)

#define AHBCLOCK    (SYSCLOCK)
#define PCLK1       (APB1CLOCK) 
#define PCLK2       (APB2CLOCK)

#define SYSLOAD     (SYSCLOCK / 1000000UL)
#define SYSCALIB    (SYSLOAD >> 1)

#define BIN 2
#define OCT 8
#define DEC 10
#define HEX 16

//for systick function definitions
extern void stdio_init_all(void);
extern uint32_t millis(void);
extern uint64_t micros(void);

/**
 * @brief generate microseconds delay
 * 
 * @param delay uint16_t require delay 
 * @note not exceed more than 1000us 
 */
static inline void delay_us(uint16_t delay)
{
    uint32_t start = SysTick->VAL; 
    uint32_t end = (delay * SYSLOAD) - SYSCALIB;
    while((start - SysTick->VAL) < end); 
}

/**
 * @brief generate milliseconds delay
 * 
 * @param delay uint32_t require delay
 */
static inline void delay_ms(uint32_t delay)
{
    do delay_us(1000); while(--delay > 0);
}

extern long mapdecimal(long mv, long smin, long smax, long emin, long emax);
extern float mapfloat(float mv, float smin, float smax, float emin, float emax);
extern void *memmem(const void *ms, size_t msl, const void *ss, size_t ssl);
extern char *split(char *ms, char *buffer, const char *hs, uint8_t ne);
extern char *dtoa(long data, char *buffer, uint8_t base, size_t count);
extern char *ftostra(double data, char *buffer, size_t count);
#endif 