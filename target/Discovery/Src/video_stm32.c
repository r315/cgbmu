#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "video_stm32.h"
#include "board.h"
#include "dmgcpu.h"

#define DOUBLE_SCREEN 1

typedef struct __attribute__((__packed__)){
    uint8_t b;    
    uint8_t g;
    uint8_t r;
    uint8_t alpha;
}palette_t;

static DMA2D_HandleTypeDef  hdma2d;
static uint32_t forground_clut[256];

uint8_t *screen;

uint8_t palette [256] = {
230,  // R
248,  // G
215,  // B
138,
192,
114,
55,
106,
87,
10,
26,
39
};

uint16_t video_cycles = 0;
Object *spriteline[MAX_OBJECTS / sizeof(Object)];
uint8_t bgdataline[40];
Object *visible_objs[MAX_LINE_OBJECTS];

uint8_t video(void){
    return 1;
}

//-----------------------------------------
//put one line of Sprite data into scanlinedata
//-----------------------------------------
FAST_CODE
void blitObjectData(Object *obj, uint8_t *dst) {
	uint8_t npixels, color, objline;
	uint8_t pal = (obj->flags & OBJECT_FLAG_PAL) ? IOOBP1 : IOOBP0;
	uint8_t pattern = obj->pattern;

	//Get pattern, for 8x16 mode the extra data is on the next pattern index
	if (IOLCDC & OBJ_SIZE) {
		objline = SPRITE_8x16_LINE_MASK(IOLY - (obj->y - SPRITE_Y_OFFSET));		
		objline = (obj->flags & OBJECT_FLAG_YFLIP) ? (SPRITE_8x16_H_MASK - 1 - objline) : objline;
	}
	else {
		// Add line offset, 2byte per line
		objline = TILE_LINE((IOLY - (obj->y - SPRITE_Y_OFFSET)));
		objline = (obj->flags & OBJECT_FLAG_YFLIP) ? (SPRITE_H - 1 - objline) : objline;
	}

	//Each Tile has 16bytes and one line os pixels neads 2 bytes
	TileData *td = (TileData*)vram + pattern;
	
	dst += obj->x - SPRITE_W;

	uint8_t lsb = td->line[objline].lsb;
	uint8_t msb = td->line[objline].msb;
	
	npixels = SPRITE_W;	

	if (obj->flags & OBJECT_FLAG_XFLIP) {		
		do{
			color = (msb & 0x1) ? 2 : 0;
			color |= (lsb & 0x1) ? 1 : 0;
			msb >>= 1;
			lsb >>= 1;
			color = (pal >> (color << 1)) & 3;
			if (color) {
				if(!(obj->flags & OBJECT_FLAG_PRIO) || !*dst)
					*dst = color;
			}
			dst++;
		}while (--npixels);
	}
	else {
		do{
			color = (msb & 0x80) ? 2 : 0;
			color |= (lsb & 0x80) ? 1 : 0;
			msb <<= 1;
			lsb <<= 1;
			color = (pal >> (color << 1)) & 3;
			if (color) {
				if (!(obj->flags & OBJECT_FLAG_PRIO) || !*dst)
					*dst = color;
			}
			dst++;
		}while(--npixels);
	}
}
/**-------------------------------------------------------
* @brief put one line from screen buffer (256x256px) into scanlinedata
*        
* @param mapline		pointer to first tile for the current scanline
* @param dst			pointer for current scanline data
* @param pixeloffset	start pixel for current scanline
* @param line           the line to put
* @param size			SCREEN_W for background, WX for window (not implemented yet)
*---------------------------------------------------------*/
FAST_CODE
void blitTileData(uint8_t *tilemapline, uint8_t *dst, uint8_t pixeloffset, uint8_t line, uint8_t size) {
	uint8_t tileindex, msb, lsb, color;
	TileData *td;

	line = TILE_LINE(line);				// Mask line in tile 

	while (size) {
		tileindex = *(tilemapline + TILE_INDEX(pixeloffset));  // add pixel offset with wraparround
		td = (IOLCDC & BG_W_DATA) ? (TileData*)(vram) + tileindex : (TileData*)(vram + TILE_DATA1_SIGNED_BASE) + (int8_t)tileindex;		
		msb = td->line[line].msb;
		lsb = td->line[line].lsb;
		msb <<= TILE_LINE(pixeloffset);
		lsb <<= TILE_LINE(pixeloffset);

		do {			
			color = (msb & 0x80) ? 2 : 0;
			color |= (lsb & 0x80) ? 1 : 0;
			msb <<= 1;
			lsb <<= 1;
			color = (IOBGP >> (color << 1)) & 3;
			*(dst++) = color;	
			pixeloffset++;
			size--;
		} while (TILE_PIXEL(pixeloffset) != 0 && size != 0);
	}
}

