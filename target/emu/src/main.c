
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#ifdef _WIN32
#include <direct.h>
#include <windows.h>
#else
#include <unistd.h>
#endif
#include "board.h"
#include "cgbmu.h"
#include "dmgcpu.h"
#include "video.h"
#include "cartridge.h"
#include "debugger.h"
#include "decoder.h"
#include "tests.h"
#include "lib2d.h"

#define MULTIPLE_CPUS	0 // Ensure dynamic allocation for cartridgs and SDL window with proper size

typedef struct threadparam_s {
	cpu_t *cpu;
	const char *romfile;
}threadparam_t;

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
	"instr_timing.gb",	  // https://github.com/retrio/gb-test-roms/tree/master/instr_timing
	"interrupt_time.gb",  // 13
	"phys.gb"             // 14
};

static const char *card_types[] = { 
	"ROM", 
	"MBC1", 
	"MBC1 + RAM", 
	"MBC1 + RAM + BATT",
	"",
	"MBC2",
	"MBC2 + BATT",
	"",
	"ROM + RAM",
	"ROM + RAM + BATT",
	"",
	"ROM + MMMO1",
	"ROM + MMMO1 + SRAM",
	"ROM + MMMO1 + SRAM + BATT",
	"",
	"ROM + MBC3 + TIMER + BATT",
	"ROM + MBC3 + TIMER + RAM + BATT", // 10
	"ROM + MBC3",
	"MBC3 + RAM",
	"MBC3 + RAM + BATT",
	"", "", "", "", "",
	"MBC5",
	"MBC5 + RAM",
	"MBC5 + RAM + BATT",
	"MBC5 + RUMBLE",
	"MBC5 + RUMBLE + SRAM",
	"MBC5 + RUMBLE + SRAM + BAT",
	"CAMERA",
};
const uint16_t lcd_pal[] = { 0xE7DA,0x8E0E,0x334A,0x08C4 };

extern uint8_t done;
static const uint8_t *mbc1_rom;
static cpu_t cpu_1, cpu_2, cpu_3, cpu_4;
#if _WIN32 && MULTIPLE_CPUS
HANDLE ghMutex;
#endif

uint32_t GetTick(void) {
	return SDL_GetTicks();
}
void DelayMs(uint32_t ms) {
	SDL_Delay(ms);
}

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

