
#include <common.h>
#include <string.h>
#include "display.h"
#include "dmgcpu.h"
#include "video.h"
#include "lcd.h"
#include "cartridge.h"
#include "debug.h"
#include "button.h"
#include "decoder.h"
#include "io.h"
#include "tests.h"

#if defined(__EMU__)

typedef struct {
	char *romfile;
	char debug;
	char test;
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

ARG_Type builtinargs[] = {
	{"-d", enableDebug},
	{"-r", romFile},
	{"-t", enableTest}
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
			*arg++;
		}
	}
}

#endif

void run(void) {
	int wait = 0;
	while (readJoyPad() != 255) {		
		decode();
		interrupts();
		timer();
		if (video() == ON) {
			//DBG_Fps();
			wait = wait - GetTicks();
			if (wait > 0)
				DelayMs(wait);
			wait = GetTicks() + 16;
		}
	}
}
//-----------------------------------------------------------
//instructions test
//-----------------------------------------------------------
int main (int argc, char *argv[])
{
	#if defined(__BB__)
	BB_Init();
	BB_ConfigPLL(PLL100);	

	LCD_Rotation(LCD_LANDSCAPE);

	DISPLAY_puts("Hello\n");

	initCpu();		

	loadRom("mario.gb");
	run();	

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
	
	LCD_Init();	 
	
	initCpu();	

	if(argc == 1) // no arguments
		testAll();
	else {
		processArgs(argc - 1, &argv[1], &param);
		if (!loadRom(param.romfile))
			exit(1);

		if (param.test) {
			testMain();
		}else
			if (param.debug) {
				debug();
			}
			else {
				run();
			}
	}	

	LCD_Close();
	#endif
	return 0;
}