//-----------------------------------------
// read OBJECT Attribute Memory for one line
//-----------------------------------------
FAST_CODE
void scanOAM() {
	uint8_t m, n, objline = IOLY + 16;	// Y position has a offset of 16pixels
	Object *pobj = (Object*)oam;

	n = 0;
	m = (IOLCDC & OBJ_SIZE) ? 1 : 0; // 8x16 objs

	for (int i = 0; i < MAX_OBJECTS; i++, pobj++) {
		if (pobj->x >= SPRITE_W && pobj->x < SCREEN_W + SPRITE_W) {			
			if (objline >= pobj->y && objline < (pobj->y + (SPRITE_H << m))) {
				visible_objs[n] = pobj;
				n++;
			}					
		}		
		if (n >= MAX_LINE_OBJECTS)
			break;
	}
	visible_objs[n] = NULL;
}
//-----------------------------------------
//
//-----------------------------------------
FAST_CODE
void scanline() {
	uint8_t *tilemapline;
	uint8_t pixel, line;
    uint8_t *scanline = screen + (IOLY * SCREEN_W);
	uint8_t *sld = scanline;

	// Get tile map base
	tilemapline = (uint8_t*)(vram + ((IOLCDC & BG_MAP) ? TILE_MAP1_BASE : TILE_MAP0_BASE));
	// Add line and scroll-y offset for getting tile pattern	
	line = (uint8_t)(IOLY + IOSCY);
	tilemapline += TILE_LINE_INDEX(line);

	memset(sld, 0, sizeof(SCREEN_W));
	blitTileData(tilemapline, sld, IOSCX, line, SCREEN_W);	

	if (IOLCDC & W_DISPLAY && IOLY >= IOWY && IOWX < SCREEN_W + 7) 
	{
		line = IOLY - IOWY;					
		sld = scanline + IOWX - 7;				//destination offset given by IOWX, WX has an offset of 7
		tilemapline = (uint8_t*)(vram + ((IOLCDC & W_MAP) ? TILE_MAP1_BASE : TILE_MAP0_BASE));
		tilemapline += TILE_LINE_INDEX(line);
		blitTileData(tilemapline, sld, 0, line, SCREEN_W - IOWX + 7);
	}

	pixel = 0;
	while (visible_objs[pixel] != NULL)
		blitObjectData(visible_objs[pixel++], scanline);

	//sld = scanlinedata;
	//for (pixel = 0; pixel < SCREEN_W; pixel++, sld++) {
	//	LCD_Data(lcd_pal[*sld]);
	//}
}
//-----------------------------------------
// Clear/set Coincidence flag on STAT
// activate STAT IF if Coincedence or OAM
//-----------------------------------------
FAST_CODE
void nextLine(void) {
	IOLY++;
	IOSTAT = (IOLY == IOLYC) ? (IOSTAT | LYC_LY_FLAG) : (IOSTAT & (~LYC_LY_FLAG));	
	//if ((IOSTAT & LYC_LY_IE) && (IOSTAT & LYC_LY_FLAG))
	if (IOSTAT & LYC_LY_FLAG)
		IOIF |= LCDC_IF;
}
//
//  LoadPalette
//
void LoadPalette(uint32_t *clut)
{   
    DMA2D_CLUTCfgTypeDef CLUTCfg;

    hdma2d.Instance = DMA2D;   
    
    /*##-1- Configure the DMA2D Mode, Color Mode and output offset #############*/ 
    hdma2d.Init.Mode          = DMA2D_M2M_PFC;
    hdma2d.Init.ColorMode     = DMA2D_OUTPUT_ARGB8888;
#ifdef DOUBLE_SCREEN
    hdma2d.Init.OutputOffset  = BSP_LCD_GetXSize() - (SCREEN_W<<1);
#else
    hdma2d.Init.OutputOffset  = BSP_LCD_GetXSize() - SCREEN_W; // Offset added to the end of each line
#endif
    hdma2d.Init.AlphaInverted = DMA2D_REGULAR_ALPHA;  /* No Output Alpha Inversion*/  
    hdma2d.Init.RedBlueSwap   = DMA2D_RB_REGULAR;     /* No Output Red & Blue swap */  

    /*##-2- DMA2D Callbacks Configuration ######################################*/
    hdma2d.XferCpltCallback  = NULL;

    /*##-3- Foreground Configuration ###########################################*/
    hdma2d.LayerCfg[DMA2D_FOREGROUND_LAYER].AlphaMode      = DMA2D_REPLACE_ALPHA;
    hdma2d.LayerCfg[DMA2D_FOREGROUND_LAYER].InputAlpha     = 0xFF; /* Opaque */
    hdma2d.LayerCfg[DMA2D_FOREGROUND_LAYER].InputColorMode = DMA2D_INPUT_L8;
    hdma2d.LayerCfg[DMA2D_FOREGROUND_LAYER].InputOffset    = 0;
    hdma2d.LayerCfg[DMA2D_FOREGROUND_LAYER].AlphaInverted  = DMA2D_REGULAR_ALPHA; /* No ForeGround Alpha inversion */  
    hdma2d.LayerCfg[DMA2D_FOREGROUND_LAYER].RedBlueSwap    = DMA2D_RB_REGULAR;    /* No ForeGround Red/Blue swap */ 
   
    /* DMA2D Initialization */
    if(HAL_DMA2D_Init(&hdma2d) == HAL_OK) 
    {
        if(HAL_DMA2D_ConfigLayer(&hdma2d, DMA2D_FOREGROUND_LAYER) == HAL_OK) 
        {
            /* Load DMA2D Foreground CLUT */
            CLUTCfg.CLUTColorMode = DMA2D_CCM_ARGB8888;     
            CLUTCfg.pCLUT = clut;
            CLUTCfg.Size = 255;

            if(HAL_DMA2D_CLUTLoad(&hdma2d, CLUTCfg, DMA2D_FOREGROUND_LAYER) == HAL_OK){
                HAL_DMA2D_PollForTransfer(&hdma2d, 100);  
            }            
        }
    }   
}

