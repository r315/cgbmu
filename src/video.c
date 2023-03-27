
#include <string.h>
#include "dmgcpu.h"
#include "video.h"
#include "board.h"
#include "liblcd.h"
#include "cgbmu.h"


static uint16_t video_cycles = 0;
static Object *visible_objs[MAX_LINE_OBJECTS];
uint8_t scanlinedata[SCREEN_W];		// one line of pixels

//-----------------------------------------
//put one line of Sprite data into scanlinedata
//-----------------------------------------
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
			if (color) {
				color = (pal >> (color << 1)) & 3;
				if(!(obj->flags & OBJECT_FLAG_PRIO) || !*dst)
					*dst = color;
			}
			msb >>= 1;
			lsb >>= 1;
			dst++;
		}while (--npixels);
	}
	else {
		do{
			color = (msb & 0x80) ? 2 : 0;
			color |= (lsb & 0x80) ? 1 : 0;
			if (color) { // Color index 0 is transparent
				color = (pal >> (color << 1)) & 3;
				if (!(obj->flags & OBJECT_FLAG_PRIO) || !*dst)
					*dst = color;
			}
			msb <<= 1;
			lsb <<= 1;
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
// Scan for visible objects
//-----------------------------------------
void scanOAM() {
	uint8_t h, n, curline, lcdc;
	Object *pobj = (Object*)oam;

	lcdc = IOLCDC;

	if (!(lcdc & OBJ_DISPLAY)) {
		visible_objs[0] = NULL;
		return;
	}

	curline = IOLY + 16;	// Y position has a offset of 16pixels

	n = 0;
	h = (lcdc & OBJ_SIZE) ? SPRITE_H << 1 : SPRITE_H; // 8x16 objs

	for (int i = 0; i < MAX_OBJECTS; i++, pobj++) {
		if (pobj->x == 0 || pobj->y == 0) continue;
		if (pobj->y > curline || (pobj->y + h) <= curline) continue;
		if (pobj->x >= SCREEN_W + SPRITE_W)continue;		
		
		visible_objs[n] = pobj;	
		if (++n >= MAX_LINE_OBJECTS)
			break;
	}

	visible_objs[n] = NULL;
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

	memset(sld, 0, SCREEN_W);
	blitTileData(tilemapline, sld, IOSCX, line, SCREEN_W);	

	if (IOLCDC & W_DISPLAY && IOLY >= IOWY && IOWX < SCREEN_W + 7) 
	{
		line = IOLY - IOWY;					
		sld = scanlinedata + IOWX - 7;				//destination offset given by IOWX, WX has an offset of 7
		tilemapline = (uint8_t*)(vram + ((IOLCDC & W_MAP) ? TILE_MAP1_BASE : TILE_MAP0_BASE));
		tilemapline += TILE_LINE_INDEX(line);
		blitTileData(tilemapline, sld, 0, line, SCREEN_W - IOWX + 7);
	}

	pixel = 0;
	while (visible_objs[pixel] != NULL)
		blitObjectData(visible_objs[pixel++], scanlinedata);

	pushScanLine(scanlinedata);
}
//-----------------------------------------
// Increment line count
// Set coincidence on STAT if  LY = LYC
// and set LCDC_IF if IME and interrupt is enabled on STAT
//-----------------------------------------
void nextLine(void) {
	uint8_t stat = IOSTAT & (~LYC_LY_FLAG);
	uint8_t ly = IOLY + 1;

	if(ly == IOLYC){
		stat |= LYC_LY_FLAG;
		if(IME)
			IOIF |= LCDC_IF;
	}

	IOSTAT = stat;
	IOLY = ly;
}
//-----------------------------------------
// return true if frame is starting
//-----------------------------------------
uint8_t video(void) {

	if (!(IOLCDC & LCD_DISPLAY)) 
		return false;

	video_cycles += instr_cycles;

	uint8_t stat = IOSTAT;

	switch (stat & V_MODE_MASK)
	{
	case V_M2: 							// Mode 2 oam access start scanline	
		if (video_cycles >= V_M2_CYCLE)
		{
			video_cycles -= V_M2_CYCLE;
			stat |= V_M3;				// Next, Mode 3 vram access
			scanOAM();
		}
		break;

	case V_M3: 							// Mode 3 vram access
		if (video_cycles >= V_M3_CYCLE)
		{
			video_cycles -= V_M3_CYCLE;
			stat &= ~(V_MODE_MASK);     // Next, Mode 0 H-blank				 
			if (stat & HB_IE) 		    // LCD STAT & H-Blank IE
				IOIF |= LCDC_IF;
			scanline();
		}
		break;

	case V_M0: 							// Mode 0 H-Blank
		if (video_cycles >= V_M0_CYCLE) {
			video_cycles -= V_M0_CYCLE;
			nextLine();			        // Finish processing scanline, go to next one
			if (IOLY < SCREEN_H) {
				stat |= V_M2;     	    // Next, Mode 2 searching oam
				if (stat & OAM_IE)
					IOIF |= LCDC_IF;
			}
			else {
				stat |= V_M1;     	    // Next, Mode 1 V-blank
				IOIF |= V_BLANK_IF;
				if (stat & VB_IE)
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
				return false;

			IOSTAT = (stat & ~V_MODE_MASK) | V_M2; 	// Next, Mode 2 searching oam
			IOLY = 0;
			return true;
		}
		break;
	}

	IOSTAT = stat;
	return false;
}
