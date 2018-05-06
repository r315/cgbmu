#include <stdint.h>
#include <inttypes.h>

int rand(void)
{


}

char strcpy(char *dst, char *src)
{
char *p = src;
  while(*p)  
    *dst++ = *p++;  
return p - src;
}

void memset(char *ptr, char value, uint32_t num)
{
  while(num--)
  {
    *ptr++ = value;
  }
}

void memcpy(char *dst, char *src, char num)
{
  while(num--)
  {
    *dst++ = *src++;    
   }
}

