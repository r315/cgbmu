#ifndef _debug_h_
#define _debug_h_

#include <stdio.h>
#include <stdint.h>
#include <display.h>

#define DBG_REG_COL_BASE 175
#define DBG_REG_ROW_BASE 9
#define DBG_REG_COL(x) (DBG_REG_COL_BASE + (x*8))
#define DBG_REG_ROW(y) (DBG_REG_ROW_BASE + (y*9))

void DBG_Fps(void);
void DBG_Reg(void);
void DBG_Mem(unsigned short addr, unsigned short siz);
void DBG_Info(char* text);

void DBG_BGmap(void);		// 32x32 BG Tile map, display must be 256x256px

int printVal(int x, int y,char *name, int v, char radix, char digitos);


/**
 * run cpu in debug mode
 * */
void debug(void);

/**

 **/
void DBG_PrintValue(uint8_t line, char *label, uint8_t val);
#endif
