#include "../src/utils/main.h"

volatile uint32_t absolutetime;

/**
 * @brief ISR routine for system tick handelr 1ms
 * 
 */
void SysTick_Handler(void)
{
    absolutetime++;
}

/**
 * @brief read millis from absolute time
 * 
 * @return uint64_t milliseconds
 */
uint32_t millis()
{
    return absolutetime;
}

/**
 * @brief read micros from absolute time
 * 
 * @return uint64_t microseconds 
 */
uint64_t micros()
{
    //SystemCoreClock = 72000000 / 1000 = 72000ticks per ms
    //SysTick will decrease counter 72000 to 0 
    //Systemload = 72000000 / 1000000 = 72ticks per us
    //Error calibration = 72 >> 1 = 72 / 2
    //We already know absolute time ms add with that micros 
    return (uint64_t)(((((SystemCoreClock / 1000) - SysTick->VAL) / SYSLOAD) - SYSCALIB) + (absolutetime * 1000L));
}

/**
 * @brief function reset & initialize core clock  
 * 
 */
void stdio_init_all(void)
{
    //ld script ram must protect from isr table rewrite 
    //RAM    (xrw)    : ORIGIN = 0x20000100,  LENGTH = 10K - 0xFF
    //SystemInit(); /*<! called before main !>*/

    //external HSE oscillator 8MHZ force mcu to 72mhz
    RCC->CFGR &=~ ((0b1111 << 18)); //clear pll config
    RCC->CFGR |=  ((0b0111 << 18)); //PLL * 9 = 72
    RCC->CFGR &=~ (1 << 22); //PLL / 1.5 = 48Mhz for USB
    
    RCC->CR  |= (1 << 16); //HSE oscillator on
    while(!(RCC->CR & (1 << 17))); //wait till clock ready 

    //https://www.st.com/resource/en/programming_manual/pm0075-stm32f10xxx-flash-memory-microcontrollers-stmicroelectronics.pdf
    RCC->CFGR |= (1 << 16);  //HSE as PLL source
    RCC->CR |= (1 << 24); //PLL on
    while(!(RCC->CR & (1 << 25))); //wait till pll ready 

    FLASH->ACR = ((1 << 4) | (0b010 << 0)); //enable prefetch and two wait states

    RCC->CFGR &=~ (0b11 << 0); //disable system clock 
    RCC->CFGR |=  (0b10 << 0); //PLL as system clock 
    while((RCC->CFGR & 0b1111) != 0b1010); //wait pll act as system clock 

    RCC->CFGR &=~ (0b1111 << 4); //AHB prescalar = 1 AHBCLK = 72MHZ
    RCC->CFGR &=~ (0b111 << 8);  //clear APB low
    RCC->CFGR |=  (0b100 << 8);  //APBL divider = 2 clock = 36MHZ (PCLK1 <-> APB1)
    RCC->CFGR &=~ (0b111 << 11); //APBH divider = 1 clock = 76MHZ (PCLK2 <-> APB2)
    RCC->CFGR &=~ (0b11 << 14);  //clear ADC prescalar
    RCC->CFGR |=  (0b10 << 14);  //ADC prescalar = 6 ADCCLK = 12MHZ

    SystemCoreClockUpdate(); //update system core 
    SysTick_Config(SystemCoreClock / 1000); //isr for 1ms delay 
    absolutetime = 0x0;
}

/**
 * @brief map decimal value to given range 
 * 
 * @param mv    main value
 * @param smin  start minimum 
 * @param smax  start maximum
 * @param emin  end minimum
 * @param emax  end maximum 
 * @return long mapped value 
 */
long mapdecimal(long mv, long smin, long smax, long emin, long emax)
{
	return (mv - smin) * (emax - emin) / (smax - smin) + emin;
}

/**
 * @brief map float value to given range
 * 
 * @param mv    main value
 * @param smin  start minimum
 * @param smax  start maximum 
 * @param emin  end minimum 
 * @param emax  end maximum 
 * @return float mapped value
 */
