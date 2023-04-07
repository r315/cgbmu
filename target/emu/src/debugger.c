
#include <cgbmu.h>
#include <string.h>
#include <stdlib.h>
#include "board.h"
#include "debugger.h"
#include "dmgcpu.h"
#include "decoder.h"
#include "cartridge.h"
#include "video.h"
#include "liblcd.h"
#include "lib2d.h"
#include "disassembler.h"

#if defined(_WIN32)
	#include <conio.h>
#endif

#if !defined(_WIN32) && !defined(linux)
void disassembleHeader(void){}
void disassemble(cpu_t *cpu){}
#endif

#define FRAME_TIME 16
#define BREAK_CONDITION(_cond_) if((_cond_)){ printf("break hit %d\n", ++break_count); dbg_state = DBG_BREAK; break; }

#define DBG_CPU		dbg_cpu
#define DBG_REG_A	DBG_CPU.A
#define DBG_REG_B	DBG_CPU.B
#define DBG_REG_C	DBG_CPU.C
#define DBG_REG_D	DBG_CPU.D
#define DBG_REG_E	DBG_CPU.E
#define DBG_REG_H	DBG_CPU.H
#define DBG_REG_L	DBG_CPU.L
#define DBG_REG_F	DBG_CPU.F
#define DBG_REG_BC	DBG_CPU.BC
#define DBG_REG_DE	DBG_CPU.DE
#define DBG_REG_HL	DBG_CPU.HL
#define DBG_REG_SP	DBG_CPU.SP
#define DBG_REG_PC	DBG_CPU.PC

#define DBG_REG_IOLY	DBG_CPU.IOLY
#define DBG_REG_IOLYC	DBG_CPU.IOLYC
#define DBG_REG_IOSTAT	DBG_CPU.IOSTAT
#define DBG_REG_IOLCDC	DBG_CPU.IOLCDC
#define DBG_REG_IOSCX	DBG_CPU.IOSCX
#define DBG_REG_IOSCY	DBG_CPU.IOSCY
#define DBG_REG_IOWX	DBG_CPU.IOWX
#define DBG_REG_IOWY	DBG_CPU.IOWY
#define DBG_REG_IOIE	DBG_CPU.IOIE
#define DBG_REG_IOIF	DBG_CPU.IOIF
#define DBG_REG_IOTAC	DBG_CPU.IOTAC
#define DBG_REG_IOTIMA	DBG_CPU.IOTIMA
#define DBG_REG_IOTMA   DBG_CPU.IOTMA
#define DBG_REG_IME	    DBG_CPU.IME
#define DBG_REG_IODIV   DBG_CPU.IODIV
#define DBG_REG_HALT	DBG_CPU.halt



enum debug_state {
	DBG_PAUSE = 0,
	DBG_BREAK,
	DBG_STEP,
	DBG_FRAME,
	DBG_SINGLE,
	DBG_CONTINUE,
};

static cpu_t dbg_cpu;

static uint8_t break_count = 0;
static uint8_t dbg_state = DBG_SINGLE;
static uint32_t frame_counter = 0;
static uint16_t break_address = 0xBEFF;

static void debugCommand(void);

