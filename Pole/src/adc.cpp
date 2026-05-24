#include <avr/io.h>

#include "gpio.h"
#include "utils.h"
#include "adc.h"

void adc_initialize()
{
  ADMUX |= (1 << REFS0);
  ADCSRA |= ((1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0));
  ADCSRA |= (1 << ADEN);
}

unsigned int read_adc(unsigned char channel)
{
  ADMUX = (ADMUX & 0xF0) | (channel & 0x0F);
  ADCSRA |= (1 << ADSC);
  while(ADCSRA & (1 << ADSC));
  return ADC;
}

float read_adc_voltage(unsigned char channel)
{
  return ((read_adc_raw(channel, 10) / adc_max_value) * 5); 
}

unsigned int read_adc_raw(unsigned char channel, unsigned int count)
{
  unsigned long returnvariable = 0;
  for(unsigned char variable = 0; variable < count; variable++)
  returnvariable = returnvariable + read_adc(channel);
  return returnvariable / count;
}
