

#include <common.h>
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


void testButtons(void){
	char *b,t;
	printf("Buttons Test\n");
	while (readJoyPad() != 255) {
		b = "";
		IOP1 = IOP14;
		t = joyPad() & 0x0f;
		switch (t) {
		case 0x0e:  b = "  [ A ]   "; break;
		case 0x0d:  b = "  [ B ]   "; break;
		case 0x0b:  b = "[ SELECT ]"; break;
		case 0x07:  b = "[ START ] "; break;
		case 15: break;
		default:
			printf("%u\n", t);
			break;
		}
		IOP1 = IOP15;
		switch (joyPad() & 0x0f) {
		case 0x0e:   b = "[ RIGHT ] "; break;
		case 0x0d:   b = "[ LEFT  ] "; break;
		case 0x0b:   b = "[  UP  ]  "; break;
		case 0x07:   b = "[ DOWN ]  "; break;
		}
		if(*b !='\0')
			printf("%s\n",b);
		SDL_Delay(20);
	}
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
extern uint8_t scanlinedata[160];
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
    dumpLine(scanlinedata, 16);

}


}

const char letters[] = {
	0x38,0x38,		// A
	0x44,0x44,
	0x44,0x44,
	0x7c,0x7c,
	0x44,0x44,
	0x44,0x44,
	0x44,0x44,
	0x00,0x00,


	0x78,0x78,			// F
	0x40,0x40,
	0x40,0x40,
	0x78,0x78,
	0x40,0x40,
	0x40,0x40,
	0x40,0x40,
	0x00,0x00,

	0xfe,0xfe,
	0x02,0xc2,
	0x82,0x82,
	0x82,0x82,
	0x82,0x82,
	0x82,0x82,
	0xfe,0xfe,
	0x00,0x00,

	0xff,0xff,
	0xff,0xff,
	0xff,0xff,
	0xff,0xff,
	0xff,0xff,
	0xff,0xff,
	0xff,0xff,
	0xff,0xff,
};
#define _P2 2
#define _P0 3
#define _P1 1
#define _P3 3

const char bg_map[] = {
	_P0,_P0,_P0,_P0,_P0,_P0,_P0,_P0,_P1,_P1,_P1,_P1,_P1,_P1,_P1,_P1,_P2,_P2,_P2,_P2,_P2,_P2,_P2,_P2,_P3,_P3,_P3,_P3,_P3,_P3,_P3,_P3,	
	_P1,_P1,_P1,_P1,_P1,_P1,_P1,_P1,_P2,_P2,_P2,_P2,_P2,_P2,_P2,_P2,_P3,_P3,_P3,_P3,_P3,_P3,_P3,_P3,_P0,_P0,_P0,_P0,_P0,_P0,_P0,_P0
};

void testBgDataLine(void) {
	char i;
	memcpy(vram, letters, sizeof(letters));
	memcpy((vram + TILE_MAP0_BASE), bg_map, sizeof(bg_map));
	IOBGP = IOOBP0 = IOOBP1 = 0xE4;
	IOLCDC |= BG_W_DATA;

	for (i = 0; i<1; i++) {
		IOLY = i;
		scanline();
		dumpLine(scanlinedata, 32);

	}


}


void testMain (void){
    //testSpriteDataLine();
	//testBgDataLine();
	testButtons();
	exit(0);
}
    
