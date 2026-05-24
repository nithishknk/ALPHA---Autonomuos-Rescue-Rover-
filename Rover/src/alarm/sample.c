#include "../src/alarm/main.h"

/*
example function for handling isr timer 

void alarm_for_servo()
{
    TIM_TypeDef *timer = getregister(timeralarm);
    if(timer->SR & (1 << 0))
    {
        // Your code 
        timer->SR &=~ (1 << 0);
    }
}
*/

uint8_t add_repeating_alarm_us(uint32_t firing)
{
    uint8_t alarmid = getalarm(); //get unclaim alarm 
    if(!alarmid) return alarmid; //we don't have spare timer 

    /* (ARR - 1) = FCLK * TOUT(s) / (PSC + 1) */
    uint32_t autoreload, prescalar; 
    for(prescalar = 0; prescalar < __UINT16_MAX__; prescalar++)
    {
        autoreload = ((unsigned)(SystemCoreClock * (firing us)) / (prescalar + 1)) - 1;
        if(autoreload && autoreload < __UINT16_MAX__) break;
    }
    if(prescalar >= __UINT16_MAX__) return 0; //can't update register for given time 

    //access timer internal registers
    TIM_TypeDef *timer = getregister(alarmid);
    switch(alarmid) //reset & enable timer clock 
    {
        case 1: 
        RCC->APB2ENR  |=  (1 << 11); //enable timer1 clock
        RCC->APB2RSTR |=  (1 << 11); //reset timer1 if incase
        RCC->APB2RSTR &=~ (1 << 11); //pullback timer1 
        break;

        case 2: 
        RCC->APB1ENR  |=  (1 << 0); //enable timer2 clock  
        RCC->APB2RSTR |=  (1 << 0); //reset timer2 if incase
        RCC->APB2RSTR &=~ (1 << 0); //pullback timer2
        break;

        case 3: 
        RCC->APB1ENR  |=  (1 << 1); //enable timer3 clock
        RCC->APB2RSTR |=  (1 << 1); //reset timer3 if incase
        RCC->APB2RSTR &=~ (1 << 1); //pullback timer 3
        break;
    }

    //add prescale & reload timer value 
    timer->CR1 = 0; //reset timer module 
    timer->PSC = prescalar; //load calculated prescale value 
    timer->ARR = autoreload; //load ARR value

    return alarmid; //alarmid for start/start/restart/reset 
}

uint8_t add_repeating_alarm_ms(uint32_t firing)
{
    uint8_t alarmid = getalarm(); //get unclaim alarm 
    if(!alarmid) return alarmid; //we don't have spare timer 

    /* (ARR - 1) = FCLK * TOUT(s) / (PSC + 1) */
    uint32_t autoreload, prescalar; 
    for(prescalar = 0; prescalar < __UINT16_MAX__; prescalar++)
    {
        autoreload = ((unsigned)(SystemCoreClock * (firing ms)) / (prescalar + 1)) - 1;
        if(autoreload && autoreload < __UINT16_MAX__) break;
    }
    if(prescalar >= __UINT16_MAX__) return 0; //can't update register for given time 

    //access timer internal registers
    TIM_TypeDef *timer = getregister(alarmid);
    switch(alarmid) //reset & enable timer clock 
    {
        case 1: 
        RCC->APB2ENR  |=  (1 << 11); //enable timer1 clock
        RCC->APB2RSTR |=  (1 << 11); //reset timer1 if incase
        RCC->APB2RSTR &=~ (1 << 11); //pullback timer1 
        break;

        case 2: 
        RCC->APB1ENR  |=  (1 << 0); //enable timer2 clock  
        RCC->APB2RSTR |=  (1 << 0); //reset timer2 if incase
        RCC->APB2RSTR &=~ (1 << 0); //pullback timer2
        break;

        case 3: 
        RCC->APB1ENR  |=  (1 << 1); //enable timer3 clock
        RCC->APB2RSTR |=  (1 << 1); //reset timer3 if incase
        RCC->APB2RSTR &=~ (1 << 1); //pullback timer 3
        break;
    }

    //add prescale & reload timer value 
    timer->CR1 = 0; //reset timer module 
    timer->PSC = prescalar; //load calculated prescale value 
    timer->ARR = autoreload; //load ARR value

    return alarmid; //alarmid for start/start/restart/reset 
}

void reset_repeating_alarm(uint8_t alarmid)
{
    TIM_TypeDef *timer = getregister(alarmid);
    __disable_irq(); timer->CNT = 0; __enable_irq();
}

void start_repeating_alarm(uint8_t alarmid, void *fun)
{
    TIM_TypeDef *timer = getregister(alarmid);
    reset_repeating_alarm(alarmid); //reset alarm before start
    switch(alarmid) 
    {
        case 1:
        NVIC_SetVector(TIM1_UP_IRQn, (uint32_t)fun); //remap isr call
        timer->EGR  |= (1 << 0); //update values into timer 
        timer->DIER |= (1 << 0); //enable update interrupt
        timer->CR1  |= (1 << 0); //enable timer counter
        NVIC_EnableIRQ(TIM1_UP_IRQn);
        break;

        case 2:
        NVIC_SetVector(TIM2_IRQn, (uint32_t)fun); //remap isr call
        timer->EGR  |= (1 << 0); //update values into timer 
        timer->DIER |= (1 << 0); //enable update interrupt
        timer->CR1  |= (1 << 0); //enable timer counter
        NVIC_EnableIRQ(TIM2_IRQn);
        break;

        case 3:
        NVIC_SetVector(TIM3_IRQn, (uint32_t)fun); //remap isr call
        timer->EGR  |= (1 << 0); //update values into timer 
        timer->DIER |= (1 << 0); //enable update interrupt
        timer->CR1  |= (1 << 0); //enable timer counter
        NVIC_EnableIRQ(TIM3_IRQn);
        break; 
    }
}

void stop_repeating_alarm(uint8_t alarmid)
{
    TIM_TypeDef *timer = getregister(alarmid);
    __disable_irq();
    switch(alarmid) 
    {
        case 1: 
        timer->CR1  &=~ (1 << 0); //disable timer counter
        timer->DIER &=~ (1 << 0); //disable update interrupt
        NVIC_DisableIRQ(TIM1_UP_IRQn); //disable isr vector
        break;

        case 2:
        timer->CR1  &=~ (1 << 0); //disable timer counter
        timer->DIER &=~ (1 << 0); //disable update interrupt
        NVIC_DisableIRQ(TIM2_IRQn);  //disable isr vector
        break;

        case 3: 
        timer->CR1  &=~ (1 << 0); //disable timer counter
        timer->DIER &=~ (1 << 0); //disable update interrupt
        NVIC_DisableIRQ(TIM3_IRQn); //disable isr vector 
        break;
    }
    __enable_irq();
}

void remove_repeating_alarm(uint8_t alarmid)
{
    stop_repeating_alarm(alarmid);
    switch(alarmid) //disable appropiar clock & reset timer 
    {
        case 1: RCC->APB2ENR &=~ (1 << 11); RCC->APB2RSTR |= (1 << 11); break; 
        case 2: RCC->APB1ENR &=~ (1 << 0);  RCC->APB1RSTR |= (1 << 0);  break;
        case 3: RCC->APB1ENR &=~ (1 << 1);  RCC->APB1RSTR |= (1 << 1);  break;
        default: break;
    }
    reset_repeating_alarm(alarmid);
}