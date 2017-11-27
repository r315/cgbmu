

#include "lcd.h"
#include "debug.h"
#include "dmgcpu.h"
#include "graphics.h"
#include "video.h"
#include "io.h"
#include "disassembler.h"

#define GetTicks() SDL_GetTicks()
#define FPS_ROW 0
#define FRAME_TIME 16

enum {
	STEP = 1,
	CONTINUE,
	PAUSE
};

uint16_t breakpoint = 0x100;
uint8_t stepping = OFF;

void debugCommans(uint8_t *st);
void decode(void);
char readLine(char *dst, uint8_t max);
void stepInstruction(void);
void stepFrame(void);

void debug(void){	
uint8_t key;
uint32_t ticks = 0, dticks;
	setFcolor(YELLOW);
	while((key = readJoyPad()) != 255){
		

		if (stepping != OFF) {
			debugCommans(&stepping);

			if (REG_PC == breakpoint && !stepping) {
				stepping = STEP;
			}
			
			switch (stepping) {
				case STEP:
					dumpRegisters();
					disassemble();
					stepping = PAUSE;
					break;

				case PAUSE:
					if (key != J_A) {
						SDL_Delay(30);
						continue;
					}
				case CONTINUE:					
					break;
			}			
		}



#if 1
		ticks = SDL_GetTicks();
		stepFrame();
		dticks = SDL_GetTicks() - ticks;
		if (dticks < FRAME_TIME && stepping == OFF) {
			SDL_Delay(FRAME_TIME - dticks);
		}
		updateFps();
#else
		stepInstruction();				
		if (frame == ON) {
			dticks = SDL_GetTicks() - ticks;
			if (dticks < FRAME_TIME && stepping == OFF) {
				SDL_Delay(FRAME_TIME - dticks);
			}
			updateFps();
			ticks = SDL_GetTicks();
			frame = OFF;
		}
#endif	
	}
}
//----------------------------------------------------*/
//
//------------------------------------------------------
void updateFps(void){
static uint32_t fpsupdatetick = 0;
static uint16_t fps = 0;
    fps++;
    
	if(GetTicks() > fpsupdatetick)
	{
		LCD_Push();
		printVal(SCREEN_W + 8,FPS_ROW,"Fps ",fps,10,5);
		fps = 0;
		fpsupdatetick = GetTicks() + 1000;
		LCD_Pop();
	}
}
//----------------------------------------------------*/
//
//------------------------------------------------------
int printNum(int x, int y,unsigned int v, char radix, char digitos)
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
	x = printNum(x,y,v,radix,digitos);	 
	return x;
}
//----------------------------------------------------*/
//
//------------------------------------------------------
void dumpRegisters(void)
{
	LCD_Push();
#if 0
	putchar('\n');
	fprintf(stdout,"AF = %.4X\n\r",REG_A << 8| REG_F);
	printf("BC = %.4X\n\r",REG_BC);
	printf("DE = %.4X\n\r",REG_DE);
	printf("HL = %.4X\n\r",REG_HL);
	printf("SP = %.4X\n\r",REG_SP);
	printf("PC = %.4X\n\r",REG_PC);
	printf("Z | N | H | C \n\r");
	printf("--+---+---+--- \n\r");
	printf("%u | %u | %u | %u\n\r",
		(REG_F & FZ) ? 1 : 0,
		(REG_F & FN) ? 1 : 0,
		(REG_F & FH) ? 1 : 0,
		(REG_F & FC) ? 1 : 0
	);
#else	
	setFcolor(YELLOW);
	//setAttribute(g_double);
//	setFont(BOLD);
	printVal(DBG_REG_COL(0), DBG_REG_ROW(0),"af ",REG_A << 8| REG_F ,16,4);
	printVal(DBG_REG_COL(0), DBG_REG_ROW(1),"bc ",REG_BC,16,4);
	printVal(DBG_REG_COL(0), DBG_REG_ROW(2),"de ",REG_DE,16,4);
	printVal(DBG_REG_COL(0), DBG_REG_ROW(3),"hl ",REG_HL,16,4);
	printVal(DBG_REG_COL(0), DBG_REG_ROW(4),"sp ",REG_SP,16,4);
	printVal(DBG_REG_COL(0), DBG_REG_ROW(5),"pc ",REG_PC,16,4);
	printVal(DBG_REG_COL(0), DBG_REG_ROW(7),"LCDC ",IOLCDC,16,2);
	printVal(DBG_REG_COL(0), DBG_REG_ROW(8),"ly ",IOLY,16,2);

	printVal(DBG_REG_COL(0), DBG_REG_ROW(10),"TIMA ",IOTIMA,16,2);
	printVal(DBG_REG_COL(0), DBG_REG_ROW(11),"DIV ",IODIV,16,2);
	
#endif
	LCD_Pop();
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
		x = printNum(0,p+(i*8),addr,16,4)+8;
		for(j=0; j < 16; j++)
	 		x = printNum(x,p+(i*9),memoryRead(addr++),16,2);	
	}
