/**
 * @file    stm32f1xx gpio.h
 * @author  saurash automations
 * @brief   stm32 gpio header file
 * @version 0.1
 * @date    2024-12-02
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef _GPIO_H
#define _GPIO_H

#include "../src/utils/main.h"

#define GPIO(pin) ((GPIO_TypeDef *) (APB2PERIPH_BASE + (0x00000800UL + (((pin >> 4) & 0xF) * 0x400L)))) /* mapped port address */

#define GPIO_INPUT                  (0b00 << 0)  /* pin act as input */
#define GPIO_SPEED_10MHZ            (0b01 << 0)  /* output speed @ 10mhz */
#define GPIO_SPEED_2MHZ             (0b10 << 0)  /* output speed @ 2mhz  */
#define GPIO_SPEED_50MHZ            (0b11 << 0)  /* output speed @ 50mhz */

#define GPIO_OUTPUT_PUSHPULL        (0b00 << 2) /* gpio output pushpull */
#define GPIO_OUTPUT_OPEN            (0b01 << 2) /* gpio output open drain */    
#define GPIO_ALTERNATE_PUSHPULL     (0b10 << 2) /* gpio alternate pushpull */
#define GPIO_ALTERNATE_OPEN         (0b11 << 2) /* gpio alternate open */

#define GPIO_ANALOG_INPUT           (0b00 << 2) /* gpio analog input */
#define GPIO_FLOATING_INPUT         (0b01 << 2) /* gpio floating input */
#define GPIO_INPUT_PUSHPULL         (0b10 << 2) /* gpio input push/pull */

#define GPIO_SET                    (1 << 4)    /* set gpio pin high */
#define GPIO_RESET                  (0 << 4)    /* reset gpio pin low */
#define GPIO_PULLUP                 GPIO_SET    /* add pullup to a pin */
#define GPIO_PULLDOWN               GPIO_RESET  /* add pulldown to pin */


typedef enum
{
    NOT_USED_PIN = -1,

    /* PORTA enumeration 0 to 15 */
    PA0, PA1, PA2, PA3, PA4, PA5, PA6, PA7,
    PA8, PA9, PA10, PA11, PA12, PA13, PA14, PA15, 

    /* PORTB enumeration 16 to 31 */
    PB0, PB1, PB2, PB3, PB4, PB5, PB6, PB7, 
    PB8, PB9, PB10, PB11, PB12, PB13, PB14, PB15, 

    /* PORTC enumeration 32 to 47 */
    PC0, PC1, PC2, PC3, PC4, PC5, PC6, PC7, 
    PC8, PC9, PC10, PC11, PC12, PC13, PC14, PC15,

    /* PORTD enumeration 48 to 63 */
    PD0, PD1, PD2, PD3, PD4, PD5, PD6, PD7, 
    PD8, PD9, PD10, PD11, PD12, PD13, PD14, PD15,

    /* PORTE enumeration 64 to 79 */
    PE0, PE1, PE2, PE3, PE4, PE5, PE6, PE7, 
    PE8, PE9, PE10, PE11, PE12, PE13, PE14, PE15,

    /* PORTF enumeration 80 to 95 */
    PF0, PF1, PF2, PF3, PF4, PF5, PF6, PF7,
    PF8, PF9, PF10, PF11, PF12, PF13, PF14, PF15, 

    /* PORTG enumeration 96 to 111 */
    PG0, PG1, PG2, PG3, PG4, PG5, PG6, PG7, 
    PG8, PG9, PG10, PG11, PG12, PG13, PG14, PG15,
}
gpio_pinmapping_t; /*stm32f mapped gpio pins */

//Legacy functions (same as inline)

extern void gpio_set_output(gpio_pinmapping_t pin);
extern void gpio_set_input(gpio_pinmapping_t pin);
extern void gpio_put_high(gpio_pinmapping_t pin);
extern void gpio_put_low(gpio_pinmapping_t pin);
extern void gpio_put_toggle(gpio_pinmapping_t pin);

/**
 * @brief stm32f gpio function config 
 * 
 * @param pin       gpio_pinmapping_t pin number
 * @param config    configuration words
 * @return true     if pin read high
 * @return false    if pin read low
 * @note  appropiar port/afio clock will enabled 
 */