void VIDEO_Update(void)
{
#ifdef DOUBLE_SCREEN   
    #define FRAME_OFFSET ( ((400 - SCREEN_W) + ((240 - SCREEN_H) * 800)) * 4)

    uint8_t *dfb = malloc(SCREEN_W * SCREEN_H * 4);
    uint8_t *fb = screen;

    for(int i = 0; i < SCREEN_H<<1; i+=2){
        for(int j = 0; j < SCREEN_W<<1; j+=2, fb++){
            uint8_t idx = *fb;
            dfb[(i*SCREEN_W<<1) + j] = idx;
            dfb[(i*SCREEN_W<<1) + j + 1] = idx;
            dfb[((i+1)*SCREEN_W<<1) + j] = idx;
            dfb[((i+1)*SCREEN_W<<1) + j + 1] = idx;
        }
    }
    //CopyBuffer((uint32_t *)LCD_FB_START_ADDRESS, (uint32_t *)screens[0], 400 - 160, 240 - 100, 320, 200);
    if (HAL_DMA2D_Start(&hdma2d, (uint32_t)dfb, LCD_FB_START_ADDRESS + FRAME_OFFSET, SCREEN_W<<1, SCREEN_H<<1) == HAL_OK)
    {
        /* Polling For DMA transfer */  
        HAL_DMA2D_PollForTransfer(&hdma2d, 100);               
    }
    free(dfb);
#else
    #define FRAME_OFFSET ( ((800 - SCREEN_W)/2 + ((480 - SCREEN_H) * 400)) * 4)
    if (HAL_DMA2D_Start(&hdma2d, (uint32_t)screen, LCD_FB_START_ADDRESS + FRAME_OFFSET, SCREEN_W, SCREEN_H) == HAL_OK)
    {
        /* Polling For DMA transfer */  
        HAL_DMA2D_PollForTransfer(&hdma2d, 0);               
    }
#endif
}

//
// I_SetPalette
//
void VIDEO_SetPalette(uint8_t *palette)
{
  palette_t *pClut = (palette_t*)forground_clut;
  uint8_t brightness = 50;

    for (uint32_t i = 0; i < 256; ++i, ++pClut)
    {
        pClut->r = *palette++;
        pClut->g = *palette++;
        pClut->b = *palette++;
        pClut->alpha = 0xFF;

        pClut->r = (pClut->r + brightness) < 256 ? pClut->r + brightness : 255;
        pClut->g = (pClut->g + brightness) < 256 ? pClut->g + brightness : 255;
        pClut->b = (pClut->b + brightness) < 256 ? pClut->b + brightness : 255;
    }
    
    LoadPalette(forground_clut);
}

void VIDEO_Init(void)
{    
    BSP_LCD_Init();

    BSP_LCD_LayerDefaultInit(DMA2D_FOREGROUND_LAYER, LCD_FB_START_ADDRESS);     
    BSP_LCD_SelectLayer(DMA2D_FOREGROUND_LAYER);

    BSP_LCD_Clear(LCD_COLOR_BLACK);

    BSP_LED_On(LED2);   

    //BSP_LCD_SetFont(&Font16);
    //BSP_LCD_DisplayStringAtLine(0, "Hello"); 

    screen = (uint8_t *)malloc(SCREEN_W * SCREEN_H);

    VIDEO_SetPalette(palette);
    //memset(screens[0], 0, 320 * 200);
    //LCD_LayerInit(0, LCD_FB_START_ADDRESS); 
    //_CopyBuffer((uint32_t *)LCD_FB_START_ADDRESS, (uint32_t *)screens[0], 400 - 160, 240 - 100, 320, 200);
}

void LCD_Window(uint16_t x, uint16_t y, uint16_t w, uint16_t h){
    VIDEO_Update();
}

