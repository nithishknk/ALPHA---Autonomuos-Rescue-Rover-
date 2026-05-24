#include "../src/gpio/main.h"

/**
 * @brief set gpio pin as output (legacy)
 * 
 * @param pin gpio_pinmapping_t pin
 */
void gpio_set_output(gpio_pinmapping_t pin)
{
    gpio_output(pin);
}

/**
 * @brief set gpio pin as input (legacy)
 * 
 * @param pin gpio_pinmapping_t pin 
 */
void gpio_set_input(gpio_pinmapping_t pin)
{
    gpio_input(pin);
}

/**
 * @brief write gpio high (or) pullup (legacy)
 * 
 * @param pin gpio_pinmapping_t pin 
 */
void gpio_put_high(gpio_pinmapping_t pin)
{
    gpio_high(pin);
}

/**
 * @brief write gpio low (or) pulldown (legacy)
 * 
 * @param pin gpio_pinmapping_t pin 
 */
void gpio_put_low(gpio_pinmapping_t pin)
{
    gpio_low(pin);
}

/**
 * @brief toggle a gpio pin (legacy)
 * 
 * @param pin gpio_pinmapping_t pin 
 */
void gpio_put_toggle(gpio_pinmapping_t pin)
{
    gpio_toggle(pin);
}