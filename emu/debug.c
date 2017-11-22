

#include "lcd.h"
#include "debug.h"
#include "dmgcpu.h"
#include "graphics.h"
#include "video.h"
#include "io.h"
#include "disassembler.h"

#define GetTicks() SDL_GetTicks()
#define FPS_ROW 0
#define REGISTERS_ROW 11
#define FRAME_TIME 16

uint16_t breakpoint = 0x100;

void decode(void);
char readLine(char *dst, uint8_t max);
void step(uint8_t key);
void stepFast(uint8_t key);

void debug(void){	
uint8_t key;
uint32_t ticks, dticks;
	setFcolor(YELLOW);
	while((key = readJoyPad()) != 255){
		ticks = SDL_GetTicks();
		//step(key);				
		stepFast(key);
		dticks = SDL_GetTicks() - ticks;
		if(dticks < FRAME_TIME)
			SDL_Delay(FRAME_TIME - dticks);
		updateFps();
	}
}
//----------------------------------------------------*/
//
//------------------------------------------------------
void updateFps(void){
static uint32_t fpsupdatetick = 0;
static int fps = 0;
    fps++;
    
	if(GetTicks() > fpsupdatetick)
	{
		printVal(SCREEN_W + 8,FPS_ROW,"Fps ",fps,10,4);
		fps = 0;
		fpsupdatetick = GetTicks() + 1000;
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
	printVal(175,0,"af ",REG_A << 8| REG_F ,16,4);
	printVal(175,9,"bc ",REG_BC,16,4);
	printVal(175,18,"de ",REG_DE,16,4);
	printVal(175,27,"hl ",REG_HL,16,4);
	printVal(175,36,"sp ",REG_SP,16,4);
	printVal(175,45,"pc ",REG_PC,16,4);
	printVal(175,64,"LCDC ",(IOLCDC & 8),16,2);
	printVal(175,84,"ly ",IOLY,16,4);

	printVal(170,94,"TIMA ",IOTIMA,16,2);
	printVal(170,104,"DIV ",IODIV,16,2);
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
			*st = 3;	   //step
			return;
		}

		cmd = strtok(line, " ");
		if(cmd == NULL) 
			return;		//empty line
		
		if(!strcmp(cmd, "bp")){
			cmd = strtok(NULL, " ");
			breakpoint = (uint16_t)strtol(cmd, &cmd, 16);
			*st = 0;	//resume
		}

		if(!strcmp(cmd, "run")){
			*st = 0;	 //resume 
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
void step(uint8_t key){
static uint8_t stepping = 0;
	
	debugCommans(&stepping);
	
	if( REG_PC == breakpoint && !stepping){
		stepping = 2;
	}

	if(stepping){
		if(stepping == 2){
			dumpRegisters();
			disassemble();	        
			stepping = 1;
			return;
		}

		if(key != J_A && stepping == 1){
			SDL_Delay(30);			
			return;
		}			
		else{
			stepping = 2;
		}
	}	
	decode();	
	timer();	
	video();
	interrupts();	        
}
//-----------------------------------------
//
//-----------------------------------------
void runCpu(int nTicks){
	while (nTicks > 0){
		interrupts();
		decode();
		timer();   
		nTicks -= GET_CYCLE(); 	
    }  
}
//-----------------------------------------
//
//-----------------------------------------
void stepFast(uint8_t key){

	LCD_Window(0, 0, SCREEN_W, SCREEN_H);

	for (IOLY = 0; IOLY < SCREEN_H; IOLY++){

		IOSTAT |= V_M2;  	// scan OAM
		if(IOSTAT & OAM_IE)	// check OAM IE
			IOIF |= STAT_IF;
		if(IOLY == IOLYC)	// check coincedence	
			IOSTAT |= LYC_LY_FLAG; 
		else
			IOSTAT &= ~LYC_LY_FLAG;			
	    runCpu(V_M2_CYCLE);
		scanOAM();

	    IOSTAT |= V_M3;  	// scan VRAM
	    runCpu(V_M3_CYCLE);
	    scanline();
    
	    IOSTAT &= ~(V_M3); 	// H-Blank
	   	if(IOSTAT & HB_IE)	
			IOIF |= STAT_IF;
	    runCpu(V_M0_CYCLE);
	}	
	
	IOSTAT |= V_M1;  		// Change to Mode 1
	IOIF |= V_BLANK_IF;		// V-Blank Flag is Always activated

	if(IOSTAT & VB_IE)		// LCD Flag is activated if IE is enabled
		IOIF |= STAT_IF;	
						
	while(IOLY < (SCREEN_H + VBLANK_LINES)){
		lycIrq();
		runCpu(V_LINE_CYCLE);	
		IOLY++;
	}
	// end of frame
}