#endif	
}
//----------------------------------------------------*/
//
//------------------------------------------------------
void printStackFrame(void)
{
unsigned short x,_sp;	
signed char i;
_sp = regs.SP;
setFcolor(PINK);
	
	for(i=7; i >-1 ; i--)
	{	
		x = printNum(0,i*8,_sp,16,4);
		x = drawChar(x,i*8,':');
		x = printNum(x,i*8,memoryRead(_sp+1),16,2);
		x = printNum(x,i*8,memoryRead(_sp),16,2);
		_sp +=2;	
	}
}
//----------------------------------------------------*/
//avalilable debug commands
//bp <addr hex>
//------------------------------------------------------
#ifdef WIN32
#include <string.h>
#include <stdlib.h>
#pragma warning(disable : 4996)
#elif defined(LINUX)
#include <string.h>
#endif
void debugCommans(uint8_t *st){
#ifdef __EMU__
	static char line[10];
	char *cmd;
	if(readLine(line, 10)){
		
		if(*line == '\0' || *line == ' ' ){ //cartridge return pressed
			*st = STEP;
			return;
		}

		cmd = strtok(line, " ");
		if(cmd == NULL) 
			return;		//empty line
		
		if(!strcmp(cmd, "bp")){
			cmd = strtok(NULL, " ");
			breakpoint = (uint16_t)strtol(cmd, &cmd, 16);
			*st = CONTINUE;
			return;
		}

		if(!strcmp(cmd, "run")){
			*st = CONTINUE; 
			return;
		}
	}
#endif
}
//----------------------------------------------------*/
//main cpu run
// decode(),
// timer(),
// video(),
// interrupts() are processed in each iteration
//------------------------------------------------------
void stepInstruction(void){
	decode();	
	timer();	
	video();
	interrupts();	
	//one instruction
}
//-----------------------------------------
//
//-----------------------------------------
extern uint16_t video_cycles;
void runCpu(uint16_t nTicks){
	while (nTicks > video_cycles){
		timer();   
		interrupts();
		decode();
		video_cycles += GET_CYCLE(); 	
    } 
	video_cycles -= nTicks; 	
}
//-----------------------------------------
//
//-----------------------------------------
void stepFrame(void){

	LCD_Window(0, 0, SCREEN_W, SCREEN_H);

	IOSTAT &= ~(V_MODE_MASK);
	for (IOLY = 0; IOLY < SCREEN_H; IOLY++){
	    
		IOSTAT = (IOLY == IOLYC) ? (IOSTAT | LYC_LY_FLAG) : (IOSTAT &= ~LYC_LY_FLAG);

		if (IOSTAT & LYC_LY_IE) 
			IOIF |= STAT_IF;
		
		IOSTAT |= V_M2;  			// Change to Mode2 scan OAM
		if(IOSTAT & OAM_IE)			// check OAM IE
			IOIF |= STAT_IF;		
		runCpu(V_M2_CYCLE);
		scanOAM();
	    
		IOSTAT |= V_M3;  			// Change to Mode3 scan VRAM
	    runCpu(V_M3_CYCLE);
	    scanline();

	    IOSTAT &= ~(V_MODE_MASK); 	// Change to Mode0 H-Blank
	   	if(IOSTAT & HB_IE)			// check H-Blank IE
			IOIF |= STAT_IF;
	    runCpu(V_M0_CYCLE);		
	}	
	
	IOSTAT |= V_M1;  		// Change to Mode 1
	IOIF |= V_BLANK_IF;		// V-Blank Flag is Always activated
	if(IOSTAT & VB_IE)		// LCD Flag is activated if IE is enabled
		IOIF |= STAT_IF;	

	while(IOLY < (SCREEN_H + VBLANK_LINES)){
		IOSTAT = (IOLY == IOLYC)? (IOSTAT | LYC_LY_FLAG) : (IOSTAT & ~LYC_LY_FLAG); 				
		runCpu(V_LINE_CYCLE);	
		IOLY++;		
	}					
	 
	// end of frame
}