float mapfloat(float mv, float smin, float smax, float emin, float emax)
{
	return (mv - smin) * (emax - emin) / (smax - smin) + emin;
}

/**
 * @brief find out substring with given length
 * 
 * @param ms  main string 
 * @param msl length of main string 
 * @param ss  substring 
 * @param ssl length of substring 
 * @return void* result 
 */
void *memmem(const void *ms, size_t msl, const void *ss, size_t ssl)
{
  const char *cur, *lst;
  const char *cl = (char*)ms;
  const char *cs = (char*)ss;

  if((!msl || !ssl) || (msl < ssl)) return NULL;
  if(ssl == 1) return memchr(cl, *cs, msl);
  
  lst = cl + msl - ssl;
  for(cur = cl; cur <= lst; cur++)
  if(cur[0] == cs[0] && memcmp(cur, cs,ssl) == 0)
  return (void*)cur;
  
  return NULL;
}

/**
 * @brief split a char array 
 * 
 * @param ms   main string 
 * @param buf  buffer to store 
 * @param hs   heystack 
 * @param ne   needle 
 * @return char*  pointer cast buffer 
 */
char *split(char *ms, char *buf, const char *hs, uint8_t ne)
{
	char *sts = strstr((char*)ms, hs);
	char *eds = strchr((char*)sts, ne);
	if(sts == NULL || eds == NULL) return NULL;
	
	for(char *lsv = sts + strlen(hs); lsv < eds; lsv++)
	*buf++ = lsv[0]; 
	while(*buf) *buf++ = '\0';
	return (char*)buf;
}

/**
 * @brief convert decimal to ascii
 * 
 * @param data to convert
 * @param buf  to store 
 * @param base conversion base 
 * @param count require count 
 * @return char* ptr cast buffer
 * @note  null count autocalculate size 
 */
char *dtoa(long data, char *buf, uint8_t base, size_t count)
{
    unsigned char conlength = count;
    char *ptr = buf;
  
    if(conlength == 0)
    {
        signed long condata = ((data < 0) ? -data : data);
        do
        {
            condata /= base;
            conlength++;
        }
        while(condata);
    }

    if(data < 0)
    {
        data =- data;
        *ptr++ = '-';
        //conlength += 1; 
    }
  
    char *p = ptr;
  
    while(conlength)
    {
        *p++ = (data ?'0' + (data % base >= 10 ?(data % base) + 'A' - '0'- 10 :data % base) :'0');
        data /= base;
        conlength--;
    }
  
    char *p1 = p;
    while(p > ptr)
    {
        char c = *--p;
        *p = *ptr;
        *ptr++ = c;
    }
    ptr = p1;
    *p1 = 0;
    return buf;
}

/**
 * @brief convert float to string 
 * 
 * @param data to convert 
 * @param buf  to store 
 * @param count require length 
 * @return char* pointer case buffer
 * @note   dot must consider as count 
 */
char *ftostra(double data, char *buf, size_t count)
{
  char *ptr = buf;
  
  if(data < 0.0F)
  {
    data =- data;
    *ptr++ = '-';
    count--;
  }
  
  double rounding = 0.5F;
  unsigned char k = 0;  
  for(k = 0; k < count; k++)
  rounding /= 10.0F;
  data += rounding;
  
  unsigned long ipart = (unsigned long)data;
  double fpart = data - (double)ipart;
  
  if(!ipart) 
  {
    *ptr++ = '0';
    count--;
  }
  else
  {
    char *p = ptr;
    while(ipart)
    {
      *p++ = '0' + ipart % 10;
      ipart /= 10;
      count--;
    }
    
    char *p1 = p;
    while(p > ptr)
    {
      char c = *--p;
      *p = *ptr;
      *ptr++ = c;
    }
    
    ptr = p1;
  }
  
  if(count)
  {
    *ptr++ = '.';
    count--;

    while(count --> 0)
    {
      fpart *= 10.0F;
      char c = (char)fpart;
      *ptr++ = '0' + c;
      fpart -= c;
    }
  }
  
  *ptr = 0;
  return buf;
}