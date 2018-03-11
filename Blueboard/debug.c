#include <graphics.h>
#include "lcd.h"
#include "dmgcpu.h"
#include "debug.h"
#include <blueboard.h>
//#define GetTicks() SDL_GetTicks()

#define FPS_ROW 0
#define REGISTERS_ROW 11

volatile unsigned char _logline=0;
int fps;

void updateFps(void){
static uint32_t fpsupdatetick = 0;
    fps++;
    
	if(GetTicks() > fpsupdatetick)
	{
		printVal(175,FPS_ROW,"Fps ",fps,10,4);
		fps = 0;
		fpsupdatetick = GetTicks() + 1000;
	}
}

//----------------------------------------------------*/
//
//------------------------------------------------------
int printInt(int x, int y,unsigned int v, char radix, char digitos)
{
unsigned char i=0,c,dig[16];	
	do{
		c = (unsigned char)(v % radix);
		if (c >= 10)c += 7;		
		c += '0';
		v /= radix;
		dig[i++]=c;
	} while(v);
	
	for(c=i;c<digitos;c++)
		x= drawChar(x,y,'0');
	
	while(i--)
	x = drawChar(x,y,dig[i]);		
	return x;		
}
//----------------------------------------------------*/
//
//------------------------------------------------------
int printVal(int x, int y,char *name, int v, char radix, char digitos)
{	
	x = drawString(x,y,name);
	x = printInt(x,y,v,radix,digitos);	 
	return x;
}
//----------------------------------------------------*/
//
//------------------------------------------------------
void dumpRegisters()
{
#ifdef NO_SDL
	putchar('\n');
	printf(" A = %.2X\n",A);
	printf("BC = %.4X\n",BC);
	printf("DE = %.4X\n",DE);
	printf("HL = %.4X\n",HL);
	printf("SP = %.4X\n",SP);
	printf("PC = %.4X\n",PC);
#else
	setFcolor(YELLOW);
	//setAttribute(g_double);
//	setFont(BOLD);
	printVal(175,REGISTERS_ROW,"af ",REG_A << 8| REG_F ,16,4);
	printVal(175,REGISTERS_ROW + (9 * 1),"bc ",REG_BC,16,4);
	printVal(175,REGISTERS_ROW + (9 * 2),"de ",REG_DE,16,4);
	printVal(175,REGISTERS_ROW + (9 * 3),"hl ",REG_HL,16,4);
	printVal(175,REGISTERS_ROW + (9 * 4),"sp ",REG_SP,16,4);
	printVal(175,REGISTERS_ROW + (9 * 5),"pc ",REG_PC,16,4);
	printVal(175,REGISTERS_ROW + (9 * 6),"LCDC ",(IOLCDC & 8),16,2);
//	printVal(175,REGISTERS_ROW + (9 * 7),"P1 ",IOP1,16,4);
	printVal(175,REGISTERS_ROW + (9 * 8),"ly ",IOLY,16,4);
#endif
}
//----------------------------------------------------*/
//
//------------------------------------------------------
void dumpMemory(unsigned short addr, unsigned short siz)
{
unsigned short i,j,x;
unsigned char p = 160;
#ifdef NO_SDL
	for(i=0;i<siz;i++)
		printf("0x%.4X = 0x%.2X\n",addr+i,memoryRead(addr+i));	
#else
	setFcolor(TOMATO);
	drawString(5*8,p-8," 0 1 2 3 4 5 6 7 8 9 A C B D E F");
	setFcolor(RED);
	for(i=0; i < (siz>>4); i++)
	{
		x = printInt(0,p+(i*8),addr,16,4)+8;
		for(j=0; j < 16; j++)
	 		x = printInt(x,p+(i*9),memoryRead(addr++),16,2);	
	}
#endif	
}
//----------------------------------------------------*/
//dump stack frame
//------------------------------------------------------
void dumpSp(void)
{
unsigned short x,_sp;	
signed char i;
_sp = REG_SP;
setFcolor(PINK);
	
	for(i=7; i >-1 ; i--)
	{	
		x = printInt(0,i*8,_sp,16,4);
		x = drawChar(x,i*8,':');
		x = printInt(x,i*8,memoryRead(_sp+1),16,2);
		x = printInt(x,i*8,memoryRead(_sp),16,2);
		_sp +=2;	
	}
}
//----------------------------------------------------*/
//
//------------------------------------------------------
void DBG_Info(char* text)
{
	DISPLAY_puts(text);
	DISPLAY_putc('\n');
}

