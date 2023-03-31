
#include <string.h>
#include <stdlib.h>
#include "board.h"
#include "cgbmu.h"
#include "dmgcpu.h"
#include "video.h"
#include "cartridge.h"
#include "debugger.h"
#include "decoder.h"
#include "tests.h"
#include "lib2d.h"


typedef struct _opt{
	const char *opt;
	char *optv;
	uint32_t flag;
	void *ctx;
	void (*parse)(void *opt);	
}opt_t;

const char *test_roms[] = {
	"cpu_instrs.gb",
	"01-special.gb",
	"02-interrupts.gb",
	"03-op sp,hl.gb",
	"04-op r,imm.gb",
	"05-op rp.gb",
	"06-ld r,r.gb",
	"07-jr,jp,call,ret,rst.gb",
	"08-misc instrs.gb",
	"09-op r,r.gb",
	"10-bit ops.gb",
	"11-op a,(hl).gb",
	"instr_timing.gb",	// https://github.com/retrio/gb-test-roms/tree/master/instr_timing
	"interrupt_time.gb",  // 13
	"phys.gb"             // 14
};

static const char *card_types[] = { "ROM", "MBC1", "MBC1 + RAM" };
const uint16_t lcd_pal[] = { 0xE7DA,0x8E0E,0x334A,0x08C4 };
static uint8_t *MBC1_ROM = NULL;

extern uint8_t done;

void optParseFlag(void *opt){
	*((uint8_t*)(((opt_t*)opt)->ctx)) |= ((opt_t*)opt)->flag;
}

void optParseStr(void *opt){
char **dst = (char**)(((opt_t*)opt)->ctx);
	dst[0] = ((opt_t*)opt)->optv;
	//printf("PARSE: %s\n",(char*)dst[0]);
}

void parseOptions(int argc, char **argv, int optc, opt_t *options) {
	while(argc--){
        for(int i = 0; i < optc; i++){
            if(memcmp(argv[argc], options[i].opt, strlen(argv[argc])) == 0){				
                options[i].optv = argv[argc + 1];
                options[i].parse(&options[i]);
            }
        }
    }
}

#include <stdio.h>
#include <stdlib.h>
#ifdef _WIN32
#pragma warning(disable : 4996)
#include <direct.h>
#else
#include <unistd.h>
#endif

int loadRom(char *file)
{
	char cwd[1024];
	FILE *fp;
	int n;

	if (getcwd(cwd, sizeof(cwd)))
		printf("Current working dir: %s\n", cwd);
	else
		perror("getcwd() error");

	if (file == NULL) {
		printf("No rom file\n");
		return 0;
	}

	printf("Loading File \"%s\"\n", file);
	fp = fopen(file, "rb");

	if (fp == NULL)
	{
		printf("File not found\n");
		return 0;
	}

	fseek(fp, 0L, SEEK_END);
	int sz = ftell(fp);
	fseek(fp, 0L, SEEK_SET);

	printf("Reading %u bytes\n", sz);

	MBC1_ROM = (uint8_t*)malloc(sz);

	n = fread(MBC1_ROM, 1, sz, fp);

	uint8_t card_type = MBC1_ROM[CARTRIDGE_TYPE_OFFSET];

	printf("Card type: %s\n", card_types[card_type]);

	if (card_type > CARTRIDGE_MBC1) {
		printf("Warning ONLY CARTRIDGE MBC1 SUPPORTED \n");
	
	}

	printf("Rom size %uKbit\n", MBC1_ROM[CARTRIDGE_TYPE_OFFSET] * 256);
	cartridgeInit(MBC1_ROM);	
	fclose(fp);
	return n;
}
//-----------------------------------------
//
//-----------------------------------------
uint8_t readButtons(void)
{
static uint8_t button = 0;	
SDL_Event ev;
const Uint8 *keys;

	if(!SDL_PollEvent( &ev )) 
	    return button;	    
	    
	if(ev.type == SDL_QUIT){
		done = 1;
	    return button;
	}
	
	keys  = SDL_GetKeyboardState(NULL);

	if(keys[SDL_SCANCODE_ESCAPE]){
		done = 1;
	    return button;
	} 

	if (ev.type == SDL_KEYDOWN || ev.type == SDL_KEYUP) {
		button = 0;
	}
	else
	{
		return button;
	}

	button |= keys[SDL_SCANCODE_DOWN] ? J_DOWN : 0;
	button |= keys[SDL_SCANCODE_UP]  ? J_UP : 0;
	button |= keys[SDL_SCANCODE_LEFT] ? J_LEFT : 0;
	button |= keys[SDL_SCANCODE_RIGHT] ? J_RIGHT : 0;
		
	button |= keys[SDL_SCANCODE_RETURN] ? J_START : 0;
	button |= keys[SDL_SCANCODE_BACKSPACE] ? J_SELECT : 0;
	button |= keys[SDL_SCANCODE_A] ? J_B : 0;
    button |= keys[SDL_SCANCODE_S ] ? J_A : 0;

    button |= keys[SDL_SCANCODE_SPACE ] ? J_A : 0;
return button;
}

