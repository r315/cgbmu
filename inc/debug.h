#ifndef _debug_h_
#define _debug_h_

#include <stdio.h>
#include <stdint.h>

#define DBG_REG_COL_BASE 175
#define DBG_REG_ROW_BASE 12
#define DBG_REG_COL(x) (DBG_REG_COL_BASE + (x*8))
#define DBG_REG_ROW(y) (DBG_REG_ROW_BASE + (y*9))

void DBG_Fps(void);
void DBG_DumpRegisters(void);
void DBG_Mem(unsigned short addr, unsigned short siz);
void DBG_Info(char* text);

void DBG_BGmap(void);		// 32x32 BG Tile map, display must be 256x256px

int DBG_printVal(int x, int y,char *name, int v, char radix, char digitos);


/**
 * run cpu in debug mode
 * */
void DBG_run(void);

/**

 **/
void DBG_PrintValue(uint8_t line, char *label, uint8_t val);
#endif
