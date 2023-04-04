#include "board.h"
#include "libbutton.h"
#include "lib2d.h"
#include "liblcd.h"
#include "cgbmu.h"
#include "cartridge.h"
#include "video.h"
#include "dmgcpu.h"



#if !defined(USE_FS)
extern uint8_t _binary_rom_start;
static const uint8_t *cartridge = &_binary_rom_start;
#endif


// Dark - light
// RGB(0,19, 25), RGB(60,127,38), RGB(170, 204, 71), RGB(248, 255, 178)
//const uint16_t lcd_pal[] = { 0xE7DA,0x8E0E,0x334A,0x08C4 };
const uint16_t lcd_pal[] = {RGB565(0x9C, 0xBD, 0x10), RGB565(0x8C, 0xAD, 0x10), RGB565(0x31, 0x63, 0x31), RGB565(0x10, 0x39, 0x10)};
static uint16_t tft_line[SCREEN_W];

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

	for (c = i; c < digitos; c++)
		x = LIB2D_Char(x, y, '0');

	while (i--)
		x = LIB2D_Char(x, y, dig[i]);
	return x;
}

void pushScanLine(uint8_t ly, uint8_t *scanline){

    uint8_t *end = scanline + SCREEN_W;
    uint8_t pixel = 0;

    while(scanline < end){
		tft_line[pixel++] = lcd_pal[*scanline++];
    }
    
    LCD_WriteArea(SCREEN_OFFSET_X, SCREEN_OFFSET_Y + ly, SCREEN_W, 1, tft_line);
}

int loadRom(char *fn)
{
    //load from SD Card here
    //cartridgeInit(&CARTRIDGE_NAME);
    //return ROM_SIZE;
    return 0;
}

uint8_t readButtons(void)
{
    uint8_t button = 0;
    //int	keys = ~LPC_GPIO1->FIOPIN & BUTTON_MASK;
    
    /*button |= ( keys & BUTTON_DOWN) ? J_DOWN : 0;
    button |= ( keys & BUTTON_UP)  ? J_UP : 0;
    button |= ( keys & BUTTON_LEFT) ? J_LEFT : 0;
    button |= ( keys & BUTTON_RIGHT) ? J_RIGHT : 0;		
    button |= ( keys & BUTTON_A) ? J_START : 0;
    */
    return button;
}


int main(void)
{
    BOARD_Init();

    LCD_SetOrientation(LCD_REVERSE_LANDSCAPE);
    
    LIB2D_Init();
    LIB2D_Print("CPU %uMHz\n", SystemCoreClock/1000000);
    
    cgbmu(cartridge);
    
    while (1)
    {
        LED1_TOGGLE;
        DelayMs(200);
    }

    return 0;
}