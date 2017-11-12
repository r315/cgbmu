#ifndef _debug_h_
#define _debug_h_

#include <stdio.h>
#include <stdint.h>

#if 1
#define DEBUG_PRINT(...) do{ printf( __VA_ARGS__ ); } while( 0 )
char *DBG_RegName(uint8_t r); 
#else
#define DEBUG_PRINT(...) do{ } while ( false )
#endif


void updateFps(void);
void dumpRegisters(void);

int printVal(int x, int y,char *name, int v, char radix, char digitos);
void dumpMemory(unsigned short addr, unsigned short siz);
void logInfo(char* text);

/**
 * run cpu in debug mode
 * */
void debug(void);
#endif
