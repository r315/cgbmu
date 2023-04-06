
#include <string.h>
#include "dmgcpu.h"
#include "video.h"
#include "cgbmu.h"



//-----------------------------------------
//put one line of Sprite data into scanlinedata
//-----------------------------------------
void blitObjectData(cpu_t *cpu, obj_t *obj, uint8_t *dst) {
	uint8_t pixel_mask, color, objline;
	uint8_t *pal = (obj->flags & OBJECT_FLAG_PAL) ? cpu->obj1pal : cpu->obj0pal;
	uint8_t pattern = obj->pattern;
	uint8_t *end, *start;

	//Get pattern, for 8x16 mode the extra data is on the next pattern index
	if (cpu->IOLCDC & OBJ_SIZE) {
		objline = SPRITE_8x16_LINE_MASK(cpu->IOLY - (obj->y - SPRITE_Y_OFFSET));
		objline = (obj->flags & OBJECT_FLAG_YFLIP) ? (SPRITE_8x16_H_MASK - 1 - objline) : objline;
	}
	else {
		// Add line offset, 2byte per line
		objline = TILE_LINE((cpu->IOLY - (obj->y - SPRITE_Y_OFFSET)));
		objline = (obj->flags & OBJECT_FLAG_YFLIP) ? (SPRITE_H - 1 - objline) : objline;
	}

	//Each Tile has 16bytes and one line of pixels needs 2 bytes
	tiledata_t *td = (tiledata_t*)cpu->vram + pattern;
	uint8_t lsb = td->line[objline].lsb;
	uint8_t msb = td->line[objline].msb;

	if (obj->flags & OBJECT_FLAG_XFLIP) {
		if (obj->x < SPRITE_W) {
			pixel_mask = 0x1 << (SPRITE_W - obj->x);
			start = dst;
			end = start + obj->x;
		}
		else if (obj->x > SCREEN_W) {
			pixel_mask = 0x1;
			start = dst + (obj->x - SPRITE_W);
			end = dst + SCREEN_W;
		}
		else {
			pixel_mask = 0x1;
			start = dst + (obj->x - SPRITE_W);
			end = start + SPRITE_W;
		}
		do{
			color = (msb & pixel_mask) ? 2 : 0;
			color |= (lsb & pixel_mask) ? 1 : 0;
			if (color) {
				if(!(obj->flags & OBJECT_FLAG_PRIO) || !*start)
					*start = pal[color];
			}
			pixel_mask <<= 1;
			start++;
		}while(start < end);
	}
	else {
		if (obj->x < SPRITE_W) {
			// Clip sprite to screen start
			pixel_mask = 0x80 >> (SPRITE_W - obj->x);
			start = dst;
			end = start + obj->x;
		}
		else if(obj->x > SCREEN_W){
			// Clip sprite to screen end
			pixel_mask = 0x80;
			start = dst + (obj->x - SPRITE_W);
			end = dst + SCREEN_W;
		}
		else {
			// No clip
			pixel_mask = 0x80;
			start = dst + (obj->x - SPRITE_W);
			end = start + SPRITE_W;
		}
		do{
			color = (msb & pixel_mask) ? 2 : 0;
			color |= (lsb & pixel_mask) ? 1 : 0;
			if (color) { // Color index 0 is transparent				
				if (!(obj->flags & OBJECT_FLAG_PRIO) || !*start)
					*start = pal[color];
			}
			pixel_mask >>= 1;
			start++;
		}while(start < end);
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
void blitTileData(cpu_t *cpu, uint8_t *tilemapline, uint8_t *dst, uint8_t pixeloffset, uint8_t line, uint8_t size) {
	uint8_t tileindex, msb, lsb, color;
	tile_t *td;

	line = TILE_LINE(line);				// Mask line in tile 

	while (size) {
		tileindex = *(tilemapline + TILE_INDEX(pixeloffset));  // add pixel offset with wraparround
		td = (cpu->IOLCDC & BG_W_DATA) ? (tile_t*)(cpu->vram) + tileindex : (tile_t*)(cpu->vram + TILE_DATA1_SIGNED_BASE) + (int8_t)tileindex;
		msb = td->line[line].msb;
		lsb = td->line[line].lsb;
		msb <<= TILE_LINE(pixeloffset);
		lsb <<= TILE_LINE(pixeloffset);

		do {			
			color = (msb & 0x80) ? 2 : 0;
			color |= (lsb & 0x80) ? 1 : 0;
			msb <<= 1;
			lsb <<= 1;
			color = (cpu->IOBGP >> (color << 1)) & 3;
			*(dst++) = color;	
			pixeloffset++;
			size--;
		} while (TILE_PIXEL(pixeloffset) != 0 && size != 0);
	}
}

//-----------------------------------------
// Scan OAM for visible objects
//-----------------------------------------
void scanOAM(cpu_t *cpu) {
	uint8_t h, n, curline, lcdc;
	obj_t *pobj = (obj_t*)cpu->oam;

	lcdc = cpu->IOLCDC;

	if (!(lcdc & OBJ_DISPLAY)) {
		cpu->visible_objs[0] = NULL;
		return;
	}

	curline = cpu->IOLY + 16;	// Y position has a offset of 16 pixel

	n = 0;
	h = (lcdc & OBJ_SIZE) ? SPRITE_H << 1 : SPRITE_H; // 8x16 objs

	for (int i = 0; i < MAX_OBJECTS; i++, pobj++) {
		if (pobj->x == 0 || pobj->y == 0) continue;
		if (pobj->y > curline || (pobj->y + h) <= curline) continue;
		if (pobj->x >= SCREEN_W + SPRITE_W)continue;
		
		cpu->visible_objs[n] = pobj;	
		if (++n >= MAX_LINE_OBJECTS)
			break;
	}

	cpu->visible_objs[n] = NULL;
}
//-----------------------------------------
//
//-----------------------------------------
void scanline(cpu_t *cpu) {
	uint8_t *tilemapline;
	uint8_t line;
	uint8_t *sld = cpu->screen_line;

	uint8_t lcdc = cpu->IOLCDC;
	uint8_t ly = cpu->IOLY;

	// Get tile map base
	tilemapline = (uint8_t*)(cpu->vram + ((lcdc & BG_MAP) ? TILE_MAP1_BASE : TILE_MAP0_BASE));
	// Add line and scroll-y offset for getting tile pattern	
	line = (uint8_t)(ly + cpu->IOSCY);
	tilemapline += TILE_LINE_INDEX(line);

	memset(sld, 0, SCREEN_W);
	blitTileData(cpu, tilemapline, sld, cpu->IOSCX, line, SCREEN_W);
	// Draw window if active and visible
	if (lcdc & W_DISPLAY && ly >= cpu->IOWY && cpu->IOWX < SCREEN_W + 7)
	{
		line = ly - cpu->IOWY;						// Calculate y offset from start of window
		sld = cpu->screen_line + cpu->IOWX - 7;		// Calculate x offset from start of window, WX has an offset of 7
		tilemapline = (uint8_t*)(cpu->vram + ((lcdc & W_MAP) ? TILE_MAP1_BASE : TILE_MAP0_BASE));
		tilemapline += TILE_LINE_INDEX(line);
		blitTileData(cpu, tilemapline, sld, 0, line, SCREEN_W - cpu->IOWX + 7);
	}

	uint32_t idx = 0;
	while (cpu->visible_objs[idx] != NULL)
		blitObjectData(cpu, cpu->visible_objs[idx++], cpu->screen_line);

	pushScanLine(cpu);
}

//-----------------------------------------
// 
//-----------------------------------------
void writeLCDC(cpu_t *cpu, uint8_t newlcdc){
	if(cpu->IOLCDC & 0x80 && !(newlcdc & 0x80)){
		cpu->IOLY= 0; // LCD Off
		cpu->IOSTAT = cpu->IOSTAT & 0xFC;  // reset LY, mode0
	}
	else if (!(cpu->IOLCDC & 0x80) && (newlcdc & 0x80)) {
		checkLine(cpu);
	}

	cpu->video_cycles = 0;
	cpu->IOLCDC = newlcdc;
}

void writeSTAT(cpu_t *cpu, uint8_t newstat){
	newstat = (newstat & ~7) | 0x80;
	cpu->IOSTAT = (cpu->IOSTAT & 7) | newstat;
}

void writeLYC(cpu_t *cpu, uint8_t newlyc){
	cpu->IOLYC = newlyc;
	checkLine(cpu);
}

void checkLine(cpu_t *cpu) {
	cpu->IOSTAT &= ~LYC_LY_FLAG;

	if(cpu->IOLY == cpu->IOLYC){
		cpu->IOSTAT |= LYC_LY_FLAG;
		if (cpu->IOSTAT & LYC_LY_IE) {			
			setInt(cpu, LCDC_IF);
		}
	}
}
//-----------------------------------------
// return true if frame is starting
//-----------------------------------------
uint8_t video(cpu_t *cpu) {

	if (!(cpu->IOLCDC & LCD_DISPLAY))
		return 0;

	cpu->video_cycles += cpu->instr_cycles;

	switch (cpu->IOSTAT & V_MODE_MASK)
	{
	case V_M2: 							// Mode 2 oam access start scanline	
		if (cpu->video_cycles >= V_M2_CYCLE)
		{
			cpu->video_cycles -= V_M2_CYCLE;
			cpu->IOSTAT |= V_M3;				// Next, Mode 3 vram access
			scanOAM(cpu);
		}
		break;

	case V_M3: 							// Mode 3 vram access
		if (cpu->video_cycles >= V_M3_CYCLE)
		{
			cpu->video_cycles -= V_M3_CYCLE;
			cpu->IOSTAT &= ~(V_MODE_MASK);     // Next, Mode 0 H-blank
			if (cpu->IOSTAT & HB_IE) 		    // LCD STAT & H-Blank IE
				setInt(cpu, LCDC_IF);
			scanline(cpu);
		}
		break;

	case V_M0: 							// Mode 0 H-Blank
		if (cpu->video_cycles >= V_M0_CYCLE) {
			cpu->video_cycles -= V_M0_CYCLE;
			cpu->IOLY++;
			checkLine(cpu);		// Finish processing scanline, go to next one
			if (cpu->IOLY < SCREEN_H) {
				cpu->IOSTAT |= V_M2;     	    // Next, Mode 2 searching oam
				if (cpu->IOSTAT & OAM_IE)
					setInt(cpu, LCDC_IF);
			}
			else {
				cpu->IOSTAT |= V_M1;           // Next, Mode 1 V-blank
				setInt(cpu, V_BLANK_IF);
				if (cpu->IOSTAT & VB_IE)
					setInt(cpu, LCDC_IF);
			}
		}
		break;

	case V_M1: 							// Mode 1 V-blank 10 lines
		if (cpu->video_cycles > V_M1_CYCLE)
		{
			cpu->video_cycles -= V_M1_CYCLE;
			if (cpu->IOLY > (SCREEN_H + VBLANK_LINES)) {
				cpu->IOLY = 0;
				checkLine(cpu);
				cpu->IOSTAT = (cpu->IOSTAT & ~V_MODE_MASK) | V_M2; 	// Next, Mode 2 searching oam
				return 1;
			}
			cpu->IOLY++;
			checkLine(cpu);
			return 0;
		}
		break;
	}

	return 0;
}
