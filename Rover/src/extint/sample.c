#include "../src/extint/main.h"

/*
void gpio_isr_routine()
{
    if(EXTI->PR & (1 << 0))
    {
        //PIN0 stuff...
        EXTI->PR |= (1 << 0);
    }
    .
    .
    .
    if(EXTI->PR & (1 << 15))
    {
        //PIN15 stuff...
        EXTI->PR |= (1 << 15);
    }
}
*/

void gpio_isr_initialize(gpio_pinmapping_t pin, uint8_t state, uint8_t edge)
{
    gpio_config(pin, (GPIO_INPUT_PUSHPULL | state)); //config gpio as input with userstate
    RCC->APB2ENR |= (1 << 0); //alternate function clock 
    AFIO->EXTICR[(pin & 15) >> 2] |= ((pin >> 4) << ((pin % 4) << 2));

    /*
    Assume PA4 -> gpio pin number PORTA -> 4
    pin >> 4; pin / 15 = 0; 4 / 15 = 0 //selected for special case
    AFIO->EXTICR[(pin & 15) >> 2]; pin & 15 = 4; result >> 2; result / 4 = 1; AFIO->EXTICR[1]
    pin % 4; 4 % 4 = 0; result << 2; result * 4 = 0;
    pin >> 4; pin / 15; 4 / 15 = 0

    Assume PB7 -> gpio pin number PORTB -> 23
    pin >> 4; pin / 15 = 1; 23 / 15 = 1 //selected for usual ext int selection
    AFIO->EXTICR[(pin & 15) >> 2]; pin & 15 = 7; result >> 2; result / 4 = 1; AFIO->EXTICR[1]
    pin % 4; 23 % 4 = 3; result << 2; result * 4 = 12;
    pin >> 4; pin / 15; 23 / 15 = 1 

    Assume PE5 -> gpio pin number PORTE -> 69
    pin >> 4; pin / 15 = 4; 69  /15 = 1; //selected for usual ext int selection
    AFIO->EXTICR[(pin & 15) >> 2]; pin & 15 = 5; result >> 2; result / 4 = 1; AFIO->EXTICR[1]
    pin % 4; 69 % 4 = 1; result << 2; result * 4 = 4;
    pin >> 4; pin / 15; 69 / 15 = 4

    Assume PC9 -> gpio pin number PORTC -> 41
    pin >> 4; pin / 15 = 2; 41 / 15 = 2; //selected for usual ext int selection
    AFIO->EXTICR[(pin & 15) >> 2]; pin & 15 = 9; result >> 2; result / 4 = 2; AFIO->EXTICR[2]
    pin % 4; 41 % 4 = 1; result << 2; result * 4 = 4;
    pin >> 4; pin / 15; 41 / 15 = 2
    */

    if(edge & GPIO_ISR_RISING) EXTI->RTSR |= (1 << (pin & 15)); //add rising edge
    else EXTI->RTSR &=~ (1 << (pin & 15)); //clear rising edge

    if(edge & GPIO_ISR_FALLING) EXTI->FTSR |= (1 << (pin & 15)); //add falling edge
    else EXTI->FTSR &=~ (1 << (pin & 15)); //clear falling edge 

    EXTI->IMR |= (1 << (pin & 15)); //mask appropiar interrupt register 
}

void gpio_isr_enable(gpio_pinmapping_t pin, void *fun)
{
    uint8_t nvicnumber = pin & 15;

    switch(nvicnumber)
    {
        case 0: //static handler for (PA0....PG0)
        NVIC_SetVector(EXTI0_IRQn, (uint32_t)fun);
        NVIC_EnableIRQ(EXTI0_IRQn);
        break;

        case 1: //static handler for (PA1....PG1)
        NVIC_SetVector(EXTI1_IRQn, (uint32_t)fun);
        NVIC_EnableIRQ(EXTI1_IRQn);
        break;

        case 2: //static handler for (PA2....PG2)
        NVIC_SetVector(EXTI2_IRQn, (uint32_t)fun);
        NVIC_EnableIRQ(EXTI2_IRQn);
        break;

        case 3: //static handler for (PA3....PG3)
        NVIC_SetVector(EXTI3_IRQn, (uint32_t)fun);
        NVIC_EnableIRQ(EXTI3_IRQn);
        break;

        case 4: //static handler for (PA4....PG4)
        NVIC_SetVector(EXTI4_IRQn, (uint32_t)fun);
        NVIC_EnableIRQ(EXTI4_IRQn);
        break;

        case 5: case 6: case 7: case 8: case 9: //shared handler for (PA5,PA6,PA7,PA8,PA9....PG9)
        NVIC_SetVector(EXTI9_5_IRQn, (uint32_t)fun);
        NVIC_EnableIRQ(EXTI9_5_IRQn);
        break;

        case 10: case 11: case 12: case 13: case 14: case 15: //shared handler for (PA10, PA11, PA12, PA13, PA14, PA15....PG15)
        NVIC_SetVector(EXTI15_10_IRQn, (uint32_t)fun);
        NVIC_EnableIRQ(EXTI15_10_IRQn);
        break;

        default: break;
    }
}

void gpio_isr_disable(gpio_pinmapping_t pin)
{
    //can't disable irq shared handler uses same irq 
    //AFIO->EXTICR[(pin & 15) >> 2] &=~ (((pin & 15) << 2) << (pin >> 4)); //disable extint selection
    AFIO->EXTICR[(pin & 15) >> 2] &=~ (((pin >> 4) << ((pin % 4) << 2)));
    EXTI->RTSR &=~ (1 << (pin & 15)); EXTI->FTSR &=~ (1 << (pin & 15)); //clear rising & falling edges
    EXTI->IMR &=~ (1 << (pin & 15)); //unmaks isr routine
}