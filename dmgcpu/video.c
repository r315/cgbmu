
#include <string.h>
#include "dmgcpu.h"
#include "video.h"
#include "lcd.h"

const unsigned short lcd_pal[] = { 0xE7DA,0x8E0E,0x334A,0x08C4 };

uint16_t video_cycles = 0;
Sprite *spriteline[MAX_SPRITES / sizeof(Sprite)];
uint8_t bgdataline[40];
uint8_t scanlinedata[160];		// one line of pixels

//-----------------------------------------
//put one line of Sprite data into scanlinedata
//-----------------------------------------
void putSpriteData(Sprite *sp, uint8_t *dst) {
	uint8_t i = SPRITE_W, color;
	uint8_t pal = (sp->flags & SPRITE_FLAG_PAL) ? IOOBP1 : IOOBP0;
	//Each Tile has 16bytes and one line os pixels neads 2 bytes
	uint8_t *p = (uint8_t*)(vram + (sp->pattern * sizeof(TileData)));	//get tiledata base
	// add line offset, 2byte per line
	p += ((sp->flags & SPRITE_FLAG_YFLIP) ? (7 - (IOLY & TILE_LINE_MASK)) : (IOLY & TILE_LINE_MASK)) << 1;
	uint8_t lsb = *p++;
	uint8_t msb = *p;

	if (sp->flags & SPRITE_FLAG_XFLIP) {
		while (i--) {
			color = (msb >> (7 - i)) & 1;
			color <<= 1;
			color |= (lsb >> (7 - i)) & 1;
			color = (pal >> (color << 1)) & 3;  // pal uses 2 bit per color
			if (color)
				*dst = color;
			dst += 1;
		}
	}
	else {
		while (i--) {
			color = (msb >> i) & 1;
			color <<= 1;
			color |= (lsb >> i) & 1;
			color = (pal >> (color << 1)) & 3;
			if (color)
				*dst = color;
			dst += 1;
		}
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
void putTileData(uint8_t *tilemapline, uint8_t *dst, uint8_t pixeloffset, uint8_t line, uint8_t size, uint8_t transparent) {
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
			if (transparent == ON) {
				if (!*dst)
					goto putpixel;
			}
			else {
			putpixel:
				color = (msb & 0x80) ? 2 : 0;
				color |= (lsb & 0x80) ? 1 : 0;
				msb <<= 1;
				lsb <<= 1;
				color = (IOBGP >> (color << 1)) & 3;
				*dst = color;
			}				
			dst += 1;
			pixeloffset++;
			size--;
		} while (TILE_PIXEL(pixeloffset) != 0 && size != 0);
	}
}

