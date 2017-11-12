
#ifdef WIN32
#include <SDL.h>
#else
#include <SDL2/SDL.h>
#endif

#include "graphics.h"
#include "dmgcpu.h"
#include "video.h"
#include "lcd.h"
#include "cartridge.h"
#include "debug.h"
#include "button.h"
#include "decoder.h"
#include "io.h"
#include "tests.h"

void run(void) {
	while (readJoyPad() != 255) {		
		interrupts();
		decode();
		timer();			
		video();
	}
}
//-----------------------------------------------------------
//instructions test
//-----------------------------------------------------------
int main (int argc, char *argv[])
{
	LCD_Init();	 
	initCpu();	
	switch(argc){
		case 1:
			testAll();
			break;
		
		case 2:
			loadRom(argv[1]);
			run();
			break;

		case 3:
			if(!strcmp(argv[1], "debug")){
				loadRom(argv[2]);
				debug();
			}
			else if (!strcmp(argv[1], "test")) {
				testRom(argv[2]);
			}
			else{
				loadRom(argv[2]);
				run();
			}

		default: 
			break;
	}
	

	LCD_Close();
	return 0;
}

