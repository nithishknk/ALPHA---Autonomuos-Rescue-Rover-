/**
 * @file    oled.h
 * @author  saurash automations
 * @brief   oled header file
 * @version 0.1
 * @date    2024-10-03
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef _OLED_H
#define _OLED_H

#include "stm32f103xb.h"
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

#include "../src/utils/main.h"
#include "../src/i2c/main.h"

#define display_comm_contrast 0x81
#define display_comm_onram    0xA4
#define display_comm_onclr    0xA5
#define display_comm_normal   0xA6
#define display_comm_invert   0xA7
#define display_comm_off      0xAE
#define display_comm_on       0xAF

#define display_config_multiplex 0xA8
#define display_config_mapscan   0xC0
#define display_config_rmapscan  0xC8
#define display_config_offset    0xD3
#define display_config_compins   0xDA

#define display_power_freq   0xD5
#define display_power_period 0xD9
#define display_power_vcomh  0xDB
#define display_power_pump   0x8D
#define display_power_extvcc 0x01
#define display_power_capvcc 0x02

#define display_line_lowcol      0x00
#define display_line_highcol     0x10
#define display_line_startline   0x40
#define display_line_memorymode  0x20
#define display_line_coladdress  0x21
#define display_line_pageaddress 0x22
#define display_line_remap       0xA0

#define display_scroll_enable    0x2F
#define display_scroll_disable   0x2E
#define display_scroll_vertical  0xA3
#define display_scroll_right     0x26
#define display_scroll_left      0x27
#define display_scroll_rvertical 0x29
#define display_scroll_lvertical 0x2A

#define black   0
#define white   1
#define invert  2

typedef struct
{
    i2c_inst_t *i2c;
    uint16_t display_height;
    uint16_t display_width;
    uint16_t buffer_size;
    uint8_t position_x;
    uint8_t position_y;
    uint8_t textsize_x;
    uint8_t textsize_y;
    uint8_t textcolor;
    char *buffer;
}
oled_config_t;

extern oled_config_t oled_config;

extern void oled_initialize(i2c_inst_t *i2c, uint16_t height, uint16_t width);
extern void oled_fill_screen(uint8_t color);
extern void oled_display(void);
extern void oled_write(uint8_t data);
extern void oled_custom_write(const char *data);

extern void oled_print(const char *data);
extern void oled_println(const char *data, size_t length);
extern void oled_decimal(uint32_t data, size_t length, int base);
extern void oled_float(float data, size_t length);
extern void oled_printf(const char *data, ...);

extern void oled_draw_pixel(uint8_t x, uint8_t y);
extern void oled_draw_line(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);
extern void oled_draw_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h);
extern void oled_fill_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h);

static inline void oled_command(uint8_t data)
{
    uint8_t buf[2] = {0x00}; buf[1] = data;
    i2c_write_sequence(oled_config.i2c, 0x78, buf, 2, true);
}

static inline void oled_set_contrast(uint8_t data)
{
    oled_command(display_comm_contrast);
    oled_command(data);
}

static inline void oled_set_invert(bool state)
{
    oled_command(state ? display_comm_invert : display_comm_normal);
}

static inline void oled_set_cursor(uint8_t x, uint8_t y)
{
    if((x > oled_config.display_width) || (y > oled_config.display_height)) return ;
    oled_config.position_x = x; oled_config.position_y = y;
}

static inline void oled_set_tsize(uint8_t x, uint8_t y)
{
    oled_config.textsize_x = (x ? x : 1);
    oled_config.textsize_y = (y ? y : 1);
}

static inline void oled_set_color(uint8_t color)
{
    oled_config.textcolor = color;
}

static inline void oled_draw_hline(uint8_t x, uint8_t y, uint8_t w)
{
    oled_draw_line(x, y, x + w - 1, y);
}

static inline void oled_draw_vline(uint8_t x, uint8_t y, uint8_t h)
{
    oled_draw_line(x, y, x, y + h - 1);
}
#endif 