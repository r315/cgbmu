
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
#include "tests.h"
#include "io.h"

char filename[100];
void decode(void);

void testRun(void) {
	while (readJoyPad() != 255) {		
		interrupts();
		decode();
		timer();			
		video();
	}
}

void testAll(void){    
    testRom(ALL_TESTS_ROM);
}

void testRom(char *fn){
    strcpy(filename, ROMS_BINARY_PATH);
    strcat(filename, fn);
    loadRom(filename);
    testRun();
}


void TEST_BUTTON(uint32_t button){
    char *b = "";
        
        switch(button){
            case BUTTON_UP:     b = "[  UP  ]  "; break;
            case BUTTON_DOWN:   b = "[ DOWN ]  "; break;
            case BUTTON_LEFT:   b = "[ LEFT  ] "; break;
            case BUTTON_RIGHT:  b = "[ RIGHT ] "; break;
            case BUTTON_A:      b = "  [ A ]   "; break;
            case BUTTON_B:      b = "  [ B ]   "; break;
            case BUTTON_START:  b = "[ START ] "; break;        
            case BUTTON_SELECT: b = "[ SELECT ]"; break;        
        }
        printf("Button pressed %s\r",b);
    }
    