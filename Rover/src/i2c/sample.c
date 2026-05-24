#include "../src/i2c/main.h"

#define I2C_TIMEOUT 2500 //in us
uint64_t i2ctimeoutreg;

void i2c_initialize(i2c_inst_t *i2c, gpio_pinmapping_t scl, gpio_pinmapping_t sda, uint16_t frequency)
{
    if(i2c == I2C1) RCC->APB1ENR |= (1 << 21); //i2c1 clock enable
    else RCC->APB1ENR |= (1 << 22); //i2c2 clock enable 

    /* set output max speed with alternate function open drain for scl, sda and enable pullup with clock */
    gpio_config(scl, GPIO_SPEED_50MHZ | GPIO_ALTERNATE_OPEN | GPIO_PULLUP);
    gpio_config(sda, GPIO_SPEED_50MHZ | GPIO_ALTERNATE_OPEN | GPIO_PULLUP);

    if(i2c == i2c1)
    {
        if(scl != PB6 && sda != PB7) AFIO->MAPR |= (1 << 1); //remapped pin
        else AFIO->MAPR &=~ (1 << 1); //default pin 
    }

    /*
    CCR = TR + TW / TPCLK1 
    TR = clock rise time
    TW = clock high time 
    TPCLK1 = 1 / PLCK
    (*values must be ns)

    TRISE = TR / TPLCK1 + 1;
    (*values must be ns)
    */
    const uint32_t pclkf = (SystemCoreClock / 2);
    i2c->CR1   |= (1 << 15); //force reset i2c module 
    i2c->CR1   = 0x00; //reset i2c registers 
    
    i2c->CR2 |= (pclkf / 1000000);
    if(frequency < 400)
    {
        i2c->CCR   = (pclkf / ((frequency * 1000) * 2));
        i2c->CCR  &=~(1 << 15); //standard mode i2c 
        i2c->TRISE = ((pclkf / 1000000) + 1);
    }
    else
    {
        i2c->CCR   =  (pclkf / ((frequency * 1000) * 3));
        i2c->CCR   |= (1 << 15); //fast mode i2c    
        i2c->TRISE = (((pclkf / 1000000 * 300) / 1000) + 1);
    }
    i2c->CR1   |= (1 << 0); //enable i2c peripheral 
}

void i2c_start(i2c_inst_t *i2c)
{
    i2c->CR1 |= ((1 << 8) | (1 << 10)); //enable start & ack for master mode
    i2ctimeoutreg = micros() + I2C_TIMEOUT;
    while(!(i2c->SR1 & (1 << 0))) {if(micros() > i2ctimeoutreg) break;} //wait till start generated...
}

void i2c_stop(i2c_inst_t *i2c)
{
    i2c->CR1 |= (1 << 9); //generate stop condition
    i2ctimeoutreg = micros() + I2C_TIMEOUT;
    while((i2c->SR1 & (1 << 4))) {if(micros() > i2ctimeoutreg) break;} //master mode no need to check stop gen...
}

void i2c_write_sequence(i2c_inst_t *i2c, uint8_t sad, uint8_t *buf, size_t length, bool stop)
{
    i2c_start(i2c); //generate start 

    i2c->DR = sad; //write slave address before start transmission
    i2ctimeoutreg = micros() + I2C_TIMEOUT;
    while(!(i2c->SR1 & (1 << 1))) {if(micros() > i2ctimeoutreg) break;} //wait till address bit set
    uint8_t tmp __attribute__((unused)) = i2c->SR1 | i2c->SR2; //clear address bit 

    while(length --> 0)
    {
        i2ctimeoutreg = micros() + I2C_TIMEOUT;
        while(!(i2c->SR1 & (1 << 7))) {if(micros() > i2ctimeoutreg) break;} //check empty buffer
        i2c->DR = *buf++; //load buffer into i2c line 
        i2ctimeoutreg = micros() + I2C_TIMEOUT;
        while(!(i2c->SR1 & (1 << 2))) {if(micros() > i2ctimeoutreg) break;} //wait till ack received 
    }

    if(stop) i2c_stop(i2c); //terminate trasnfer 
}

void i2c_read_sequence(i2c_inst_t *i2c, uint8_t sad, uint8_t *buf, size_t length)
{
    //refer data sheet STM32 page no 633 (i2c master received)

    i2c_start(i2c); //generate start (or) restart before transmission 

    i2c->DR = sad | 0x01; //write slave address before start transmission
    i2ctimeoutreg = micros() + I2C_TIMEOUT;
    while(!(i2c->SR1 & (1 << 1))) {if(micros() > i2ctimeoutreg) break;} //wait till addr bit set 
    uint8_t temp __attribute__((unused)) = i2c->SR1 | i2c->SR2; //clear adr bit 

    while(length --> 0)
    {
        if(!length)
        {
            i2c->CR1 &=~ (1 << 10); //clear the ack bit 
            i2c_stop(i2c); //generate stop condition 
            i2ctimeoutreg = micros() + I2C_TIMEOUT;
            while(!(i2c->SR1 & (1 << 6))) {if(micros() > i2ctimeoutreg) break;} //wait for new data
            *buf++ = (uint8_t)i2c->DR; //store data on buffer
        }
        else 
        {
            i2ctimeoutreg = micros() + I2C_TIMEOUT;
            while(!(i2c->SR1 & (1 << 6))) {if(micros() > i2ctimeoutreg) break;} //wait for new data
            *buf++ = (uint8_t)i2c->DR; //store data on buffer 
            if(length != 1) i2c->CR1 |= (1 << 10); //set ack for next byte 
        }
    }
}