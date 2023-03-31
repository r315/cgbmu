
#include <cgbmu.h>
#include <string.h>
#include <stdlib.h>
#include "board.h"
#include "debugger.h"
#include "dmgcpu.h"
#include "decoder.h"
#include "video.h"
#include "liblcd.h"
#include "lib2d.h"

#if defined(_WIN32) || defined(linux)
	#include <conio.h>
	#include "disassembler.h"
#else
void disassembleHeader(void){}
void disassemble(void){}
#endif
	#define FRAME_TIME 16
	#define BREAK_CONDITION(_cond_) if((_cond_)){ printf("break hit\n"); dbg_state = DBG_BREAK; break; }

enum debug_state {
	DBG_PAUSE = 0,
	DBG_BREAK,
	DBG_STEP,
	DBG_FRAME,
	DBG_SINGLE,
	DBG_CONTINUE,
};

static uint32_t frame_counter;
static uint8_t dbg_state = DBG_FRAME;
static uint32_t frame_counter = 0;
static uint16_t break_address = 0;

static void debugCommand(void);

void DBG_SingleStep(void) {
	decode();
	if (video()) {
		frame_counter++;
		updateFps(); 
#if 0
		static uint32_t dtics = GetTick() - dtics;
		if (dtics < FRAME_TIME) {
			SDL_Delay(FRAME_TIME - dtics);
		}
		dtics = GetTick();
#endif
	}
	timer();
	// serial
	interrupts();
}

void DBG_FrameStep(void)
{
	static uint32_t cycles = 0;
	uint32_t dtics = GetTick();

	while (cycles < V_FRAME_CYCLE) {
		decode();
		video();
		timer();
		interrupts();
		cycles += instr_cycles;
	}
	cycles -= V_FRAME_CYCLE;

	frame_counter++;
	updateFps();

	dtics = GetTick() - dtics;
	if (dtics < FRAME_TIME) {
		DelayMs(FRAME_TIME - dtics);
	}
}

void DBG_run(void){	
	uint8_t key;
	initCpu();

	disassembleHeader();
	printf(">");
	DBG_DumpRegisters();

	while((key = readButtons()) != 255){
		debugCommand();
		switch (dbg_state) {

		case DBG_BREAK:			
			dbg_state = DBG_PAUSE;
			disassemble();			
			DBG_DumpRegisters();
			printf(">");
			continue;

		case DBG_STEP:
			DBG_SingleStep();
			dbg_state = DBG_BREAK;
			break;
		
		case DBG_PAUSE:
			DelayMs(30);
			break;

		case DBG_FRAME:
			DBG_FrameStep();
			//BREAK_CONDITION(key == J_A);
			BREAK_CONDITION(REG_PC == break_address);
			break;

		case DBG_SINGLE:
			DBG_SingleStep();			
			//BREAK_CONDITION(key == J_A);
			BREAK_CONDITION(REG_PC == break_address);
			//BREAK_CONDITION(IOIF & JOYPAD_IF);
			//BREAK_CONDITION(memoryRead(REG_PC) == 0xD9);
			//BREAK_CONDITION(REG_PC == 0x0150);
			//BREAK_CONDITION(REG_PC == 0x0100);			
			//BREAK_CONDITION(halt_state == HALT_ACTIVE);
			break;

		default:
			break;
		}
	}
}

//----------------------------------------------------*/
//
//------------------------------------------------------
int DBG_printVal(int x, int y,char *name, int v, char radix, char digitos)
{	
	LIB2D_SetFcolor(LCD_RED);
	x = LIB2D_Text(x,y, name);
	LIB2D_SetFcolor(LCD_YELLOW);
	x = drawInt(x,y,v,radix,digitos);	 
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
	DBG_printVal(TEXT_COL1, TEXT_ROW2, "af:", REG_A << 8| REG_F ,16,4);
	DBG_printVal(TEXT_COL1, TEXT_ROW3, "bc:", REG_BC,16,4);
	DBG_printVal(TEXT_COL1, TEXT_ROW4, "de:", REG_DE,16,4);
	DBG_printVal(TEXT_COL1, TEXT_ROW5, "hl:", REG_HL,16,4);
	DBG_printVal(TEXT_COL1, TEXT_ROW6, "sp:", REG_SP,16,4);
	DBG_printVal(TEXT_COL1, TEXT_ROW7, "pc:", REG_PC,16,4);
	DBG_printVal(TEXT_COL1, TEXT_ROW9, "LCDC:", IOLCDC,16,2);
	DBG_printVal(TEXT_COL1, TEXT_ROW10,"STAT:", IOSTAT, 16, 2);
	DBG_printVal(TEXT_COL1, TEXT_ROW11,"LY:  ", IOLY,16,2);
	DBG_printVal(TEXT_COL1, TEXT_ROW12,"LYC: ", IOLYC, 16, 2);
	DBG_printVal(TEXT_COL1, TEXT_ROW13,"SCX: ", IOSCX, 16, 2);
	DBG_printVal(TEXT_COL1, TEXT_ROW14,"SCY: ", IOSCY, 16, 2);
	DBG_printVal(TEXT_COL1, TEXT_ROW15,"WX:  ", IOWX, 16, 2);
	DBG_printVal(TEXT_COL1, TEXT_ROW16,"WY:  ", IOWY, 16, 2);

	DBG_printVal(TEXT_COL2, TEXT_ROW1, "IE:  ", IOIE, 16, 2);
	DBG_printVal(TEXT_COL2, TEXT_ROW2, "IF:  ", IOIF, 16, 2);
	DBG_printVal(TEXT_COL2, TEXT_ROW3, "TIMA:", IOTIMA,16,2);
	DBG_printVal(TEXT_COL2, TEXT_ROW4, "TMA: ", IOTMA, 16, 2);
	DBG_printVal(TEXT_COL2, TEXT_ROW5, "TAC: ", IOTAC, 16, 2);
	DBG_printVal(TEXT_COL2, TEXT_ROW6, "DIV: ", IODIV,16,2);

	DBG_printVal(TEXT_COL2, TEXT_ROW8, "IME: ", IME, 16, 1);
	DBG_printVal(TEXT_COL2, TEXT_ROW9, "HALT:", halt_state, 16, 1);
	
#endif
}
//----------------------------------------------------*/
//
//------------------------------------------------------
void DBG_DumpMemory(unsigned short addr, unsigned short siz)
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
		x = drawInt(0,p+(i*8),addr,16,4)+8;
		for(j=0; j < 16; j++)
	 		x = drawInt(x,p+(i*9),memoryRead(addr++),16,2);	
	}
