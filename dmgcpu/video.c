
#include <string.h>
#include "dmgcpu.h"
#include "video.h"
#include "lcd.h"

const unsigned short lcd_pal[] = { 0xE7DA,0x8E0E,0x334A,0x08C4 };

uint16_t video_cycles = 0;
Object *spriteline[MAX_OBJECTS / sizeof(Object)];
uint8_t bgdataline[40];
uint8_t scanlinedata[160];		// one line of pixels
Object *objs[MAX_LINE_OBJECTS];

//-----------------------------------------
//put one line of Sprite data into scanlinedata
//-----------------------------------------
void putObjectData(Object *obj, uint8_t *dst) {
	uint8_t i = SPRITE_W, color, line;
	uint8_t pal = (obj->flags & OBJECT_FLAG_PAL) ? IOOBP1 : IOOBP0;
	//Each Tile has 16bytes and one line os pixels neads 2 bytes
	TileData *td = (TileData*)vram + obj->pattern;
	// Add line offset, 2byte per line
	line = (obj->flags & OBJECT_FLAG_YFLIP) ? (7 - TILE_LINE(IOLY)) : TILE_LINE(IOLY);

	uint8_t lsb = td->line[line].lsb;
	uint8_t msb = td->line[line].msb;

	dst += obj->x - SPRITE_W;

	if (obj->flags & OBJECT_FLAG_XFLIP) {		
		while (i--) {
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
		}
	}
	else {
		while (i--) {
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
void putTileData(uint8_t *tilemapline, uint8_t *dst, uint8_t pixeloffset, uint8_t line, uint8_t size) {
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
void scanOAM() {
	uint8_t i, n, objline = IOLY + 16;	// Y position has a offset of 16pixels
	Object *pobj = (Object*)oam;

	n = 0;
	for (i = 0; i < MAX_OBJECTS; i++, pobj++) {
		if (pobj->x >= SPRITE_W && pobj->x < SCREEN_W + SPRITE_W) {
			if (IOLCDC & OBJ_SIZE) {	// 8x16 objs
				if (objline >= pobj->y && objline < (pobj->y + (SPRITE_H * 2))) {
					objs[n] = pobj;
					n++;
				}
			}
			else {
				if (objline >= pobj->y && objline < (pobj->y + SPRITE_W)) {
					objs[n] = pobj;
					n++;
				}
			}
		}		
		if (n >= MAX_LINE_OBJECTS)
			break;
	}
	objs[n] = NULL;
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
	line = (uint8_t)(IOLY + IOSCY);
	tilemapline += TILE_LINE_INDEX(line);

	memset(scanlinedata, 0, sizeof(scanlinedata));
	putTileData(tilemapline, sld, IOSCX, line, SCREEN_W);	

	if (IOLCDC & W_DISPLAY && IOLY >= IOWY && IOWX < SCREEN_W + 7) 
	{
		line = IOLY - IOWY;					
		sld = scanlinedata + IOWX - 7;				//destination offset given by IOWX, WX has an offset of 7
		tilemapline = (uint8_t*)(vram + ((IOLCDC & W_MAP) ? TILE_MAP1_BASE : TILE_MAP0_BASE));
		tilemapline += TILE_LINE_INDEX(line);
		putTileData(tilemapline, sld, 0, line, SCREEN_W - IOWX + 7);
	}

	pixel = 0;
	while (objs[pixel] != NULL)
		putObjectData(objs[pixel++], scanlinedata);

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
	//if ((IOSTAT & LYC_LY_IE) && (IOSTAT & LYC_LY_FLAG))
	if (IOSTAT & LYC_LY_FLAG)
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
