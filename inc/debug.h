#ifndef _debug_h_
#define _debug_h_

#include <stdio.h>
#include <stdint.h>

#define RUN_FLAG_DEBUG	(1<<0)
#define RUN_FLAG_TEST	(1<<1)
#define RUN_FLAG_MODE	(1<<2)
#define RUN_FLAG_FILE	(1<<3)

#define DBG_REG_COL_BASE 160
#define DBG_REG_ROW_BASE 12
#define DBG_REG_COL(x) (DBG_REG_COL_BASE + (x*8))
#define DBG_REG_ROW(y) (DBG_REG_ROW_BASE + (y*9))

#define DBG_TEXT_POS(_X,_Y) (_X*8 + 200), (_Y*8 + 0)

#define DBG_TEXT_LINE1		DBG_TEXT_POS(0, 1)
#define DBG_TEXT_LINE2		DBG_TEXT_POS(0, 2)
#define DBG_TEXT_LINE3		DBG_TEXT_POS(0, 3)
#define DBG_TEXT_LINE4		DBG_TEXT_POS(0, 4)
#define DBG_TEXT_LINE5		DBG_TEXT_POS(0, 5)
#define DBG_TEXT_LINE6		DBG_TEXT_POS(0, 6)
#define DBG_TEXT_LINE7		DBG_TEXT_POS(0, 7)
#define DBG_TEXT_LINE8		DBG_TEXT_POS(0, 8)
#define DBG_TEXT_LINE9		DBG_TEXT_POS(0, 9)
#define DBG_TEXT_LINE10		DBG_TEXT_POS(0, 10)
#define DBG_TEXT_LINE11		DBG_TEXT_POS(0, 11)
#define DBG_TEXT_LINE12		DBG_TEXT_POS(0, 12)
#define DBG_TEXT_LINE13		DBG_TEXT_POS(0, 13)
#define DBG_TEXT_LINE14		DBG_TEXT_POS(0, 14)

void DBG_Fps(void);
void DBG_DumpRegisters(void);
void DBG_Mem(unsigned short addr, unsigned short siz);
void DBG_Info(char* text);
void DBG_BGmap(void);		// 32x32 BG Tile map, display must be 256x256px
int  DBG_printVal(int x, int y,char *name, int v, char radix, char digitos);
void DBG_run();
void DBG_SingleStep(void);
void DBG_PrintValue(uint8_t line, char *label, uint8_t val);
#endif
