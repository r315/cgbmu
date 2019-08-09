
#include <cgbmu.h>
#include <string.h>
#include "board.h"
#include "dmgcpu.h"
#include "video.h"
#include "cartridge.h"
#include "debug.h"
#include "decoder.h"
#include "tests.h"


void cgbmu(uint8_t mode);


typedef struct {
	char *romfile;
	char debug;
	char test;
	char mode;
}Param_Type;

typedef struct {
	const char *agr;
	void(*argHandler)(char *params, Param_Type *param);

}ARG_Type;

void enableDebug(char *params, Param_Type *param) {
	param->debug = 1;
}

void romFile(char *params, Param_Type *param) {
	param->romfile = params;
}

void enableTest(char *params, Param_Type *param) {
	param->test = 1;
}

void mode(char *params, Param_Type *param) {
	param->mode = 1;
}

ARG_Type builtinargs[] = {
	{"-d", enableDebug},
	{"-r", romFile},
	{"-t", enableTest},
	{"-i", mode}
};

void argFound(char **args, Param_Type *param) {
	int i;
	char *arg = args[0];
	for (i = 0; i < sizeof(builtinargs) / sizeof(ARG_Type); i++) {		
		if (arg[0] == builtinargs[i].agr[0] && arg[1] == builtinargs[i].agr[1])
		{
			if (arg[2] == ' ' || arg[2] == '\0') {			    
				builtinargs[i].argHandler(args[1], param);
			}
			else {				
				builtinargs[i].argHandler(&arg[2], param);
			}
			return;
		}
	}
}

void processArgs(int argc, char *argv[], Param_Type *param) {
	int i;
	char *arg;

	for (i = 0; i < argc; i++) {
		arg = argv[i];
		while (*arg) {
			if (*arg == '-') {
				argFound(&argv[i], param);
				break;
			}
			arg++;
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

uint8_t MBC1_ROM[0x10000];

#define LOG_TAG "CARTRIDGE"

int loadRom(char *file)
{
	char cwd[1024];
	FILE *fp;
	int n;


	if (getcwd(cwd, sizeof(cwd)))
		fprintf(stdout, "Current working dir: %s\n", cwd);
	else
		perror("getcwd() error");

	fprintf(stdout, "%s: Loading File \"%s\"\n", LOG_TAG, file);
	fp = fopen(file, "rb");

	if (fp == NULL)
	{
		printf("%s: File not found\n", LOG_TAG);
		return 0;
	}

	n = fread(MBC1_ROM, 1, ROM_SIZE*4, fp);
	printf("%s: Rom file loaded!\n", LOG_TAG);

	if (MBC1_ROM[CARTRIDGE_TYPE_OFFSET] != CARTRIDGE_MBC1) {
		printf("################### ONLY CARTRIDGE MBC1 SUPPORTED ################\n");
		n = 0;
	}
	else {
		ROM0 = MBC1_ROM;
		ROMBANK = MBC1_ROM + ROM_SIZE;
	}
	
	bankselect = 1;
	fclose(fp);
	return n;
}
//--------------------------------------------------
//
//--------------------------------------------------
int loadRombank(uint8_t bank)
{	
	bankselect = bank;
	ROMBANK = MBC1_ROM + (bankselect << 14);
	//printf("Loaded Rom Bank %u\n", bankselect);
	return ROM_SIZE;
}
//-----------------------------------------
//
//-----------------------------------------
uint8_t readJoyPad(void)
{
static uint8_t button = 0;	
SDL_Event ev;
const Uint8 *keys;

	if(!SDL_PollEvent( &ev )) 
	    return button;	    
	    
	if(ev.type == SDL_QUIT){
		button = 255;
	    return button;
	}
	
	keys  = SDL_GetKeyboardState(NULL);

	if(keys[SDL_SCANCODE_ESCAPE]){
		button = 255;
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
//-----------------------------------------------------------
//
//-----------------------------------------------------------
void printHelp(void){
printf("Available options:\n\n"
				"\t -d   Debug on\n"
				"\t -r   <romfile>\n"
				"\t -t   tests\n"
				"\t -i   Instruction mode loop");
}
//-----------------------------------------------------------
//instructions test
//-----------------------------------------------------------
int main (int argc, char *argv[])
{
	#if defined(__BB__)
	

	#elif defined(__ESP03__)	

	#define HSPI_DIVIDER 1
	
	int i = 0;
	nosdk8266_init();

	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U,FUNC_GPIO2);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO5_U,FUNC_GPIO5);

	Cache_Read_Disable();	
	Cache_Read_Enable(0,0,1);	

    HSPI_Init(HSPI_DIVIDER, HSPI_MODE_TX);
	LCD_Init();
	LCD_Clear(RED);
	LCD_Bkl(ON);

	LCD_Rect(0,0,120,120,BLUE);

	GPIO_OUTPUT_SET(0,0);

	float val = 3.141592;
	DISPLAY_printf("Float Test\nPI: %f\n", val);

	LCD_Rotation(LCD_LANDSCAPE);

	initCpu();
	

	#elif defined(__EMU__)	

	static Param_Type param;
	memset(&param, 0, sizeof(Param_Type));	
	
	if(argc == 1) // no arguments
	{
		printHelp();
	}

	LCD_Init();
	
	processArgs(argc - 1, &argv[1], &param);

	if (param.test || !loadRom(param.romfile)) {
		//testAll();
		TEST_main();
	}else if (param.debug) {
		printf("Running in debug mode\n");
		DBG_run();
	}else if (param.mode) {
		printf("Instruction mode\n");
		cgbmu(1);
	}else {
		printf("Frame loop mode\n");
		cgbmu(0);
	}		

	LCD_Close();
	#endif
	return 0;
}

