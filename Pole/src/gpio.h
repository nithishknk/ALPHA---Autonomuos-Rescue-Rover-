#ifndef _GPIO_H
#define _GPIO_H

#define portstate 0
#define trisstate 3
#define pinstate  6

static volatile unsigned char *pinmapping[] = {&PORTD, &PORTB, &PORTC, &DDRD, &DDRB, &DDRC, &PIND, &PINB, &PINC};

typedef enum
{
  AN0 = 16, AN1, AN2, AN3, AN4, AN5, AN6, AN7,
  RD0 = 0, RD1, RD2, RD3, RD4, RD5, RD6, RD7,
  RB0, RB1, RB2, RB3, RB4, RB5, RB6, RB7,
  RC0, RC1, RC2, RC3, RC4, RC5, RC6, RC7
}
atmega328_gpio_pin_mapping;

#if defined(__AVR_ATmega2560__)
#define gpio_set_output(pin)  pinMode(pin, OUTPUT);
#define gpio_set_input(pin)   pinMode(pin, INPUT);
#define gpio_set_toggle(pin)  pinMode(pin, !pinMode(pin));

#define gpio_put_high(pin)    digitalWrite(pin, HIGH);
#define gpio_put_low(pin)     digitalWrite(pin, LOW);
#define gpio_put_toggle(pin)  digitalWrite(pin, !digitalRead(pin));

#define gpio_get(pin)         digitalRead(pin);

#else
#define gpio_set_output(pin)  (*(pinmapping[trisstate + (pin >> 3)]) |= (1 << (pin & 0x07)))
#define gpio_set_input(pin)   (*(pinmapping[trisstate + (pin >> 3)]) &=~(1 << (pin & 0x07)))
#define gpio_set_toggle(pin)  (*(pinmapping[trisstate + (pin >> 3)]) ^= (1 << (pin & 0x07)))

#define gpio_put_high(pin)    (*(pinmapping[portstate + (pin >> 3)]) |= (1 << (pin & 0x07)))
#define gpio_put_low(pin)     (*(pinmapping[portstate + (pin >> 3)]) &=~(1 << (pin & 0x07)))
#define gpio_put_toggle(pin)  (*(pinmapping[portstate + (pin >> 3)]) ^= (1 << (pin & 0x07)))

#define gpio_get(pin)         (*(pinmapping[pinstate + (pin >> 3)]) & (1 << (pin & 0x07)) ?1 :0)
#endif
#endif
