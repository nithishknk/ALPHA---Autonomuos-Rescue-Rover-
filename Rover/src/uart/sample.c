#include "../src/uart/main.h"

uint8_t serialarray1[150], serialcount1;
uint8_t serialarray2[150], serialcount2;
uint8_t serialarray3[150], serialcount3;

void serial1_isr_handler()
{
    serialarray1[serialcount1] = serial_receive(uart1);
    serialcount1 = (serialcount1 < 150 ? serialcount1 + 1 : 150);
}

void serial2_isr_handler()
{
    serialarray2[serialcount2] = serial_receive(uart2);
    serialcount2 = (serialcount2 < 150 ? serialcount2 + 1 : 150);
}

void serial3_isr_handler() 
{
    serialarray3[serialcount3] = serial_receive(uart3);
    serialcount3 = (serialcount3 < 150 ? serialcount3 + 1 : 150);
}

void serial_initialize(uart_inst_t *uart, gpio_pinmapping_t txr, gpio_pinmapping_t rxr, uint32_t baudrate)
{
    uint8_t uart_index_number = uartindex(uart);

    switch(uart_index_number)
    {
        case 1: RCC->APB2ENR |= (1 << 14); break; //usart 1 clock enable
        case 2: RCC->APB1ENR |= (1 << 17); break; //usart 2 clock enable
        case 3: RCC->APB1ENR |= (1 << 18); break; //usart 3 clock enable 
    }

    //appropiar txr & rxr pin configuration with clock enable 
    gpio_config(txr, GPIO_SPEED_50MHZ | GPIO_ALTERNATE_PUSHPULL);
    gpio_config(rxr, GPIO_INPUT | GPIO_FLOATING_INPUT);

    if(uart == uart1)
    {
        if(txr != PA9 && rxr != PA10) AFIO->MAPR |= (1 << 2); //remapped pin
        else AFIO->MAPR &=~ (1 << 2); //default pin 
    }

    /*
    BAUD = FCK / (16 * UARTDIV)
    UARTDIV = FCK / (16 * BAUD)

    UARTDIV = 72000000 / (16 * 9600) = 468.75
    468 = 0x1D4; 0.75 * 16 = 0xC
    UARTDIV = 0x1D4C */

    double ref = ((uart_index_number == 1 ? PCLK2 : PCLK1) / (16 * baudrate));
    uint32_t intp = (int)ref; double frap = ref - intp;
    uart->BRR =  ((intp << 4) | ((unsigned)(16.0F * frap))); 
    uart->CR1 &=~ (1 << 12); uart->CR2 &=~ (0b11 << 12); //1st, 8dt, 1sp, nopar 
    uart->CR1 |= ((1 << 3) | (1 << 13)); //enable txr and uart
}

void serial_enable(uart_inst_t *uart, void *fun, bool state)
{
    uint8_t uart_index_number = uartindex(uart);

    switch(uart_index_number)
    {
        case 1:
        if(fun != NULL) NVIC_SetVector(USART1_IRQn, (uint32_t)fun); //custom isr handler
        else NVIC_SetVector(USART1_IRQn, (uint32_t)serial1_isr_handler); //predefined handler
        if(state) NVIC_EnableIRQ(USART1_IRQn); //enable irq1 isr
        else NVIC_DisableIRQ(USART1_IRQn); //disable irq1 isr
        break;

        case 2:
        if(fun != NULL) NVIC_SetVector(USART2_IRQn, (uint32_t)fun); //custom isr handler
        else NVIC_SetVector(USART2_IRQn, (uint32_t)serial2_isr_handler); //predefined handler
        if(state) NVIC_EnableIRQ(USART2_IRQn); //enable irq2 isr
        else NVIC_DisableIRQ(USART2_IRQn); //disable irq2 isr
        break;

        case 3:
        if(fun != NULL) NVIC_SetVector(USART3_IRQn, (uint32_t)fun); //custom isr handler
        else NVIC_SetVector(USART3_IRQn, (uint32_t)serial3_isr_handler); //predefined handler 
        if(state) NVIC_EnableIRQ(USART3_IRQn); //enable irq3 isr
        else NVIC_DisableIRQ(USART3_IRQn); //disable irq3 isr 
        break;
    }

    if(state) uart->CR1 |= ((1 << 5) | (1 << 2)); //enable receive & isr
    else uart->CR1 &=~ ((1 << 5) | (1 << 2)); //disable receive interrupt  
}

uint8_t serial_receive(uart_inst_t *uart)
{
    while(!(uart->SR & (1 << 5))); //wait till data received
    return uart->DR; //read data
}

void serial_send(uart_inst_t *uart, uint8_t data)
{
    while(!(uart->SR & (1 << 7))); //wait till buffer empty 
    uart->DR = data; //load data
}

void serial_write(uart_inst_t *uart, const char *data)
{
    while(*data) serial_send(uart, *data++);
}

void serial_writeln(uart_inst_t *uart, const char *data, size_t length)
{
    while(length --> 0) serial_send(uart, *data++);
}

void serial_printf(uart_inst_t *uart, const char *fmt, ...)
{
    char *ptr = (char*)malloc(512 * sizeof(char));
    va_list args; va_start(args, fmt);
    uint16_t length = vsnprintf(ptr, 512, fmt, args);
    serial_writeln(uart, ptr, length); free(ptr);
}

void serial_decimal(uart_inst_t *uart, uint32_t data, uint8_t base)
{
    char *ptr = (char*)malloc(16 * sizeof(char));
    serial_write(uart, dtoa(data, ptr, base, 0));
    free(ptr);
}

void serial_float(uart_inst_t *uart, float data, size_t length)
{
    char *ptr = (char*)malloc(16 * sizeof(char));
    serial_write(uart, ftostra(data, ptr, length));
    free(ptr);
}

void serial_flush(uart_inst_t *uart)
{
    uint8_t uart_index_number = uartindex(uart);

    switch(uart_index_number)
    {
        case 1: memset(serialarray1, '\0', sizeof(serialarray1)); serialcount1 = 0; break;
        case 2: memset(serialarray2, '\0', sizeof(serialarray2)); serialcount2 = 0; break;
        case 3: memset(serialarray3, '\0', sizeof(serialarray3)); serialcount3 = 0; break;
    }
}

bool serial_talkback(uart_inst_t *uart, bool nl, const char *data, size_t length, uint16_t timer)
{
    uint8_t uart_index_number = uartindex(uart);
    uint8_t *buffer, *count;

    switch(uart_index_number)
    {
        case 1: buffer = &serialarray1[0]; count = &serialcount1; break;
        case 2: buffer = &serialarray2[0]; count = &serialcount2; break;
        case 3: buffer = &serialarray3[0]; count = &serialcount3; break;
    }
    if(nl) serial_write(uart, "\r\n");

    do
    {
        if(memmem(buffer, count[0], data, length) != NULL) return true;
        if(memmem(buffer, count[0], "ERROR", 5) != NULL) return false;
        delay_ms(1);
    }
    while(timer --> 0);
    return false;
}