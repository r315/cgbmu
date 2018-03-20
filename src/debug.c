
#include <common.h>
#include <string.h>
#include <stdlib.h>
#include "lcd.h"
#include "debug.h"
#include "dmgcpu.h"
#include "display.h"
#include "video.h"
#include "io.h"

#define FPS_ROW 0


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

#if defined(__EMU__)
void debug(void){	
uint8_t key;
uint32_t ticks = 0, dticks;
	DISPLAY_SetFcolor(YELLOW);
	while((key = readJoyPad()) != 255){		

		if (stepping != OFF) {
			//debugCommans(&stepping);

			if (REG_PC == breakpoint && !stepping) {
				stepping = STEP;
			}
			
			switch (stepping) {
				case STEP:
					DBG_Reg();
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
		DBG_Fps();
#else
		stepInstruction();				
		if (frame == ON) {
			dticks = SDL_GetTicks() - ticks;
			if (dticks < FRAME_TIME && stepping == OFF) {
				SDL_Delay(FRAME_TIME - dticks);
			}
			DBG_Fps();
			ticks = SDL_GetTicks();
			frame = OFF;
		}
#endif	
	}
}
#endif
//----------------------------------------------------*/
//
//------------------------------------------------------
void DBG_Fps(void){
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
		x= DISPLAY_Char(x,y,'0');
	
	while(i--)
	x = DISPLAY_Char(x,y,dig[i]);		
	return x;		
}
//----------------------------------------------------*/
//
//------------------------------------------------------
int printVal(int x, int y,char *name, int v, char radix, char digitos)
{	
	x = DISPLAY_Text(x,y,name);
	x = printInt(x,y,v,radix,digitos);	 
	return x;
}
//----------------------------------------------------*/
//
//------------------------------------------------------
void DBG_Reg(void)
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
	DISPLAY_SetFcolor(YELLOW);
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
	
	printVal(DBG_REG_COL(0), DBG_REG_ROW(13), "SCX ", IOSCX, 16, 2);
	printVal(DBG_REG_COL(0), DBG_REG_ROW(14), "SCY ", IOSCY, 16, 2);
	
#endif
	LCD_Pop();
}
//----------------------------------------------------*/
//
//------------------------------------------------------
void DBG_Mem(unsigned short addr, unsigned short siz)
{
unsigned short i,j,x;
unsigned char p = 160;
#ifdef NO_SDL
	for(i=0;i<siz;i++)
		printf("0x%.4X = 0x%.2X\n",addr+i,memoryRead(addr+i));	
#else
	DISPLAY_SetFcolor(TOMATO);
	DISPLAY_Text(5*8, p-8, " 0 1 2 3 4 5 6 7 8 9 A C B D E F");
	DISPLAY_SetFcolor(RED);
	for(i=0; i < (siz>>4); i++)
	{
		x = printInt(0,p+(i*8),addr,16,4)+8;
		for(j=0; j < 16; j++)
	 		x = printInt(x,p+(i*9),memoryRead(addr++),16,2);	
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
	DISPLAY_SetFcolor(PINK);
	
	for(i=7; i >-1 ; i--)
	{	
		x = printInt(0,i*8,_sp,16,4);
		x = DISPLAY_Char(x,i*8,':');
		x = printInt(x,i*8,memoryRead(_sp+1),16,2);
		x = printInt(x,i*8,memoryRead(_sp),16,2);
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

		cmd = (char*)strtok(line, " ");
		if(cmd == NULL) 
			return;		//empty line
		
		if(!strcmp(cmd, "bp")){
			cmd = (char*)strtok(NULL, " ");
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
	    
		IOSTAT = (IOLY == IOLYC) ? (IOSTAT | LYC_LY_FLAG) : (IOSTAT & (~LYC_LY_FLAG));

		if (IOSTAT & (LYC_LY_IE | LYC_LY_FLAG)) 
			IOIF |= LCDC_IF;
		
		IOSTAT |= V_M2;  			// Change to Mode2 scan OAM
		if(IOSTAT & OAM_IE)			// check OAM IE
			IOIF |= LCDC_IF;		
		runCpu(V_M2_CYCLE);
		scanOAM();
	    
		IOSTAT |= V_M3;  			// Change to Mode3 scan VRAM
	    runCpu(V_M3_CYCLE);
		scanline();

	    IOSTAT &= ~(V_MODE_MASK); 	// Change to Mode0 H-Blank
	   	if(IOSTAT & HB_IE)			// check H-Blank IE
			IOIF |= LCDC_IF;
	    runCpu(V_M0_CYCLE);		
	}	
	
	IOSTAT |= V_M1;  		// Change to Mode 1
	IOIF |= V_BLANK_IF;		// V-Blank Flag is Always activated
	if(IOSTAT & VB_IE)		// LCD Flag is activated if IE is enabled
		IOIF |= LCDC_IF;	

	while(IOLY < (SCREEN_H + VBLANK_LINES)){
		IOSTAT = (IOLY == IOLYC)? (IOSTAT | LYC_LY_FLAG) : (IOSTAT & ~LYC_LY_FLAG); 				
		runCpu(V_LINE_CYCLE);	
		IOLY++;		
	}					
	//DBG_BGmap();
	// end of frame
}

//----------------------------------------------------*/
//
//------------------------------------------------------
void DBG_Info(char* text)
{
	DISPLAY_puts(text);
	DISPLAY_putc('\n');
}

extern const unsigned short lcd_pal[];
void DBG_DrawTileLine(uint8_t msb, uint8_t lsb) {
	uint8_t i, color;
	for (i = 0; i < 8; i++) {
		color = (msb & 0x80) ? 2 : 0;
		color |= (lsb & 0x80) ? 1 : 0;
		color = (IOBGP >> (color << 1));
		LCD_Data(lcd_pal[color & 3]);
		msb <<= 1;
		lsb <<= 1;
	}
}

void DBG_DrawTile(uint8_t x, uint8_t y, TileData *td) {
	uint8_t i;
	LCD_Window(x * 8, y * 8, 8, 8);
	for(i = 0; i < 8; i++) {
		DBG_DrawTileLine(td->line[i].msb, td->line[i].lsb);
	} 
}

void DBG_BGmap(void) {
	uint8_t w, h;
	uint8_t *bgmapbase;
	TileData *td;
	uint8_t offset;

	bgmapbase = (uint8_t*)(vram + ((IOLCDC & BG_MAP) ? TILE_MAP1_BASE : TILE_MAP0_BASE));

	for (h = 0; h < 32; h++) {
		for (w = 0; w < 32; w++) {
			offset = *(bgmapbase + w + (h * 32));

			if (IOLCDC & BG_W_DATA) {
				td = (TileData*)(vram) + offset;
			}
			else {
				td = (TileData*)(vram + TILE_DATA1_SIGNED_BASE) + (int8_t)(offset);
			}
			DBG_DrawTile(w,h,td);
		}
	}
}


void DBG_PrintValue(uint8_t line, char *label, uint8_t val) {
	LCD_Push();
	DISPLAY_SetFcolor(YELLOW);
	printVal(DBG_REG_COL(0), DBG_REG_ROW(line), label, val, 16, 2);
	LCD_Pop();
}
