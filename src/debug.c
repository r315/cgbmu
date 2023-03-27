
#include <cgbmu.h>
#include <string.h>
#include <stdlib.h>
#include "board.h"
#include "debug.h"
#include "dmgcpu.h"
#include "video.h"
#include "liblcd.h"
#include "lib2d.h"

#if defined(_WIN32) || defined(linux)
	#include "disassembler.h"
	#define FRAME_TIME 16

	#define BREAK_CONDITION(_cond_) if((_cond_)){ printf("break hit\n"); dbg_state = DBG_STEP; break; } 
#endif

enum debug_state{
	DBG_RUNNING = 0,
	DBG_STEP,
	DBG_PAUSE,
	DBG_SINGLE,
	DBG_CONTINUE,
};

uint16_t breakpoint = 0x100;

void debugCommans(uint8_t *st);
void decode(void);
char readLine(char *dst, uint8_t max);
void stepInstruction(void);
void runOneFrame(void);
uint8_t readButtons(void);
void disassemble(void);
void disassembleHeader(void);

#if defined(_WIN32) || defined(linux)

void DBG_SingleStep(void) {
	// execute one instruction
	decode();
	timer();
	video();
	interrupts();
}

void DBG_run(void){	
	uint8_t key, skey;
	uint32_t ticks = 0, dticks;
	uint8_t dbg_state = DBG_RUNNING;

	LIB2D_SetFcolor(LCD_YELLOW); 
	initCpu();

	disassembleHeader();

	DBG_DumpRegisters();

	while((key = readButtons()) != 255){			
		switch (dbg_state) {
		case DBG_STEP:
			dbg_state = DBG_PAUSE;
			disassemble();
			DBG_DumpRegisters();
			continue;

		case DBG_PAUSE:
			if (key == J_A || key == J_B) {
				dbg_state = DBG_CONTINUE;
				skey = key;
				break;
			}
			skey = 0;
			SDL_Delay(30);
			continue;

		case DBG_CONTINUE:
			if (skey & J_A && key != J_A) {
				dbg_state = DBG_SINGLE;
				break;
			}

			if (skey & J_B && key != J_B) {
				dbg_state = DBG_RUNNING;
				break;
			}

			SDL_Delay(30);
			continue;

		case DBG_SINGLE:
			DBG_SingleStep();
			dbg_state = DBG_STEP;
			break;

		case DBG_RUNNING:
			// Place here breakpoints
			BREAK_CONDITION(key == J_A);			
			//BREAK_CONDITION(memoryRead(REG_PC) == 0xD9);
			//BREAK_CONDITION(memoryRead(REG_PC) == 0xFB);
			//BREAK_CONDITION(halt_state == HALT_ACTIVE);

#if 0
			DBG_SingleStep();
#else
			decode();
			timer();
			if (video()) { 
				dticks = SDL_GetTicks() - ticks;
				if (dticks < FRAME_TIME) {
					SDL_Delay(FRAME_TIME - dticks);
				}
				DBG_Fps(); 
				ticks = SDL_GetTicks();
			}
			interrupts();
#endif
			break;

		default:
			break;
		}
	}
}
#endif
//----------------------------------------------------*/
//Note FPS is double of the frequency of a pin toggle
//------------------------------------------------------
void DBG_Fps(void){
static uint32_t fpsupdatetick = 0;
static uint16_t fps = 0;
    fps++;
    
	if(GetTick() > fpsupdatetick)
	{
		DBG_printVal(DBG_TEXT_POS(0,0), "Fps ",fps,10,5);
		fps = 0;
		fpsupdatetick = GetTick() + 1000;
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
		x = LIB2D_Char(x,y,'0');
	
	while(i--)
	x = LIB2D_Char(x,y,dig[i]);		
	return x;		
}
//----------------------------------------------------*/
//
//------------------------------------------------------
int DBG_printVal(int x, int y,char *name, int v, char radix, char digitos)
{	
	LIB2D_SetFcolor(LCD_RED);
	x = LIB2D_Text(x,y, name);
	LIB2D_SetFcolor(LCD_YELLOW);
	x = printInt(x,y,v,radix,digitos);	 
	return x;
}
//----------------------------------------------------*/
//
//------------------------------------------------------
void DBG_DumpRegisters(void)
{
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
	
	//setAttribute(g_double);
//	setFont(BOLD);
	DBG_printVal(DBG_TEXT_LINE1, "af: ",REG_A << 8| REG_F ,16,4);
	DBG_printVal(DBG_TEXT_LINE2,"bc: ",REG_BC,16,4);
	DBG_printVal(DBG_TEXT_LINE3,"de: ",REG_DE,16,4);
	DBG_printVal(DBG_TEXT_LINE4,"hl: ",REG_HL,16,4);
	DBG_printVal(DBG_TEXT_LINE5,"sp: ",REG_SP,16,4);
	DBG_printVal(DBG_TEXT_LINE6,"pc: ",REG_PC,16,4);
	DBG_printVal(DBG_TEXT_LINE7,"LCDC: ",IOLCDC,16,2);
	DBG_printVal(DBG_TEXT_LINE8,"STAT: ", IOSTAT, 16, 2);
	DBG_printVal(DBG_TEXT_LINE9,"ly:   ",IOLY,16,2);

	DBG_printVal(DBG_TEXT_LINE10,"TIMA: ",IOTIMA,16,2);
	DBG_printVal(DBG_TEXT_LINE11,"DIV:  ",IODIV,16,2);
	
	DBG_printVal(DBG_TEXT_LINE13,"SCX:  ", IOSCX, 16, 2);
	DBG_printVal(DBG_TEXT_LINE14,"SCY:  ", IOSCY, 16, 2);
	
#endif
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
	LIB2D_SetFcolor(LCD_TOMATO);
	LIB2D_Text(5*8, p-8, " 0 1 2 3 4 5 6 7 8 9 A C B D E F");
	LIB2D_SetFcolor(LCD_RED);
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
	LIB2D_SetFcolor(LCD_PINK);
	
	for(i=7; i >-1 ; i--)
	{	
		x = printInt(0,i*8,_sp,16,4);
		x = LIB2D_Char(x,i*8,':');
		x = printInt(x,i*8,memoryRead(_sp+1),16,2);
		x = printInt(x,i*8,memoryRead(_sp),16,2);
		_sp +=2;	
	}
}
//----------------------------------------------------*/
//available debug commands
//bp <addr hex>
//------------------------------------------------------
#if defined(_WIN32)
#include <string.h>
#include <stdlib.h>
#elif defined(linux)
#include <string.h>
#endif
void debugCommans(uint8_t *st){
#if defined(_WIN32) || defined(linux)
	static char line[10];
	char *cmd;
	if(readLine(line, 10)){
		
		if(*line == '\0' || *line == ' ' ){ //cartridge return pressed
			*st = DBG_STEP;
			return;
		}

		cmd = (char*)strtok(line, " ");
		if(cmd == NULL) 
			return;		//empty line
		
		if(!strcmp(cmd, "bp")){
			cmd = (char*)strtok(NULL, " ");
			breakpoint = (uint16_t)strtol(cmd, &cmd, 16);
			*st = DBG_CONTINUE;
			return;
		}

		if(!strcmp(cmd, "run")){
			*st = DBG_CONTINUE; 
			return;
		}
	}

#endif
}

//----------------------------------------------------*/
//
//------------------------------------------------------
void _DBG_Info(char* text)
{
	LIB2D_Print("%s\n", text);
}

extern const unsigned short lcd_pal[];
void DBG_DrawTileLine(uint8_t msb, uint8_t lsb) {
	uint8_t i, color;
	for (i = 0; i < 8; i++) {
		color = (msb & 0x80) ? 2 : 0;
		color |= (lsb & 0x80) ? 1 : 0;
		color = (IOBGP >> (color << 1));
		//LCD_Data(lcd_pal[color & 3]);
		msb <<= 1;
		lsb <<= 1;
	}
}

void DBG_DrawTile(uint8_t x, uint8_t y, TileData *td) {
	uint8_t i;
	//LCD_Window(x * 8, y * 8, 8, 8);
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
	LIB2D_SetFcolor(LCD_YELLOW);
	DBG_printVal(DBG_REG_COL(0), DBG_REG_ROW(line), label, val, 16, 2);
}
