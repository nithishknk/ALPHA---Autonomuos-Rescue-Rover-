#include <avr/io.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "utils.h"

long mapvalue(long mainvalue, long startmin, long startmax, long endmin, long endmax)
{
  return (mainvalue - startmin) * (endmax - endmin) / (startmax - startmin) + endmin;
}

char *smemchr(register const char *mainstring, int substring, int length)
{
  const unsigned char *localpointer = (const unsigned char*)mainstring;
  while(length --> 0)
  if(*localpointer == substring) return (char*)localpointer;
  else localpointer++;
  return '\0';
} 

char *smemrchr(register const char *mainstring, int substring, int length)
{
  char *localpointer = (char*)mainstring;
  while(length --> 0)
  if(mainstring[length] == substring) 
  return (char*)localpointer + length;
  return '\0';
} 

void *smemmem(const void *mainstring, int lengthofmainstring, const void *substring, int lengthofsubstring)
{
  const char *cur, *last;
  const char *cl = (char*)mainstring;
  const char *cs = (char*)substring;
  
  if((!lengthofmainstring && !lengthofsubstring) ||
  (lengthofmainstring < lengthofsubstring)) return '\0';
  
  if(lengthofsubstring == 1) return (void*)smemchr((char*)mainstring, 
  *cs, lengthofmainstring); 
  
  last = cl + lengthofmainstring - lengthofsubstring;
  for(cur = cl; cur <= last; cur++)
  if(cur[0] == cs[0] && memcmp(cur, cs, lengthofsubstring) == 0)
  return (void*)cur;
  
  return '\0';
} 


char *dtoa(signed long data, char *buf, unsigned char base, unsigned char *count)
{
  unsigned char conlength = count[0];
  char *ptr = buf;
  
  if(data < 0)
  {
    data =- data;
    *ptr++ = '-';
    conlength = (conlength ?conlength-- :conlength++);
  }
  
  if(!conlength)
  {
    signed long condata = data;
    do
    {
      condata /= base;
      conlength++;
    }
    while(condata);
  }
  
  *count = conlength;
  char *p = ptr;
  while(conlength)
  {
    *p++ = (data ?'0' + data % base :'0');
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
  *ptr = 0;
  return buf;
}


char *ftostra(double data, char *buf, unsigned char count)
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


char splitstring(unsigned char *mainstring, unsigned char *array, const char *startofstring, unsigned char endofstring)
{
  unsigned char *sts = strstr(mainstring, startofstring);
  unsigned char *eds = strchr(sts, endofstring);
  
  if(sts == '\0' || eds == '\0') return false;
  
  for(unsigned char *lsv = sts + strlen(startofstring);
  lsv < eds; lsv++)
  *array++ = (isdigit(lsv[0]) || lsv[0] == '.' ?lsv[0] :'\0');
  
  while(*array) *array++ = '\0';
  return true;
} 
