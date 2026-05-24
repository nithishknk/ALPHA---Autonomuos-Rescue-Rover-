#include "../src/spi/main.h"

uint8_t spi1_repeated_data = 0x00;
uint8_t spi2_repeated_data = 0x00;

void spi_initialize(spi_inst_t *spi, uint8_t sck, uint8_t mosi, uint8_t miso, spi_freq_t freq)
{
    if(spi == SPI1) RCC->APB2ENR |= (1 << 12); //spi1 clock enable
    else RCC->APB1ENR |= (1 << 14); //spi2 clock enabele

    gpio_config(sck,  GPIO_SPEED_50MHZ | GPIO_ALTERNATE_PUSHPULL);
    gpio_config(miso, GPIO_INPUT | GPIO_FLOATING_INPUT);
    gpio_config(mosi, GPIO_SPEED_50MHZ | GPIO_ALTERNATE_PUSHPULL);

    if(spi == spi1)
    {
        if(sck != PA5 && miso != PA6 && mosi != PA7) AFIO->MAPR |= (1 << 0); //remapped pin
        else AFIO->MAPR &=~ (1 << 0); //default pin 
    }

    //default function initialization
    spi->CR1 = 0x00; //disable spi peripherals
    
    spi->CR1 |= (1 << 2); //master mode 
    spi->CR1 &=~ ((1 << 0) | (1 << 1)); //cpol = 0; cpha = 0
    spi->CR1 &=~ (1 << 7); //msb first 
    spi->CR1 &=~ (1 << 11); //8bit data mode 
    
    spi->CR1 |= ((1 << 8) | (1 << 9)); //software slave management
    spi->CR1 &=~ (1 << 10); //full duplex mode
    spi->CR2 = 0x00; //disable irq & dma use polling method 
    spi->CR1 &=~ ((1 << 3) | (1 << 4) | (1 << 5)); //reset baudregister
    spi->CR1 |= (freq << 3); //select require baudrate 

    spi->CR1 |= (1 << 6); //enable spi peripherals
}

void spi_set_repeated(spi_inst_t *spi, uint8_t data)
{
    if(spi == SPI1) spi1_repeated_data = data; //set repeated for spi1
    else spi2_repeated_data = data; //set repeated for spi2
}

void spi_write(spi_inst_t *spi, uint8_t data)
{
    spi->DR = data;
    while(!(spi->SR & (1 << 1))); //wait till tx buffer empty
    while((spi->SR & (1 << 7)));  //wait busy flag reset
    uint8_t tmp1 __attribute__((unused)) = spi->DR; //dummy receive
    uint8_t tmp2 __attribute__((unused)) = spi->SR; //clear overrun 
}

uint8_t spi_read(spi_inst_t *spi)
{
    while((spi->SR & (1 << 7))); //wait busy flag reset
    spi->DR = (spi == SPI1 ? spi1_repeated_data : spi2_repeated_data); //dummy write
    while(!(spi->SR & (1 << 0))); //wait for rx flag set
    return spi->DR; //return read data from spi line
}

uint8_t spi_read_write(spi_inst_t *spi, uint8_t data)
{
    spi->DR = data;
    while(!(spi->SR & (1 << 1))); //wait till tx buffer empty
    while(!(spi->SR & (1 << 0))); //wait for rx flag set
    while((spi->SR & (1 << 7)));  //wait busy flag reset
    return spi->DR; //return read data from spi line 
}

void spi_write_sequence(spi_inst_t *spi, uint8_t *data, uint16_t length)
{
    for(uint16_t k = 0; k < length; k++)
    {
        while(!(spi->SR & (1 << 1))); //wait till tx buffer empty
        spi->DR = data[k];
    }

    while(!(spi->SR & (1 << 1))); //wait till tx buffer empty
    while((spi->SR & (1 << 7)));  //wait busy flag reset
    uint8_t tmp1 __attribute__((unused)) = spi->DR; //dummy receive
    uint8_t tmp2 __attribute__((unused)) = spi->SR; //clear overrun
}

void spi_read_sequence(spi_inst_t *spi, uint8_t *data, uint16_t length)
{
    for(uint16_t k = 0; k < length; k++) //poll till reaches it's length
    data[k] = spi_read(spi); //read sequence of data from spi
}

void spi_read_write_sequence(spi_inst_t *spi, uint8_t *data, uint16_t length)
{
    for(uint16_t k = 0; k < length; k++) //poll till reaches it's length
    data[k] = spi_read_write(spi, data[k]); //write then read sequence of data from spi
}