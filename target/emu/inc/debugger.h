#ifndef _debugger_h_
#define _debugger_h_

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


#define TEXT_COL(X)			(X*8 * 8 + 161)
#define TEXT_ROW(Y)			(Y*9)
#define TEXT_COL1			TEXT_COL(0)
#define TEXT_COL2			TEXT_COL(1)

#define TEXT_ROW1			TEXT_ROW(0)
#define TEXT_ROW2			TEXT_ROW(1)
#define TEXT_ROW3			TEXT_ROW(2)
#define TEXT_ROW4			TEXT_ROW(3)
#define TEXT_ROW5			TEXT_ROW(4)
#define TEXT_ROW6			TEXT_ROW(5)
#define TEXT_ROW7			TEXT_ROW(6)
#define TEXT_ROW8			TEXT_ROW(7)
#define TEXT_ROW9			TEXT_ROW(8)
#define TEXT_ROW10			TEXT_ROW(9)
#define TEXT_ROW11			TEXT_ROW(10)
#define TEXT_ROW12			TEXT_ROW(11)
#define TEXT_ROW13			TEXT_ROW(12)
#define TEXT_ROW14			TEXT_ROW(13)
#define TEXT_ROW15			TEXT_ROW(14)
#define TEXT_ROW16			TEXT_ROW(15)
#define TEXT_ROW17			TEXT_ROW(16)
#define TEXT_ROW18			TEXT_ROW(17)

void DBG_run();
void DBG_Fps(void);
void DBG_DumpRegisters(void);
void DBG_DumpMemory(unsigned short addr, unsigned short siz);
void DBG_DumpStackFrame(void);
void DBG_Info(char* text);
#endif
