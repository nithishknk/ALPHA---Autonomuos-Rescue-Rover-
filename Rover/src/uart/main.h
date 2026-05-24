/**
 * @file    uart header file.h
 * @author  saurash automations
 * @brief   stm32 uart implementation
 * @version 0.1
 * @date    2024-12-10
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef _UART_H
#define _UART_H

#include <stdbool.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "../src/utils/main.h"
#include "../src/gpio/main.h"

typedef USART_TypeDef uart_inst_t;
#define uart1 (USART1)
#define uart2 (USART2)
#define uart3 (USART3)
#define uartindex(x) ((x == USART1) ? 1 : (x == USART2) ? 2 : 3)

extern uint8_t serialarray1[150], serialcount1;
extern uint8_t serialarray2[150], serialcount2;
extern uint8_t serialarray3[150], serialcount3;

extern void serial_initialize(uart_inst_t *uart, gpio_pinmapping_t txr, gpio_pinmapping_t rxr, uint32_t baudrate);
extern void serial_enable(uart_inst_t *uart, void *fun, bool state);
extern uint8_t serial_receive(uart_inst_t *uart);

extern void serial_send(uart_inst_t *uart, uint8_t data);
extern void serial_write(uart_inst_t *uart, const char *data);
extern void serial_writeln(uart_inst_t *uart, const char *data, size_t length);

extern void serial_printf(uart_inst_t *uart, const char *fmt, ...);
extern void serial_decimal(uart_inst_t *uart, uint32_t data, uint8_t base);
extern void serial_float(uart_inst_t *uart, float data, size_t length);

extern void serial_flush(uart_inst_t *uart);
extern bool serial_talkback(uart_inst_t *uart, bool nl, const char *data, size_t length, uint16_t timer);

#endif 