#endif	
}
//----------------------------------------------------*/
//
//------------------------------------------------------
void DBG_DumpStackFrame(void)
{
	unsigned short x,_sp;
	signed char i;
	_sp = regs.SP;
	LIB2D_SetFcolor(LCD_PINK);
	
	for(i=7; i >-1 ; i--)
	{	
		x = drawInt(0,i*8,_sp,16,4);
		x = LIB2D_Char(x,i*8,':');
		x = drawInt(x,i*8,memoryRead(_sp+1),16,2);
		x = drawInt(x,i*8,memoryRead(_sp),16,2);
		_sp +=2;	
	}
}


//----------------------------------------------------*/
//
//------------------------------------------------------
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
	for (i = 0; i < 8; i++) {
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
				td = (TileData*)(vram)+offset;
			}
			else {
				td = (TileData*)(vram + TILE_DATA1_SIGNED_BASE) + (int8_t)(offset);
			}
			DBG_DrawTile(w, h, td);
		}
	}
}

/**
 * @brief debug console related stuff
 * 
 */
#if defined(_WIN32)
char readLine(char *dst, uint8_t max) {
	char *p = dst, *end = dst + max;
	int c;

	if (end == dst || !_kbhit()) {
		return 0;
	}
	
	while (*p != '\0' && p < end) {
		p++;
	}

	if (p == end) {
		// Signal line even if incomplete
		dst[max - 1] = '\0';
		return 1;
	}

	while (_kbhit()) {
		c = _getche();
		if (c == '\r') {
			*p = '\0';			
			return dst - p;
		}
		if (c >= ' ' && c <= 'z') {
			*(p++) = (char)c;
		}

		if (p == end) {
			dst[max - 1] = '\0';
			return dst - p;
		}
	}

	return 0;
}
#elif defined(linux)

#include <termios.h>
#include <unistd.h>    
#include <sys/time.h>

static struct termios initial_settings, new_settings;
static int peek_character = -1, init = 0;
void init_keyboard(void){
    tcgetattr(0,&initial_settings);
    new_settings = initial_settings;
    new_settings.c_lflag &= ~ICANON;  //disable cannonical mode    
    new_settings.c_cc[VMIN] = 0;
    new_settings.c_cc[VTIME] = 0;
    tcsetattr(0, TCSANOW, &new_settings);
    init = 1;
}

void close_keyboard(void){
    tcsetattr(0, TCSANOW, &initial_settings);
}

int kbhit(void){

unsigned char ch;
int nread;
    if (peek_character != -1) 
        return 1;

    if(!init) 
        init_keyboard();
        
    new_settings.c_cc[VMIN]=0;
    tcsetattr(0, TCSANOW, &new_settings);
    nread = read(0,&ch,1);
    new_settings.c_cc[VMIN]=1;
    tcsetattr(0, TCSANOW, &new_settings);
    if(nread == 1){
        peek_character = ch;
        return 1;
    }
   return 0;
}

int getch(void){
char ch;
int n;
    if(peek_character != -1)
    {
        ch = peek_character;
        peek_character = -1;
        return ch;
    }
    n = read(0,&ch,1);
    if(!n)
        ch = 0;
    return ch;
}

char readLine(char *dst, uint8_t max) {
	static char *p;
	int c;
	if (!p) p = dst;

	if (kbhit()) {
		c = getch();
		if (c == '\n') {
            *p = '\0';
			p = dst;
			return 1;
		}
		if (c >= ' ' && c <= 'z') {
			*(p++) = (char)c;
		}
	}
	return 0;
}
#endif
//----------------------------------------------------*/
//available debug commands
//bp <addr hex>
//------------------------------------------------------
static void debugCommand(void){
#if defined(_WIN32) || defined(linux)
	static char line[20];

	if(readLine(line, sizeof(line))){
		char *cmd = (char*)strtok(line, " ");
		if (cmd != NULL) {
			if (!strcmp(cmd, "bp")) {
				cmd = (char*)strtok(NULL, " ");
				break_address = (uint16_t)strtol(cmd, &cmd, 16);
			}
			else if (!strcmp(cmd, "run")) {
				if ((cmd = (char*)strtok(NULL, " ")) != NULL) {
					if (cmd[0] == 's') {
						dbg_state = DBG_SINGLE;
					}
				}
				else {
					dbg_state = DBG_FRAME;
				}
			}
		}

		memset(line, 0, sizeof(line));
	}
	else {
		// break on space bar
		if (line[0] == ' ') {
			line[0] = '\0';
			dbg_state = DBG_BREAK;
			printf("\r");
		}
	}

#endif
}
