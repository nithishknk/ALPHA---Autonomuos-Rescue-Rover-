#include <avr/io.h>

#include "gpio.h"
#include "utils.h"
#include "spi.h"

unsigned char serialclock;
unsigned char masterinslaveout;
unsigned char masteroutslavein;

void spi_initialize(char sck, char miso, char mosi)
{
  serialclock = sck;
  masterinslaveout = miso;
  masteroutslavein = mosi;

  gpio_set_output(serialclock);
  gpio_set_output(masteroutslavein);
  gpio_set_input(masterinslaveout);
  
  gpio_put_high(masteroutslavein);
  gpio_put_low(serialclock);
}

void spi_write(unsigned char data)
{
  for(unsigned char shifting = 0x80; shifting; shifting >>= 1)
  {
    if(shifting & data) gpio_put_high(masteroutslavein);
    else gpio_put_low(masteroutslavein);
    gpio_put_high(serialclock);
    gpio_put_low(serialclock);
  }
}

unsigned char spi_read()
{
  unsigned char returnvariable = 0;
  for(unsigned char shifting = 0x80; shifting; shifting >>= 1)
  {
    if(gpio_get(masterinslaveout)) returnvariable |= shifting;
    else returnvariable &=~ shifting;
    gpio_put_high(serialclock); 
    gpio_put_low(serialclock); 
  }
  return returnvariable;
}

unsigned char spi_read_write(unsigned char data)
{
  unsigned char returnvariable = 0;
  for(unsigned char shifting = 0x80; shifting; shifting >>= 1)
  {
    if(shifting & data) gpio_put_high(masteroutslavein);
    else gpio_put_low(masteroutslavein); gpio_put_high(serialclock); 
    if(gpio_get(masterinslaveout)) returnvariable |= shifting;
    else returnvariable &=~ shifting; gpio_put_low(serialclock); 
  }
  return returnvariable;
}

void spi_write_sequence(unsigned char *data, unsigned int length)
{
  for(unsigned int k = 0; k < length; k++)
  spi_write(data[k]);
}

void spi_read_sequence(unsigned char *data, unsigned int length)
{
  for(unsigned int k = 0; k < length; k++)
  data[k] = spi_read();
}

void spi_read_write_sequence(unsigned char *data, unsigned int length)
{
  for(unsigned int k = 0; k < length; k++)
  data[k] = spi_read_write(data[k]);
}