//-----------------------------------------
// read OBJECT Attribute Memory for one line
//-----------------------------------------
void scanOAM() {
	uint8_t i, n, tileline = (IOLY + 16) >> 3;	// Y position has a offset of 16pixels
	Sprite *poam = (Sprite*)&oam[0];

	memset(scanlinedata, 0, sizeof(scanlinedata));

	n = 0;
	for (i = 0; i < MAX_SPRITES; i++) {
		if (tileline == (poam->y >> 3)) {
			if (poam->x >= SPRITE_W && poam->x < SCREEN_W + SPRITE_W) {
				putSpriteData(poam, scanlinedata + poam->x - 8);
				n++;
			}
		}
		poam += 1;
		if (n >= MAX_LINE_SPRITES)
			break;
	}
}
//-----------------------------------------
//
//-----------------------------------------
void scanline() {
	uint8_t *tilemapline;
	uint8_t pixel, line;
	uint8_t *sld = scanlinedata;

	// Get tile map base
	tilemapline = (uint8_t*)(vram + ((IOLCDC & BG_MAP) ? TILE_MAP1_BASE : TILE_MAP0_BASE));
	// Add line and scroll-y offset for getting tile pattern	
	//bgmapline += (TILE_LINE_INDEX(IOLY) + TILE_LINE_INDEX(IOSCY)) & BG_SIZE_MASK;
	line = (uint8_t)(IOLY + IOSCY);
	tilemapline += TILE_LINE_INDEX(line);

	putTileData(tilemapline, sld, IOSCX, line, SCREEN_W, ON);

	if (IOLCDC & W_DISPLAY && IOLY >= IOWY && IOWX < SCREEN_W + 7) 
	{
		line = IOLY - IOWY;					
		sld = scanlinedata + IOWX - 7;				//destination offset given by IOWX, WX has an offset of 7
		tilemapline = (uint8_t*)(vram + ((IOLCDC & W_MAP) ? TILE_MAP1_BASE : TILE_MAP0_BASE));
		tilemapline += TILE_LINE_INDEX(line);
		putTileData(tilemapline, sld, 0, line, SCREEN_W - IOWX + 7, OFF);
	}

	sld = scanlinedata;
	for (pixel = 0; pixel < SCREEN_W; pixel++, sld++) {
		LCD_Data(lcd_pal[*sld]);
	}
}
//-----------------------------------------
// Clear/set Coincidence flag on STAT
// activate STAT IF if Coincedence or OAM
//-----------------------------------------
void nextLine(void) {
	IOLY++;
	IOSTAT = (IOLY == IOLYC) ? (IOSTAT | LYC_LY_FLAG) : (IOSTAT & (~LYC_LY_FLAG));	
	if (IOSTAT & (LYC_LY_IE | LYC_LY_FLAG))
		IOIF |= LCDC_IF;
}
//-----------------------------------------
//
//-----------------------------------------
uint8_t video(void) {
	uint8_t frame = OFF;
	if (!(IOLCDC & LCD_DISPLAY)) return frame; 	// Lcd controller off	

	video_cycles += GET_CYCLE();

	switch (IOSTAT & V_MODE_MASK)
	{
	case V_M2: 							// Mode 2 oam access start scanline	
		if (video_cycles > V_M2_CYCLE)
		{
			video_cycles -= V_M2_CYCLE;
			IOSTAT |= V_M3;				// Next, Mode 3 vram access				
			scanOAM();
		}
		break;

	case V_M3: 							// Mode 3 vram access
		if (video_cycles > V_M3_CYCLE)
		{
			video_cycles -= V_M3_CYCLE;
			IOSTAT &= ~(V_MODE_MASK);  // Next, Mode 0 H-blank				 
			if (IOSTAT & HB_IE) 		// LCD STAT & H-Blank IE
				IOIF |= LCDC_IF;
			scanline();
		}
		break;

	case V_M0: 							// Mode 0 H-Blank
		if (video_cycles > V_M0_CYCLE) {
			video_cycles -= V_M0_CYCLE;
			nextLine();			// Finish processing scanline, go to next one
			if (IOLY < SCREEN_H) {
				IOSTAT |= V_M2;     	// Next, Mode 2 searching oam
				if (IOSTAT & OAM_IE)
					IOIF |= LCDC_IF;
			}
			else {
				IOSTAT |= V_M1;     	// Next, Mode 1 V-blank
				IOIF |= V_BLANK_IF;
				if (IOSTAT & VB_IE)
					IOIF |= LCDC_IF;
			}
		}
		break;

	case V_M1: 							// Mode 1 V-blank 10 lines
		if (video_cycles > V_LINE_CYCLE)
		{
			video_cycles -= V_LINE_CYCLE;
			nextLine();
			if (IOLY < (SCREEN_H + VBLANK_LINES))
				return frame;

			IOSTAT &= ~(V_MODE_MASK); 	// Next, Mode 2 searching oam
			IOSTAT |= V_M2;

			IOLY = 0;
			frame = ON;
			LCD_Window(0, 0, SCREEN_W, SCREEN_H);
		}
		break;
	}
	return frame;
}
