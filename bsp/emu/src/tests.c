

#include <cgbmu.h>
#include <string.h>
#include "board.h"
#include "dmgcpu.h"
#include "video.h"
#include "cartridge.h"
#include "debug.h"
#include "decoder.h"
#include "tests.h"


char filename[100];
extern const char bg_map[32 * 32];
extern const char tilesData[0x1000];

void testRun(void) {
	while (readJoyPad() != 255) {
		interrupts();
		decode();
		timer();
		video();
	}
}

void testAll(void) {
	//testRom(ALL_TESTS_ROM);
	LCD_Window(0, 0, LCD_W, LCD_H);
	LCD_Fill(LCD_SIZE, RED);
	DISPLAY_Text(0,0,"Testing Buttons");
	testButtons();
}

void testRom(char *fn) {
	//strcpy(filename, ROMS_BINARY_PATH);
	//strcat(filename, fn);
	//loadRom(filename);
	loadRom(fn);
	testRun();
}

void testButtons(void) {
	char *name, scanned;
	printf("Buttons Test\n");
	while (readJoyPad() != 255) {
		name = "";

		IOP1 = IOP14;
		scanned = joyPad() & 0x0f;
		switch (scanned) {
			case 0x0e:  name = "  [ A ]   "; break;
			case 0x0d:  name = "  [ B ]   "; break;
			case 0x0b:  name = "[ SELECT ]"; break;
			case 0x07:  name = "[ START ] "; break;			
			default: 
				IOP1 = IOP15;
				scanned = joyPad() & 0x0f;
				switch (scanned) {
					case 0x0e:   name = "[ RIGHT ] "; break;
					case 0x0d:   name = "[ LEFT  ] "; break;
					case 0x0b:   name = "[  UP  ]  "; break;
					case 0x07:   name = "[ DOWN ]  "; break;
					default: break;
				}
				break;
		}
		if (*name != '\0'){
			DISPLAY_printf("%s\n", name);
			printf("%u %s\n", scanned,name);
		}

		#if defined(__EMU__)
		//LCD_Update();		
		SDL_Delay(20);
		#endif
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
	16,16,0,OBJECT_FLAG_XFLIP,
	16 + 8,8,0,OBJECT_FLAG_YFLIP,
	16 + 8,16,0,(OBJECT_FLAG_XFLIP | OBJECT_FLAG_YFLIP)
};
extern uint8_t scanlinedata[160];
void dumpLine(uint8_t *buf, uint8_t size) {
	uint8_t i;
	putchar('[');
	for (i = 0; i < size; i++, buf++) {
		if (*buf)
			printf("%02X", *buf);
		else
			printf("  ");
	}
	printf("]\n");
}

void testSpriteDataLine(void) {
	char i;
	memcpy(vram, f_letter, sizeof(f_letter));
	memcpy(oam, sprites, sizeof(sprites));
	IOOBP0 = IOOBP1 = 0xE4;

	for (i = 0; i < 16; i++) {
		IOLY = i;
		scanOAM();
		dumpLine(scanlinedata, 16);

	}
}

void testBgDataLine(void) {
	char i;
	memcpy(vram, tilesData, sizeof(tilesData));
	memcpy((vram + TILE_MAP0_BASE), bg_map, sizeof(bg_map));
	IOBGP = IOOBP0 = IOOBP1 = 0xE4;
	IOLCDC |= BG_W_DATA;

	for (i = 0; i < 1; i++) {
		IOLY = i;
		scanline();
		dumpLine(scanlinedata, 32);

	}
}

void loadTileMapAndData(void) {
	memcpy(vram, tilesData, sizeof(tilesData));
	memcpy((vram + TILE_MAP0_BASE), bg_map, sizeof(bg_map));
	IOBGP = IOOBP0 = IOOBP1 = 0xE4;
	IOLCDC |= BG_W_DATA;
	IOSCX = 0;
	IOSCY = 0;
}

void Test_DBG_BGmap(void) {
	loadTileMapAndData();
	while (1) {
		//DBG_BGmap();
		LCD_Window(0, 0, SCREEN_W, SCREEN_H);
		for (IOLY = 0; IOLY < SCREEN_H; IOLY++) {
			memset(scanlinedata, 0, sizeof(scanlinedata));
			scanline();
		}

		switch (readJoyPad()) {
		case 255: return;
		case J_UP: IOSCY--; DBG_Reg(); break;
		case J_DOWN: IOSCY++; DBG_Reg(); break;
		case J_LEFT: IOSCX--; DBG_Reg(); break;
		case J_RIGHT: IOSCX++; DBG_Reg(); break;
			
		}
		DelayMs(16);
	}
}

void Test_DBG_Sprites(void) {
	loadTileMapAndData();

	Object *sp = (Object*)&oam[0];

	sp->x = 8;
	sp->y = 16;
	sp->pattern = 1;


	while (1) {
		//DBG_BGmap();
		LCD_Window(0, 0, SCREEN_W, SCREEN_H);
		for (IOLY = 0; IOLY < SCREEN_H; IOLY++) {
			scanOAM();
			scanline();
		}

		switch (readJoyPad()) {
		case 255: return;
		case J_UP: sp->y--; DBG_PrintValue(0,"sp.y ", sp->y); break;
		case J_DOWN: sp->y++; DBG_PrintValue(0,"sp.y ", sp->y); break;
		case J_LEFT: sp->x--; DBG_PrintValue(1,"sp.x ", sp->x); break;
		case J_RIGHT: sp->x++; DBG_PrintValue(1,"sp.x ", sp->x); break;

		}
		DelayMs(30);
	}
}


void testMain(void) {
	//testSpriteDataLine();
	//testBgDataLine();
	//testButtons();
	//Test_DBG_BGmap();
	Test_DBG_Sprites();
	exit(0);
}