int loadRom(const uint8_t **dst, const char *file)
{
	char cwd[1024];
	FILE *fp;
	int rom_size;

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

	uint8_t *rom_data = (uint8_t*)malloc(sz);

    rom_size = fread(rom_data, 1, sz, fp);

	uint8_t card_type = rom_data[CARTRIDGE_TYPE_OFFSET];

	printf("Card type: %s\n", card_types[card_type]);

    char *title = (char*)&rom_data[CARTRIDGE_TITLE_OFFSET];

    memset(cwd, '\0', 16);

    for (int i = 0; i < 14; i++, title++) {
        cwd[i] = *title;
        if (*title == '\0') {
            break;
        }
    }

	printf("Game Title: %s\n", cwd);

	if (card_type > CARTRIDGE_MBC1) {
		printf("Warning ONLY CARTRIDGE MBC1 SUPPORTED \n");
	}

	printf("Rom size %ukByte\n", 32 << rom_data[CARTRIDGE_SIZE_OFFSET]);
	fclose(fp);

	*dst = rom_data;
	return rom_size;
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
		cgbmuExit();
	    return 255;
	}
	
	keys  = SDL_GetKeyboardState(NULL);

	if(keys[SDL_SCANCODE_ESCAPE]){
		cgbmuExit();
	    return 255;
	} 

	if (ev.type == SDL_KEYUP || ev.type == SDL_KEYDOWN)
		button = 0;
	else
		return button;

	if (keys[SDL_SCANCODE_X]) {
		button = 0;
		return 'x';
	}

	if (keys[SDL_SCANCODE_C]) {
		button = 0;
		return 'c';
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


void pushScanLine(cpu_t *cpu) {
	uint8_t *pixel = cpu->screen_line;
	uint8_t *end = cpu->screen_line + SCREEN_W;
#if _WIN32 && MULTIPLE_CPUS
	WaitForSingleObject(
		ghMutex,    // handle to mutex
		INFINITE);  // no
	
	if(cpu->id == 2) {
		LCD_Window(SCREEN_OFFSET_X + SCREEN_W, SCREEN_OFFSET_Y + cpu->IOLY, SCREEN_W, 1);
	}
	else if (cpu->id == 3) {
		LCD_Window(SCREEN_OFFSET_X, SCREEN_OFFSET_Y + SCREEN_H + cpu->IOLY, SCREEN_W, 1);
	}
	else if (cpu->id == 4) {
		LCD_Window(SCREEN_OFFSET_X + SCREEN_W, SCREEN_OFFSET_Y + SCREEN_H + cpu->IOLY, SCREEN_W, 1);
	}
	else 
#endif
	{
		LCD_Window(SCREEN_OFFSET_X, SCREEN_OFFSET_Y + cpu->IOLY, SCREEN_W, 1);
	}
	
	while (pixel < end) {
		LCD_Data(lcd_pal[*pixel++]);
	}
#if _WIN32 && MULTIPLE_CPUS
	ReleaseMutex(ghMutex);
#endif
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

#if _WIN32 && MULTIPLE_CPUS
	WaitForSingleObject(
		ghMutex,    // handle to mutex
		INFINITE);  // no
#endif
	for (c = i; c < digitos; c++)
		x = LIB2D_Char(x, y, '0');

	while (i--)
		x = LIB2D_Char(x, y, dig[i]);
#if _WIN32 && MULTIPLE_CPUS
	ReleaseMutex(ghMutex);
#endif
	return x;
}

void drawFps(cpu_t *cpu) {
	static uint32_t fpsupdatetick = 0;
	static uint16_t fps = 0;
	fps++;

	if (GetTick() > fpsupdatetick)
	{
		drawInt(SCREEN_W * 2 + 8, cpu->id * 10, fps, 10, 4);
		fps = 0;
		fpsupdatetick = GetTick() + 1000;
	}
}

int loadTestRom(uint8_t nr) {
	char *path = malloc(128);

	strcpy(path, (const char*)ROMS_PATH"/tests");

	int len = strlen(path);
	*(path + len) = '/';
	strcpy(path + len + 1, test_roms[nr]);
	
	return loadRom(&mbc1_rom, path);
}


#if _WIN32 && MULTIPLE_CPUS
DWORD WINAPI threadRun(LPVOID ptr){
#else
void threadRun(void *ptr) {
#endif

	const uint8_t *rom_data;
	threadparam_t *run = (threadparam_t*)ptr;
	cpu_t *cpu;

	if(loadRom(&rom_data, run->romfile) == 0){
		return;
	}

	cpu = run->cpu;

	cartridgeInit(cpu, rom_data);
	initCpu(cpu);
	
	while (readButtons() != 255) {
#if 1
		// slow path
		decode(cpu);
		if (video(cpu)) {
			drawFps(cpu);
		}
		timer(cpu);
		serial(cpu);
		interrupts(cpu);
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

#if _WIN32 && MULTIPLE_CPUS
	ghMutex = CreateMutex(
		NULL,              // default security attributes
		FALSE,             // initially not owned
		NULL);             // unnamed mutex

	HANDLE threads[4];
	cpu_1.id = 1;
	cpu_2.id = 2;
	cpu_3.id = 3;
	cpu_4.id = 4;
	threadparam_t thread1_param = { &cpu_1, (const char*)ROM_PATH"/dkl.gb" };
	threadparam_t thread2_param = { &cpu_2, (const char*)ROM_PATH"/mario.gb" };
	threadparam_t thread3_param = { &cpu_3, (const char*)ROM_PATH"/Alleyway.gb" };
	threadparam_t thread4_param = { &cpu_4, (const char*)ROM_PATH"/tetris.gb" };

	threads[0] = CreateThread(NULL, 0, threadRun, &thread1_param, 0, NULL);
	threads[1] = CreateThread(NULL, 0, threadRun, &thread2_param, 0, NULL);
	threads[2] = CreateThread(NULL, 0, threadRun, &thread3_param, 0, NULL);
	threads[3] = CreateThread(NULL, 0, threadRun, &thread4_param, 0, NULL);

	WaitForMultipleObjects(4, threads, true, INFINITE);
#endif
	
#if 1
	if(loadRom(&mbc1_rom, romfile) > 0) {
#else
	if(loadTestRom(0) > 0){
#endif
		//DBG_run(mbc1_rom);		// Run loaded rom in debug mode
	
		cgbmu(mbc1_rom);  // Run emulator in normal mode	
	}
	else {
		LIB2D_Text(0, 4, "Fail to load rom");
		SDL_Delay(5000);
	}

	LCD_Close();

	if(mbc1_rom)
		free((void*)mbc1_rom);

	return 0;
}

