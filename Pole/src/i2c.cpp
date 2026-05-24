#include <Arduino.h>
#include <avr/io.h>

#include "utils.h"
#include "i2c.h"

void i2c_initialize(unsigned int freq)
{
  unsigned char reg =  (F_CPU / ((freq * 1000) - 16UL) / 2); //for 1:1PS
  TWSR = 0x00;
  TWBR = reg;
  TWCR |= (1 << TWEN);
}

void i2c_start(void)
{
  TWCR = ((1 << TWINT) | (1 << TWSTA) | (1 << TWEN));
  while(!(TWCR & (1 << TWINT))) continue;
}

void i2c_stop(void)
{
  TWCR = ((1 << TWINT) | (1 << TWSTO) | (1 << TWEN));
}

void i2c_restart(void)
{
  //No Restart Condition For AVR controllers instead start
  TWCR = ((1 << TWINT) | (1 << TWSTA) | (1 << TWEN));
  while(!(TWCR & (1 << TWINT))) continue;
}

unsigned char i2c_write(unsigned char data)
{
  TWDR = data & 0xFF;
  TWCR = ((1 << TWINT) | (1 << TWEN) | (1 << TWEA));
  while(!(TWCR & (1 << TWINT))) continue;
  return ((TWSR & 0xF8) == 0x18 ?false :true);
}

unsigned char i2c_read(unsigned char ack)
{
  TWCR = ((1 << TWINT) | (1 << TWEN) | (ack ?0 :1 << TWEA));
  while(!(TWCR & (1 << TWINT))) continue;
  return TWDR;
}

unsigned char i2c_register_write(unsigned char dad, unsigned char rad, unsigned char dt)
{
  unsigned char ackstate;
  i2c_start();
  i2c_write(dad);
  i2c_write(rad);
  ackstate = i2c_write(dt);
  i2c_stop();
  return ackstate;
}

unsigned char i2c_register_read(unsigned char dad, unsigned char rad)
{
  unsigned char returnvariable;
  i2c_start();
  i2c_write(dad);
  i2c_write(rad);
  i2c_restart();
  i2c_write(dad | 1);
  returnvariable = i2c_read(true);
  i2c_stop();
  return returnvariable;
}

void i2c_sequence_write(unsigned char dad, unsigned char rad, unsigned char *dar, unsigned int length)
{
  i2c_start();
  i2c_write(dad);
  i2c_write(rad);
  while(length --> 0)
  i2c_write(*dar++);
  i2c_stop();
}

void i2c_sequence_read(unsigned char dad, unsigned char rad, unsigned char *dar, unsigned int length)
{
  memset(dar, '\0', length);
  i2c_start();
  i2c_write(dad);
  i2c_write(rad);
  i2c_restart();
  i2c_write(dad | 1);
  while(length --> 0)
  *dar++ = i2c_read(length ?false :true);
  i2c_stop();
}

unsigned char i2c_bus_scan(void)
{
  unsigned char ackstate;
  for(unsigned int i = 0x10; i < 0xFF; i += 2)
  {
    i2c_start();
    ackstate = i2c_write(i);
    i2c_stop();

    if(!ackstate) return i;
    else delay_ms(100);
  }
  return false;
}