void pushScanLine(uint8_t *scanline) {
	uint16_t tft_line[SCREEN_W];
	uint8_t *end = scanline + SCREEN_W;
	uint8_t pixel = 0;

	while (scanline < end) {
		tft_line[pixel++] = lcd_pal[*scanline++];
	}

	LCD_WriteArea(SCREEN_OFFSET_X, SCREEN_OFFSET_Y + IOLY, SCREEN_W, 1, tft_line);
}

int drawInt(int x, int y, unsigned int v, char radix, char digitos)
{
	unsigned char i = 0, c, dig[16];
	do {
		c = (unsigned char)(v % radix);
		if (c >= 10)c += 7;
		c += '0';
		v /= radix;
		dig[i++] = c;
	} while (v);

	for (c = i; c < digitos; c++)
		x = LIB2D_Char(x, y, '0');

	while (i--)
		x = LIB2D_Char(x, y, dig[i]);
	return x;
}

int loadTestRom(uint8_t nr) {
	char *path = malloc(128);

	strcpy(path, (const char*)ROM_PATH"/tests"); // Defined on project properties

	int len = strlen(path);
	*(path + len) = '/';
	strcpy(path + len + 1, test_roms[nr]);
	
	return loadRom(path);
}

void dry_run(void) {
	initCpu();
	while (readButtons() != 255) {
#if 1
		// slow path
		runOneStep();
#else
		// Fastest run
		runOneFrame();
#endif
	}
}

void printHelp(void) {
	printf("Available options:\n\n"
		"\t -d   Debug on\n"
		"\t -r   <romfile>\n"
		"\t -t   tests\n"
		"\t -i   Instruction mode loop\n");
}
//-----------------------------------------------------------
//instructions test
//-----------------------------------------------------------
int main (int argc, char *argv[])
{
char *romfile = NULL;
uint8_t flags = 0;
opt_t options[] = {
	{"-d", NULL, RUN_FLAG_DEBUG, &flags, optParseFlag},
	{"-t", NULL, RUN_FLAG_TEST, &flags, optParseFlag},
	{"-r", NULL, RUN_FLAG_FILE, &romfile,optParseStr},
	//{"-i", NULL, RUN_FLAG_MODE, &instrs_test_rom_path, optParseStr}
};
	
	if(argc == 1) // no arguments
	{
		printHelp();
		return 0;
	}

	LCD_Init(NULL);
	LIB2D_Init();
	
	parseOptions(argc, argv, sizeof(options)/sizeof(opt_t), options);

#if 1
	if(loadRom(romfile) > 0) {
#else
	if(loadTestRom(0) > 0){
#endif
		//dry_run();		// Generic run

		//DBG_run();		// Run loaded rom in debug mode
	
		cgbmu(MBC1_ROM);  // Run emulator in normal mode	
	}
	else {
		LIB2D_Text(0, 4, "Fail to load rom");
		SDL_Delay(5000);
	}

	LCD_Close();

	if(MBC1_ROM)
		free(MBC1_ROM);

	return 0;
}

