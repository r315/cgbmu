#ifndef _debug_h_
#define _debug_h_

#include <stdio.h>
#include <stdint.h>

#define DBG_REG_COL_BASE 175
#define DBG_REG_ROW_BASE 9
#define DBG_REG_COL(x) (DBG_REG_COL_BASE + (x*8))
#define DBG_REG_ROW(y) (DBG_REG_ROW_BASE + (y*9))
void updateFps(void);
void dumpRegisters(void);

int printVal(int x, int y,char *name, int v, char radix, char digitos);
void dumpMemory(unsigned short addr, unsigned short siz);


/**
 * run cpu in debug mode
 * */
void debug(void);
#endif
