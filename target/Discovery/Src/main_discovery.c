#include <stdlib.h>
#include "board.h"
#include "dmgcpu.h"
#include "cartridge.h"
#include "cgbmu.h"
#include "video.h"
#include "pcf8574.h"
#include "lib2d.h"

#if !defined(USE_FS)
extern uint8_t _binary_rom_start;
uint8_t *cartridge = &_binary_rom_start;
#endif

static uint8_t *screen;
uint8_t rgb_pal [256] = {230,248,215, 138,192,114, 55,106,87, 10,26,39};
static const uint32_t argb_pal[] = {0xFFE6F8D7, 0xFF8AC072, 0xFF376A57, 0xFF0A1A27};
#if 0
static void LCD_ConfigVideoDma(uint32_t dst, uint32_t src, uint16_t w, uint16_t h, uint32_t bgc)
{
    DMA2D->CR = DMA2D_CR_M2M_PFC;
    DMA2D->FGMAR = src;
    DMA2D->FGOR = 0;
    DMA2D->FGPFCCR = DMA2D_FGPFCCR_SET_ALPHA(0xFF) |        // Replace alpha 
                     DMA2D_FGPFCCR_SET_CS(256 - 1) |        // CLUT Size
                     DMA2D_FGPFCCR_SET_CM(DMA2D_INPUT_L8) | // Input color format
                     DMA2D_FGPFCCR_CCM;                     // RGB CLUT Mode
    DMA2D->OPFCCR = DMA2D_OPFCCR_SET_CM(DMA2D_OUTPUT_ARGB8888) |
                    //DMA2D_OPFCCR_RBS |                    // Swap Red Blue
                    0;
    DMA2D->OMAR = dst;  									// Absolute start memory of top left
    DMA2D->OOR = BSP_LCD_GetXSize() - w;                    // Add offset to start of next line
    DMA2D->NLR = DMA2D_NLR_PLNL(w, h);

    LTDC->BCCR = bgc;
}
#endif
void videoInit(void)
{    
    BSP_LCD_Init();

    BSP_LCD_LayerDefaultInit(DMA2D_FOREGROUND_LAYER, LCD_FB_START_ADDRESS);     
    BSP_LCD_SelectLayer(DMA2D_FOREGROUND_LAYER);

    BSP_LCD_Clear(LCD_COLOR_BLACK);

    BSP_LED_On(LED2);

    //screen = (uint8_t *)malloc(SCREEN_W * SCREEN_H);
    //LCD_ConfigVideoDma(LCD_FB_START_ADDRESS + SCREEN_OFFSET_X + (SCREEN_OFFSET_Y * BSP_LCD_GetXSize()), (uint32_t)screen, 160, 144, 0);
    screen = (uint8_t*)(LCD_FB_START_ADDRESS + SCREEN_OFFSET_X + (SCREEN_OFFSET_Y * BSP_LCD_GetXSize() * 4));

	uint8_t *pal = rgb_pal;
	for (uint32_t i = 0; i < 256; ++i)
    {
        uint8_t r = *pal++;
        uint8_t g = *pal++;
        uint8_t b = *pal++;

		((uint32_t*)DMA2D->FGCLUT)[i] = (r << 16) | (g << 8) | (b << 0);		
    }
}

/**
 * @brief 
 * 
 * @param x 
 * @param y 
 * @param v 
 * @param radix 
 * @param digitos 
 * @return int 
 */
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

/**
 * @brief 
 * 
 * @param scanline 
 */
void pushScanLine(uint8_t ly, uint8_t *scanline){

    uint8_t *end = scanline + SCREEN_W;
    uint32_t *dst = (uint32_t*)(screen + (ly * BSP_LCD_GetXSize() * 4));

	if(ly == 0){
		//DMA2D->CR |= DMA2D_CR_START;
        BSP_LED_Toggle(LED2);
	}

    while(scanline < end){        
		*dst++ = argb_pal[*scanline++];
    }

    //do{

    //}while(DMA2D->CR & DMA2D_CR_START);
}
/**
 * @brief 
 * 
 * @param fn 
 * @return int 
 */
int loadRom(const uint8_t **dst, char *fn)
{
    //cartridgeInit(cartridge);
    //return ROM_SIZE;
    return 0;
}

/**
 * @brief 
 * 
 * @return uint8_t 
 */
uint8_t readButtons(void)
{
    uint8_t button = 0;

    uint8_t io_val = 255;
    io_drv_pcf8574.read(&io_val, 1);
    io_val = ~io_val;

    if((io_val & BUTTON_CENTER) || BSP_PB_GetState(BUTTON_USER)){ button |= J_START; }
    if(io_val & BUTTON_UP){ button |= J_UP; }
    if(io_val & BUTTON_DOWN){ button |= J_DOWN; }
    if(io_val & BUTTON_LEFT){ button |= J_LEFT; }
    if(io_val & BUTTON_RIGHT){ button |= J_RIGHT; }
    if(io_val & BUTTON_A){ button |= J_SELECT; }
    if(io_val & BUTTON_B){ button |= J_A; }
    if(io_val & BUTTON_C){ button |= J_B; }

    return button;
}

/**
 * @brief 
 * 
 * @return int 
 */
int main(void){
    BOARD_Init();
    videoInit();

    BSP_LCD_SetTextColor(LCD_COLOR_RED);
    BSP_LCD_SetBackColor(LCD_COLOR_BLACK);

    cgbmu(cartridge);
}

