#include <avr/io.h>
#include <Arduino.h>
#include <string.h>

#include "gpio.h"
#include "utils.h"
#include "i2c.h"
#include "oled.h"

unsigned char requirecolor = white;
unsigned char textsizex = 1, textsizey = 1;
unsigned char positionx = 0, positiony = 0;
unsigned char buffer[(display_height * display_width) / 8];

/*!
* @brief    initialize oled module 
* @param    - no parameters 
* @return   return true if address match
* @note     oledpinconfig must be defined
*/
char oled_initialize(unsigned long freq)
{
    i2c_initialize(freq);
    
    oled_command(display_comm_off);
    oled_command(display_power_freq);
    oled_command(0x80);
    oled_command(display_config_multiplex);
    oled_command(display_height - 1);
    oled_command(display_config_offset);
    oled_command(0x00);
    oled_command(display_line_startline);
    oled_command(display_power_pump);
    oled_command(0x14);
    oled_command(display_line_memorymode);
    oled_command(0x00);
    oled_command(display_line_remap | 1);
    oled_command(display_config_rmapscan);
    oled_command(display_config_compins);
    oled_command(0x02);
    oled_command(display_comm_contrast);
    oled_command(0x8F);
    oled_command(display_power_period);
    oled_command(0xF1);
    oled_command(display_power_vcomh);
    oled_command(0x40);
    oled_command(display_comm_onram);
    oled_command(display_comm_normal);
    oled_command(display_scroll_disable);
    oled_command(display_comm_on);
    
    positionx = 0;
    positiony = 0;
    oled_screen_clear();
    oled_display();

    return true;
}

/*!
* @brief    send command to oled module
* @param    command require command to send
* @return   function not a return type
*/
void oled_command(unsigned char command)
{
    i2c_register_write(0x78, 0x00, command);
}

/*!
* @brief    fill the screen
* @param    - no parameters
* @return   function not a return type
*/
void oled_screen_fill(void)
{
    memset(buffer, 0xFF, sizeof(buffer));
    oled_display();
}

/*!
* @brief    clear the screen
* @param    - no parameters
* @return   function not a return type
*/
void oled_screen_clear(void)
{
    memset(buffer, 0x00, sizeof(buffer));
    oled_display(); positionx = positiony = 0;
}

/*!
* @brief    adjust screen contrast
* @param    data require contrast
* @return   function not a return type
*/
void oled_screen_contrast(unsigned char data)
{
    oled_command(display_comm_contrast);
    oled_command(data);
}

/*!
* @brief    display buffer data
* @param    - no paramters
* @return   function not a return type
*/
void oled_display(void)
{
    oled_command(display_line_coladdress);
    oled_command(0);
    oled_command(display_width - 1);
    oled_command(display_line_pageaddress);
    oled_command(0);
    oled_command(3);
    i2c_sequence_write(0x78, display_line_startline, buffer, sizeof(buffer));
}

/*!
* @brief    invert display
* @param    state true to invert
* @return   function not a return type
*/
void oled_display_invert(unsigned char state)
{
    oled_command(state ?display_comm_invert :display_comm_normal);
}

/*!
* @brief    set oled cursor
* @param    x width of display (0 - 128)
* @param    y height of display (0 - 32,64)
* @return   function not a return type
*/
void oled_display_cursor(unsigned char x, unsigned char y)
{
    if((x > display_width) || (y > display_height)) return ;
    positionx = x; positiony = y;
}

/*!
* @brief    set a color for pixel write
* @param    color require color
* @return   function not a return type
*/
void oled_display_color(unsigned char color)
{
    requirecolor = color;
}

/*!
* @brief    set a text size (default: 5 x 7)
* @param    sizex - size of x (* 7 max 4)
* @param    sizey - size of y (* 5 max 25)
* @return   function not a return type
*/
void oled_text_size(unsigned char sizex, unsigned char sizey)
{
    textsizex = (sizey ? sizey : 1);
    textsizey = (sizex ? sizex : 1);
}

/*!
* @brief    draw a single pixel on buffer
* @param    x width of display
* @param    y height of display
* @return   function not a return type
*/
void oled_draw_pixel(unsigned char x, unsigned char y)
{
    if((x > display_width) || (y > display_height)) return ;

    switch(requirecolor)
    {
        case white:  buffer[x + (uint16_t)(y / 8) * display_width] |= (1 << (y & 7)); break;
        case black:  buffer[x + (uint16_t)(y / 8) * display_width] &=~(1 << (y & 7)); break;
        case invert: buffer[x + (uint16_t)(y / 8) * display_width] ^= (1 << (y & 7)); break;
        default: break;
    }
}

/*!
* @brief    draw a bitmap on oled
* @param    bmp require bmp data
* @return   function not a return type
*/
void oled_draw_bitmap(const char *bmp)
{
  char *s = bmp;
  for(unsigned int k = 0; k < 512; k++)
  buffer[k] = pgm_read_byte(s++);
}

/*! 
* @brief    draw a line on oled
* @param    x1 - x start point (0 - 32)
* @param    y1 - y start point (0 - 128)
* @param    x2 - x end point (0 - 31)
* @param    y2 - y end point (0 - 128)
* @return   function not a return type
*/
void oled_draw_line(unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2)
{
    int steep = abs(y2 - y1) > abs(x2 - x1);
    unsigned char reference;

    if(steep)
    {
        reference = y1;
        y1 = x1; x1 = reference;
        reference = y2;
        y2 = x2; x2 = reference;
    }

    if(x1 > x2)
    {
        reference = x1;
        x1 = x2; x2 = reference;
        reference = y1;
        y1 = y2; y2 = reference;
    }

    int dx, dy;
    dx = x2 - x1;
    dy = abs(y2 - y1);

    int err = dx / 2;
    int ystep;

    if(y1 < y2) ystep = 1;
    else ystep = -1;

    for(; x1 <= x2; x1++)
    {
        if(steep) oled_draw_pixel(y1, x1);
        else oled_draw_pixel(x1, y1);
        
        err -= dy;
        if(err < 0)
        {
            y1 += ystep;
            err += dx;
        }
    }
}

