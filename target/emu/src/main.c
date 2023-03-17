
#include <cgbmu.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include "board.h"
#include "dmgcpu.h"
#include "video.h"
#include "cartridge.h"
#include "debug.h"
#include "decoder.h"
#include "tests.h"

void cgbmu(uint8_t mode);

typedef struct _opt{
	const char *opt;
	char *optv;
	uint32_t flag;
	void *ctx;
	void (*parse)(void *opt);	
}opt_t;

void optParseFlag(void *opt){
	*((uint8_t*)(((opt_t*)opt)->ctx)) |= ((opt_t*)opt)->flag;
}

void optParseInt(void *opt){
}

void optParseStr(void *opt){
char **dst = (char**)(((opt_t*)opt)->ctx);
	dst[0] = ((opt_t*)opt)->optv;
	//printf("PARSE: %s\n",(char*)dst[0]);
}

void parseOptions(int argc, char **argv, int optc, opt_t *options) {
	while(argc--){
        for(uint32_t i = 0; i < optc; i++){
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

static uint8_t *MBC1_ROM;

#define LOG_TAG "MAIN"

int loadRom(char *file)
{
	char cwd[1024];
	FILE *fp;
	int n;

	if (getcwd(cwd, sizeof(cwd)))
		fprintf(stdout, "Current working dir: %s\n", cwd);
	else
		perror("getcwd() error");

	if (file == NULL) {
		fprintf(stdout, "%s: No rom file\n", __func__);
		return 0;
	}

	fprintf(stdout, "%s: Loading File \"%s\"\n", LOG_TAG, file);
	fp = fopen(file, "rb");

	if (fp == NULL)
	{
		printf("%s: File not found\n", LOG_TAG);
		return 0;
	}

	fseek(fp, 0L, SEEK_END);
	int sz = ftell(fp);
	fseek(fp, 0L, SEEK_SET);

	printf("%s: Reading %u bytes\n", LOG_TAG, sz);

	MBC1_ROM = (uint8_t*)malloc(sz);

	n = fread(MBC1_ROM, 1, sz, fp);

	if (MBC1_ROM[CARTRIDGE_TYPE_OFFSET] > CARTRIDGE_MBC1) {
		printf("################### ONLY CARTRIDGE MBC1 SUPPORTED ################\n%u\n", MBC1_ROM[CARTRIDGE_TYPE_OFFSET]);
		n = 0;
	}
	else {		
		printf("Rom size %uKbit\n", MBC1_ROM[CARTRIDGE_TYPE_OFFSET] * 256);
		cartridgeInit(MBC1_ROM);
	}
	fclose(fp);
	return n;
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

void *sdf(void *r){
	while(1){
		LCD_Update();
		SDL_Delay(20);
	}
	return NULL;
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
	#if defined(__ESP03__)	

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
	LIB2D_Print("Float Test\nPI: %f\n", val);

	LCD_Rotation(LCD_LANDSCAPE);

	initCpu();
	

	#else
char *romfile;
uint8_t flags = 0;
opt_t options[] = {
	{"-d", NULL, RUN_FLAG_DEBUG, &flags, optParseFlag},
	{"-t", NULL, RUN_FLAG_TEST, &flags, optParseFlag},
	{"-r", NULL, RUN_FLAG_FILE, &romfile,optParseStr},
	{"-i", NULL, RUN_FLAG_MODE, &flags, optParseFlag}
};
	
	if(argc == 1) // no arguments
	{
		printHelp();
		return 0;
	}

	LCD_Init();

	pthread_t update;
	pthread_create(&update,NULL,sdf,NULL);
	
	parseOptions(argc, argv, sizeof(options)/sizeof(opt_t), options);	

	if (!loadRom(romfile)) {
		//if(flags & RUN_FLAG_TEST){
			//testAll();
			TEST_main(flags);
		//}
	}else{
	
		if(flags & RUN_FLAG_DEBUG){
			printf("Running in debug mode\n");
			LCD_SetWidth(256);
			DBG_run(flags);
		}else{
			cgbmu(flags);
		}
	}
	
	/*
	 if ((flags & FLAG_DEBUG)) {

	}else if ((flags & FLAG_MODE)) {
		printf("Instruction mode\n");
		cgbmu(1);
	}else {
		printf("Frame loop mode\n");
		cgbmu(0);
	}		
*/
	LCD_Close();
	free(MBC1_ROM);
	#endif
	return 0;
}

