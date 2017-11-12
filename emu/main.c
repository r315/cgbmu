
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


void run(void){
char pause = 1, key = 0;
 dumpRegisters();
 while(key != -1){
	if(!pause){
		dumpRegisters();
		interrupts();		     
   		decode();
		timer();		
		if(!video()){
			cycles = 0;	
			frameReady = 0;
			LCD_Window(0,0,160,144);	
		}
		//pause = 1;
	}
	
	key = joyPad();
	if(key == 1)
		pause = 0;
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
			runTest();
			break;

		case 3:
			loadRom(argv[2]);
			if(!strcmp(argv[1], "debug")){
				debug();
			}
			else{
				runTest();
			}

		default: 
			break;
	}
	

	LCD_Close();
	return 0;
}

