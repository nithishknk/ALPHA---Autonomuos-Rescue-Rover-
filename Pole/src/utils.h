#ifndef _UTILS_H
#define _UTILS_H

#include <util/delay.h>

#define F_CPU             16000000UL
#define onemachinecycle   0.0625F
#define machinecycleus    16

#define delay_us(delay)\
{\
  unsigned int reqdelay = delay;\
  while(--reqdelay) _delay_us(0.69);\
}

#define delay_ms(delay)\
{\
  unsigned char routinedelay;\
  unsigned int requiredelay = delay;\
  do\
  {\
    routinedelay = 4;\
    do\
    {\
      _delay_us(249);\
    }\
    while(--routinedelay);\
  }\
  while(--requiredelay);\
}

#define BIN 2
#define OCT 8
#define DEC 10
#define HEX 16

#define enable  true
#define disable false
#define null    ((void *)0)

extern long mapvalue(long value, long startmin, long startmax,long endmin, long endmax);
extern char *smemchr(register const char *mainstring, int substring,int length);
extern char *smemrchr(register const char *mainstring, int substring, int length);
extern void *smemmem(const void *mainstring, int lengthofmainstring, const void *substring, int lengthofsubstring);
extern char *dtoa(signed long data, char *buf, unsigned char base, unsigned char *count);
extern char *ftostra(double data, char *buf, unsigned char count);
extern char splitstring(unsigned char *mainstring, unsigned char *array,const char *startofstring, unsigned char endofstring);
#endif