/*!
* @brief    draw an horizontal line 
* @param    x - coordinate x (0 - 31)
* @param    y - coordinate y (0 - 127)
* @param    w - width in pixels (0 - 127)
* @return   function not a return type
*/
void oled_draw_hline(unsigned char x, unsigned char y, unsigned char w)
{
    oled_draw_line(x, y, x + w - 1, y);
}

/*!
* @brief    draw a vertical line
* @param    x - coordinate x (0 - 31)
* @param    y - coordinate y (0 - 127)
* @param    h - height in pixels (0 - 31)
* @return   function not a return type
*/
void oled_draw_vline(unsigned char x, unsigned char y, unsigned char h)
{
    oled_draw_line(x, y, x, y + h - 1);
}

/*!
* @brief    draw a border 
* @param    x coordinate x (0 - 32, 64)
* @param    y coordinate y (0 - 128)
* @param    w width   (0 - 128)
* @param    h height  (0 - 32)
* @return   function not a return type
*/
void oled_draw_border(unsigned char x, unsigned char y, unsigned char w, unsigned char h)
{
    oled_draw_hline(x, y, w);
    oled_draw_hline(x, y + h - 1, w);
    oled_draw_vline(x, y, h);
    oled_draw_vline(x + w - 1, y, h);
}

/*!
* @brief    draw a border with filled
* @param    x - coordinate x (0 - 31)
* @param    y - coordinate y (0 - 127)
* @param    w - width (0 - 128)
* @param    h - height (0 - 31)
* @return   function not a return type
*/
void oled_fill_border(unsigned char x, unsigned char y, unsigned char w, unsigned char h)
{
    for(unsigned char variable = x; variable < x + w; variable++)
    oled_draw_vline(variable, y, h);
}

/*!
* @brief    write customized char on lcd display
* @param    data require custom char digits
* @param    function not a return type
*/
void oled_custom_char(const char *data)
{
    if((positionx + textsizex * 8) > display_width) oled_display_cursor(0, positiony + (textsizey * 8));

    for(unsigned char i = 0; i < 5; i++)
    {
        unsigned char byte = data[i];
        for(unsigned char j = 0; j < 8; j++, byte >>= 1)
        {
          requirecolor = (byte & 1 ?white :black);
          if(textsizex == 1 && textsizey == 1)
          oled_draw_pixel(positionx + i, positiony + j);
          else oled_fill_border(positionx + i * textsizex, positiony + j * textsizey, textsizex, textsizey);
        }
    }

    requirecolor = white;
    oled_display_cursor(positionx + (textsizex * 8), positiony);
    if(positionx > (display_width - 8))
    oled_display_cursor(0, positiony + 8);
}

/*!
* @brief    write a single char on oled display 
* @param    data require to write
* @return   function not a return type
*/
void oled_write(unsigned char data)
{
    if(data == '\r') oled_display_cursor(0, positiony);
    if(data == '\n') oled_display_cursor(0, positiony + (textsizey * 8));
    if((positionx + textsizex * 8) > display_width) oled_display_cursor(0, positiony + (textsizey * 8));
    if(data == '\r' || data == '\n') return ;

    //unsigned char byte;
    for(unsigned char i = 0; i < 5; i++)
    {
        unsigned char byte = pgm_read_byte(font5x8[data-32] + i);
        for(unsigned char j = 0; j < 8; j++, byte >>= 1)
        {
          requirecolor = (byte & 1 ?white :black);
          if(textsizex == 1 && textsizey == 1)
          oled_draw_pixel(positionx + i, positiony + j);
          else oled_fill_border(positionx + i * textsizex, positiony + j * textsizey, textsizex, textsizey);
        }
    }

    requirecolor = white;
    oled_display_cursor(positionx + (textsizex * 8), positiony);
    if(positionx > (display_width - 8))
    oled_display_cursor(0, positiony + 8);
}

/*!
* @brief    print a sequence of char on oled
* @param    data require char to print on oled
* @return   function not a return type
*/
void oled_print(const char *data)
{
    while(*data)
    oled_write(*data++);
}

/*!
* @brief    print a sequence of char with length on oled
* @param    data require char to print on oled
* @param    length require length to print
* @return   function not a return type
*/
void oled_println(const char *data, unsigned char length)
{
    while(length --> 0)
    oled_write(*data++);
}

/*!
* @brief    print a numberic digit on oled
* @param    data require data to print
* @param    length require length
* @param    base number system
* @return   function not a return type
*/
void oled_decimal(unsigned long data, unsigned char length, int base)
{
    char *ptr = (char*)malloc(16 * sizeof(char));
    oled_print(dtoa(data, ptr, base, &length));
    free(ptr);
}

/*!
* @brief    print a float on oled
* @param    data require data to print
* @param    length require length
* @return   function not a return type
* @note     default number conversion DEC
*/
void oled_float(float data, unsigned char length)
{
    char *ptr = (char*)malloc(16 * sizeof(char));
    oled_print(ftostra(data, ptr, length));
    free(ptr);
}