static inline bool gpio_config(gpio_pinmapping_t pin, uint8_t config)
{
    GPIO_TypeDef *gpio = GPIO(pin); //port address
    RCC->APB2ENR |= ((1 << ((pin >> 4) & 0xF)) << 2); //enable port clock
    if(((config >> 2) & 0b11) > 1) RCC->APB2ENR |= (1 << 0); //alternate function clock   

    if((pin & 15) < 8) 
    {
        gpio->CRL &=~ (0b1111 << ((pin & 7) << 2)); //clear mode & config words
        gpio->CRL |= ((config & 0b1111) << ((pin & 7) << 2)); //set mode & config words
    }
    else
    {
        gpio->CRH &=~ (0b1111 << ((pin & 7) << 2)); //clear mode & config words
        gpio->CRH |= ((config & 0b1111) << ((pin & 7) << 2)); //set mode & config words 
    }

    if(config & (1 << 4)) gpio->ODR |= ((1 << (pin & 15))); //same register for set/pullup
    else gpio->ODR &=~ ((1 << (pin & 15))); //same register for reset/pulldown

    return ((gpio->IDR & (1 << (pin & 15))) ?true :false);
}

/**
 * @brief function read gpio input
 * 
 * @param pin gpio_pinmapping_t pin
 * @return true  high
 * @return false low
 */
static inline bool gpio_get(gpio_pinmapping_t pin)
{
    GPIO_TypeDef *gpio = GPIO(pin); //port address
    return ((gpio->IDR & (1 << (pin & 15))) ? true : false);
}

/**
 * @brief set pin as output
 * 
 * @param pin gpio_pinmapping_t pin
 */
static inline void gpio_output(gpio_pinmapping_t pin)
{
    GPIO_TypeDef *gpio = GPIO(pin); //port address
    RCC->APB2ENR |= ((1 << ((pin >> 4) & 0xF)) << 2); //enable port clock

    if((pin & 15) < 8) //for pins 0...7 
    {
        gpio->CRL &=~ (0b1111 << ((pin & 7) << 2)); //clear apporopiar flag
        gpio->CRL |=  (0b0001 << ((pin & 7) << 2)); //output pushpull with 10mhz speed
    }
    else //for pins 8...15
    {
        gpio->CRH &=~ (0b1111 << ((pin & 7) << 2)); //clear appropiar flag
        gpio->CRH |=  (0b0001 << ((pin & 7) << 2)); //output pushpull with 10mhz speed
    }
}

/**
 * @brief set pin as input
 * 
 * @param pin gpio_pinmapping_t pin
 */
static inline void gpio_input(gpio_pinmapping_t pin)
{
    GPIO_TypeDef *gpio = GPIO(pin); //port address
    RCC->APB2ENR |= ((1 << ((pin >> 4) & 0xF)) << 2); //enable port clock

    if((pin & 15) < 8) //for pins 0...7
    {
        gpio->CRL &=~ (0b1111 << ((pin & 7) << 2)); //clear appropiar flag
        gpio->CRL |=  (0b1000 << ((pin & 7) << 2)); //input pullup/down mode 
    }
    else
    {
        gpio->CRH &=~ (0b1111 << ((pin & 7) << 2)); //clear appropiar flag
        gpio->CRH |=  (0b1000 << ((pin & 7) << 2)); //input pullup/down mode 
    }
}

/**
 * @brief write pin high (or) add pullup
 * 
 * @param pin gpio_pinmapping_t pin 
 */
static inline void gpio_high(gpio_pinmapping_t pin)
{
    GPIO_TypeDef *gpio = GPIO(pin); //port address
    gpio->ODR |= (1 << (pin & 15)); //set corresponding odr bit 
}

/**
 * @brief write pin low (or) add pulldown
 * 
 * @param pin gpio_pinmapping_t pin 
 */
static inline void gpio_low(gpio_pinmapping_t pin)
{
    GPIO_TypeDef *gpio = GPIO(pin); //port address
    gpio->ODR &=~ (1 << (pin & 15)); //reset corresponding odr bit 
}

/**
 * @brief toggle an output pin
 * 
 * @param pin gpio_pinmapping_t pin 
 */
static inline void gpio_toggle(gpio_pinmapping_t pin)
{
    GPIO_TypeDef *gpio = GPIO(pin); //port address
    gpio->ODR ^= (1 << (pin & 15)); //toggle output direction BSRR work
}
#endif 