
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

const char f_letter[] = {
    0x78,0x78,
    0x40,0x40,
    0x40,0x40,
    0x78,0x78,
    0x40,0x40,
    0x40,0x40,
    0x40,0x40,
    0x00,0x00
};
const char sprites[] = {
    16,8,0,0,
    16,16,0,SPRITE_FLAG_XFLIP,
    16+8,8,0,SPRITE_FLAG_YFLIP,
    16+8,16,0,(SPRITE_FLAG_XFLIP|SPRITE_FLAG_YFLIP)
};
extern uint8_t spritedataline[160];
void dumpLine(uint8_t *buf, uint8_t size){
uint8_t i;
    putchar('[');
    for(i=0;i<size;i++, buf++){
        if(*buf)
		    printf("%02X",*buf);
        else
            printf("  ");        
    }
    printf("]\n");    
}

void testSpriteDataLine(void){
char i;
    memcpy(vram,f_letter,sizeof(f_letter));
    memcpy(oam,sprites,sizeof(sprites));
    IOOBP0 = IOOBP1 = 0xE4;

for(i=0; i<16; i++){
    IOLY = i;
    scanOAM();
    dumpLine(spritedataline, 16);

}


}

int _main (int argc, char *argv[]){
    testSpriteDataLine();
}
    