#ifndef _ADC_H
#define _ADC_H

#define adc_bit_rate  10
#define adc_max_value 1023.0F

extern void adc_initialize(void);
extern unsigned int read_adc(unsigned char);
extern unsigned int read_adc_raw(unsigned char, unsigned int);
extern float read_adc_voltage(unsigned char);
#endif
