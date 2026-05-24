#include "../src/oled/main.h"
#include "../src/oled/font5x8.h"

oled_config_t oled_config;

void oled_initialize(i2c_inst_t *i2c, uint16_t height, uint16_t width)
{
    oled_config.i2c = i2c;
    oled_config.display_height = height;
    oled_config.display_width = width;
    oled_config.buffer_size = ((height * width) >> 3);
    oled_config.buffer = (char*)malloc(oled_config.buffer_size * sizeof(char));
    oled_set_cursor(0, 0); oled_set_tsize(1, 1); oled_set_color(white);

    oled_command(display_comm_off);

    oled_command(display_power_freq);
    oled_command(0x80);

    oled_command(display_config_multiplex);
    oled_command(height - 1);

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
    if(height == 32) oled_command(0x02);
    else oled_command(0x12);

    oled_command(display_line_remap);
    oled_command(display_config_mapscan);

    oled_command(display_config_compins);
    if(height == 32) oled_command(0x00);
    else oled_command(0x12);

    oled_command(display_comm_contrast);
    if(height == 32) oled_command(0x8F);
    else oled_command(0xCF);

    oled_command(display_power_period);
    oled_command(0xF1);

    oled_command(display_power_vcomh);
    oled_command(0x40);

    oled_command(display_comm_onram);

    oled_set_invert(false);

    oled_command(display_scroll_disable);

    oled_command(display_comm_on);

    oled_fill_screen(black);
}

void oled_fill_screen(uint8_t color)
{
    if(color) memset(oled_config.buffer, 0xFF, oled_config.buffer_size);
    else memset(oled_config.buffer, 0x00, oled_config.buffer_size);
    oled_display();
}

void oled_display(void)
{
    if(oled_config.display_height == 32)
    {
        oled_command(display_line_coladdress);
        oled_command(0); 
        oled_command(oled_config.display_width - 1);

        oled_command(display_line_pageaddress);
        oled_command(0);
        oled_command((oled_config.display_height >> 3) - 1);

        char *ptr = (char*)malloc((oled_config.buffer_size + 1) * sizeof(char));
        ptr[0] = display_line_startline; memcpy(&ptr[1], oled_config.buffer, oled_config.buffer_size);
        i2c_write_sequence(oled_config.i2c, 0x78, (uint8_t*)ptr, oled_config.buffer_size + 1, true); free(ptr);
    }
    else 
    {
        for(uint16_t i = 0; i < (oled_config.display_height >> 3); i++)
        {
            oled_command(0xB0 + i + 0);
            oled_command(2 & 0xF);
            oled_command(0x10 | (2 >> 4));

            char *ptr = (char*)malloc(129 * sizeof(char));
            ptr[0] = display_line_startline; memcpy(&ptr[1], &oled_config.buffer[128 * i], 128);
            i2c_write_sequence(oled_config.i2c, 0x78, (uint8_t*)ptr, 129, true); free(ptr);
        }
    }
}

void oled_write(uint8_t data)
{
    if(data == '\r') oled_set_cursor(0, oled_config.position_y);
    if(data == '\n') oled_set_cursor(oled_config.position_x, oled_config.position_y + (oled_config.textsize_y * 8));
    if((oled_config.position_x + oled_config.textsize_x * 8) > oled_config.display_width) 
    oled_set_cursor(0, oled_config.position_y + (oled_config.textsize_y * 8));
    if(data == '\r' || data == '\n') return ;

    for(uint8_t i = 0; i < 5; i++)
    {
        uint8_t lutbyte = font5x8[data - 32][i];
        for(uint8_t j = 0; j < 8; j++, lutbyte >>= 1)
        {
            oled_set_color(lutbyte & 1);
            if(oled_config.textsize_x == 1 && oled_config.textsize_y == 1)
            oled_draw_pixel(oled_config.position_x + i, oled_config.position_y + j);
            else oled_fill_rect(oled_config.position_x + i * oled_config.textsize_x,
            oled_config.position_y + j * oled_config.textsize_y, oled_config.textsize_x, oled_config.textsize_y);
        }
    }

    oled_set_color(white);
    oled_set_cursor(oled_config.position_x + (oled_config.textsize_x * 8), oled_config.position_y);
    if(oled_config.position_x > (oled_config.display_width - 8)) 
    oled_set_cursor(0, oled_config.position_y + 8);
}

void oled_custom_write(const char *data)
{
    if((oled_config.position_x + oled_config.textsize_x * 8) > oled_config.display_width) 
    oled_set_cursor(0, oled_config.position_y + (oled_config.textsize_y * 8));

    for(uint8_t i = 0; i < 5; i++)
    {
        uint8_t lutbyte = data[i];
        for(uint8_t j = 0; j < 8; j++, lutbyte >>= 1)
        {
            oled_set_color(lutbyte & 1); 
            if(oled_config.textsize_x == 1 && oled_config.textsize_y == 1)
            oled_draw_pixel(oled_config.position_x + i, oled_config.position_y + j);
            else oled_fill_rect(oled_config.position_x + i * oled_config.textsize_x,
            oled_config.position_y + j * oled_config.textsize_y, oled_config.textsize_x, oled_config.textsize_y);
        }
    }

    oled_set_color(white);
    oled_set_cursor(oled_config.position_x + (oled_config.textsize_x * 8), oled_config.position_y);
    if(oled_config.position_x > (oled_config.display_width - 8)) 
    oled_set_cursor(0, oled_config.position_y + 8);
}

void oled_print(const char *data)
{
    while(*data)
    oled_write(*data++);
}

void oled_println(const char *data, size_t length)
{
    while(length --> 0)
    oled_write(*data++);
}

void oled_decimal(uint32_t data, size_t length, int base)
{
    char *ptr = (char*)malloc(16 * sizeof(char));
    oled_print(dtoa(data, ptr, base, length)); 
    free(ptr);
}

void oled_float(float data, size_t length)
{
    char *ptr = (char*)malloc(16 * sizeof(char));
    oled_print(ftostra(data, ptr, length));
    free(ptr);
}

void oled_printf(const char *data, ...)
{
    char *ptr = (char*)malloc(512 * sizeof(char));
    va_list args; va_start(args, data);
    size_t length = vsnprintf(ptr, 512, data, args);
    oled_println(ptr, length); free(ptr);
}

void oled_draw_pixel(uint8_t x, uint8_t y)
{
    if((x > oled_config.display_width) || (y > oled_config.display_height)) return ;

    switch(oled_config.textcolor)
    {
        case white:  oled_config.buffer[x + (uint16_t)(y / 8) * oled_config.display_width] |=  (1 << (y & 7)); break;
        case black:  oled_config.buffer[x + (uint16_t)(y / 8) * oled_config.display_width] &=~ (1 << (y & 7)); break;
        case invert: oled_config.buffer[x + (uint16_t)(y / 8) * oled_config.display_width] ^=  (1 << (y & 7)); break;
        default: break;
    }
}

void oled_draw_line(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2)
{
    int16_t steep = abs(y2 - y1) > abs(x2 - x1);
    uint8_t reference;

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

void oled_draw_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
{
    oled_draw_hline(x, y, w);
    oled_draw_hline(x, y + h - 1, w);
    oled_draw_vline(x, y, h);
    oled_draw_vline(x + w - 1, y, h);
}

void oled_fill_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
{
    for(uint8_t var = x; var < x + w; var++)
    oled_draw_vline(var, y, h);
}