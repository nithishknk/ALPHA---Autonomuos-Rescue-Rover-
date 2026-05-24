#include "../src/hcsr04/main.h"

gpio_pinmapping_t radartrigger;
gpio_pinmapping_t radarecho;

volatile uint64_t startmicros;
volatile uint16_t distance;

void radar_isr_routine()
{
    if(EXTI->PR & (1 << (radarecho & 15)))
    {
        if(gpio_get(radarecho)) startmicros = micros();
        else if(!gpio_get(radarecho)) distance = (uint16_t)((micros() - startmicros) * 0.034) / 2;
        EXTI->PR |= (1 << (radarecho & 15));
    }
}

void radar_initialize(gpio_pinmapping_t trigger, gpio_pinmapping_t echo)
{
    radartrigger = trigger;
    radarecho = echo;

    gpio_set_output(trigger);
    gpio_put_low(trigger);

    gpio_isr_initialize(echo, GPIO_PULLDOWN, GPIO_ISR_CHANGE);
    startmicros = distance = 0;
    gpio_isr_enable(echo, radar_isr_routine);

    gpio_high(radartrigger); delay_us(2);
    gpio_low(radartrigger); delay_us(10);
}

uint16_t radar_fetch(void) 
{ 
    gpio_high(radartrigger); delay_us(2);
    gpio_low(radartrigger); delay_us(10);
    return distance; 
}