void DBG_SingleStep(void) {
	decode(&dbg_cpu);
	if (video(&dbg_cpu)) {
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
	timer(&dbg_cpu);
	serial(&dbg_cpu);
	interrupts(&dbg_cpu);
}

void DBG_FrameStep(void)
{
	static uint32_t cycles = 0;
	uint32_t dtics = GetTick();

	while (cycles < V_FRAME_CYCLE) {
		decode(&dbg_cpu);
		video(&dbg_cpu);
		timer(&dbg_cpu);
		interrupts(&dbg_cpu);
		cycles += dbg_cpu.instr_cycles;
	}
	cycles -= V_FRAME_CYCLE;

	frame_counter++;
	updateFps();

	dtics = GetTick() - dtics;
	if (dtics < FRAME_TIME) {
		DelayMs(FRAME_TIME - dtics);
	}
}

void DBG_run(const uint8_t *rom){	
	uint8_t key;

	cartridgeInit(&dbg_cpu, rom);
	initCpu(&dbg_cpu);

	disassembleHeader();
	//printf(">");
	DBG_DumpRegisters();

	while((key = readButtons()) != 255){
		//debugCommand(); // very slow
		switch (dbg_state) {

		case DBG_BREAK:			
			dbg_state = DBG_PAUSE;
			disassemble(&dbg_cpu);			
			DBG_DumpRegisters();
			//printf(">");
			continue;

		case DBG_STEP:
			DBG_SingleStep();
			dbg_state = DBG_BREAK;
			break;
		
		case DBG_PAUSE:
			if (key == 'x') {
				dbg_state = DBG_STEP;
			}
			else if (key == 'c') {
				dbg_state = DBG_SINGLE;
			}

			DelayMs(30);
			break;

		case DBG_FRAME:
			DBG_FrameStep();
			//BREAK_CONDITION(key == J_A);
			BREAK_CONDITION(dbg_cpu.PC == break_address);
			break;

		case DBG_SINGLE:
			DBG_SingleStep();
			//BREAK_CONDITION(key == J_A);
			BREAK_CONDITION(dbg_cpu.PC == break_address);
			//BREAK_CONDITION(IOIF & JOYPAD_IF);
			//BREAK_CONDITION(memoryRead(REG_PC) == 0xD9);
			//BREAK_CONDITION(REG_PC == 0x0150);
			//BREAK_CONDITION(REG_PC == 0x0100);
			if (key == 'x') {
				dbg_state = DBG_STEP;
			}
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
	DBG_printVal(TEXT_COL1, TEXT_ROW2, "af:", DBG_REG_A << 8 | DBG_REG_F ,16,4);
	DBG_printVal(TEXT_COL1, TEXT_ROW3, "bc:", DBG_REG_BC,16,4);
	DBG_printVal(TEXT_COL1, TEXT_ROW4, "de:", DBG_REG_DE,16,4);
	DBG_printVal(TEXT_COL1, TEXT_ROW5, "hl:", DBG_REG_HL,16,4);
	DBG_printVal(TEXT_COL1, TEXT_ROW6, "sp:", DBG_REG_SP,16,4);
	DBG_printVal(TEXT_COL1, TEXT_ROW7, "pc:", DBG_REG_PC,16,4);
	DBG_printVal(TEXT_COL1, TEXT_ROW9, "LCDC:", DBG_REG_IOLCDC,16,2);
	DBG_printVal(TEXT_COL1, TEXT_ROW10,"STAT:", DBG_REG_IOSTAT, 16, 2);
	DBG_printVal(TEXT_COL1, TEXT_ROW11,"LY:  ", DBG_REG_IOLY,16,2);
	DBG_printVal(TEXT_COL1, TEXT_ROW12,"LYC: ", DBG_REG_IOLYC, 16, 2);
	DBG_printVal(TEXT_COL1, TEXT_ROW13,"SCX: ", DBG_REG_IOSCX, 16, 2);
	DBG_printVal(TEXT_COL1, TEXT_ROW14,"SCY: ", DBG_REG_IOSCY, 16, 2);
	DBG_printVal(TEXT_COL1, TEXT_ROW15,"WX:  ", DBG_REG_IOWX, 16, 2);
	DBG_printVal(TEXT_COL1, TEXT_ROW16,"WY:  ", DBG_REG_IOWY, 16, 2);

	DBG_printVal(TEXT_COL2, TEXT_ROW1, "IE:  ", DBG_REG_IOIE, 16, 2);
	DBG_printVal(TEXT_COL2, TEXT_ROW2, "IF:  ", DBG_REG_IOIF, 16, 2);
	DBG_printVal(TEXT_COL2, TEXT_ROW3, "TIMA:", DBG_REG_IOTIMA,16,2);
	DBG_printVal(TEXT_COL2, TEXT_ROW4, "TMA: ", DBG_REG_IOTMA, 16, 2);
	DBG_printVal(TEXT_COL2, TEXT_ROW5, "TAC: ", DBG_REG_IOTAC, 16, 2);
	DBG_printVal(TEXT_COL2, TEXT_ROW6, "DIV: ", DBG_REG_IODIV,16,2);

	DBG_printVal(TEXT_COL2, TEXT_ROW8, "IME: ", DBG_REG_IME, 16, 1);
	DBG_printVal(TEXT_COL2, TEXT_ROW9, "HALT:", DBG_REG_HALT, 16, 1);
	
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
	 		x = drawInt(x,p+(i*9),memoryRead(&DBG_CPU, addr++),16,2);
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
	_sp = DBG_CPU.SP;
	LIB2D_SetFcolor(LCD_PINK);
	
	for(i=7; i >-1 ; i--)
	{	
		x = drawInt(0,i*8,_sp,16,4);
		x = LIB2D_Char(x,i*8,':');
		x = drawInt(x,i*8,memoryRead(&DBG_CPU, _sp+1),16,2);
		x = drawInt(x,i*8,memoryRead(&DBG_CPU, _sp),16,2);
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
		color = (DBG_CPU.IOBGP >> (color << 1));
		//LCD_Data(lcd_pal[color & 3]);
		msb <<= 1;
		lsb <<= 1;
	}
}

void DBG_DrawTile(uint8_t x, uint8_t y, tiledata_t *td) {
	uint8_t i;
	//LCD_Window(x * 8, y * 8, 8, 8);
	for (i = 0; i < 8; i++) {
		DBG_DrawTileLine(td->line[i].msb, td->line[i].lsb);
	}
}

void DBG_BGmap(void) {
	uint8_t w, h;
	uint8_t *bgmapbase;
	tiledata_t *td;
	uint8_t offset;

	bgmapbase = (uint8_t*)(DBG_CPU.vram + ((DBG_CPU.IOLCDC & BG_MAP) ? TILE_MAP1_BASE : TILE_MAP0_BASE));

	for (h = 0; h < 32; h++) {
		for (w = 0; w < 32; w++) {
			offset = *(bgmapbase + w + (h * 32));

			if (DBG_CPU.IOLCDC & BG_W_DATA) {
				td = (tiledata_t*)(DBG_CPU.vram)+offset;
			}
			else {
				td = (tiledata_t*)(DBG_CPU.vram + TILE_DATA1_SIGNED_BASE) + (int8_t)(offset);
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
#if defined(_WIN32) || defined(linux)
static void debugCommand(void){
	static char line[20];

	if(readLine(line, sizeof(line))){
		char *cmd = (char*)strtok(line, " ");
		if (cmd != NULL) {
			if (!strcmp(cmd, "bp")) {
				cmd = (char*)strtok(NULL, " ");
				break_address = (uint16_t)strtol(cmd, &cmd, 16);
			}
			else if (!strcmp(cmd, "run")) {
				cmd = (char*)strtok(NULL, " ");
				if (cmd != NULL) {
					if (cmd[0] == 'f') {
						dbg_state = DBG_FRAME;
					}
					else {
						dbg_state = DBG_SINGLE;
					}
				}
				else {
					dbg_state = DBG_SINGLE;
				}
			}
			else if (!strcmp(cmd, "reset")) {
				initCpu(&DBG_CPU);
			}
		}

		memset(line, 0, sizeof(line));
	}
	else {
		// break on space bar
		if (line[0] == ' ') {
			line[0] = '\0';
			if (dbg_state == DBG_SINGLE || dbg_state == DBG_FRAME) {
				dbg_state = DBG_BREAK;
				printf("\r");
			}
			else if (dbg_state == DBG_PAUSE) {
				dbg_state = DBG_STEP;
			}
		}
	}

}
#endif
