#include "dmgcpu.h"
#include "video.h"
#include "lcd.h"


uint16_t video_cycles = 0;
uint8_t frame;

const unsigned short lcd_pal[]={0xE7DA,0x8E0E,0x334A,0x08C4};

Sprite *spriteline[MAX_SPRITES/sizeof(Sprite)];
uint8_t bgdataline[40];
uint8_t scanlinedata[160];		// one line of pixels


//-----------------------------------------
//put one line of Sprite data into scanlinedata
//-----------------------------------------
void putSpriteData(Sprite *sp, uint8_t *dst){
uint8_t i = SPRITE_W, color;
uint8_t pal = (sp->flags & SPRITE_FLAG_PAL) ? IOOBP1 : IOOBP0;
//Each Tile has 16bytes and one line os pixels neads 2 bytes
uint8_t *p  = (uint8_t*)(vram + (sp->pattern * sizeof(TileData)));	//get tiledata base
// add line offset, 2byte per line
p += ((sp->flags & SPRITE_FLAG_YFLIP) ? (7 - (IOLY & TILE_LINE_MASK)) : (IOLY & TILE_LINE_MASK)) << 1; 
uint8_t lsb = *p++;
uint8_t msb = *p;

	if(sp->flags & SPRITE_FLAG_XFLIP){
		while(i--){
			color = (msb >> (7 - i)) & 1;
			color <<=1;
			color |= (lsb >> (7 - i)) & 1;
			color = (pal >> (color << 1)) & 3;  // pal uses 2 bit per color
			if (color)
				*dst = color;
			dst += 1;		
		}
	}else{
		while(i--){
			color = (msb >> i) & 1;
			color <<=1;
			color |= (lsb >> i) & 1;
			color = (pal >> (color << 1)) & 3;
			if (color)
				*dst = color;
			dst += 1;		
		}
	}	
}
//-----------------------------------------
//put one line of tile data into scanlinedata
//-----------------------------------------
void putTileData(uint8_t *mapline, uint8_t *pixel, uint8_t *dst) {
	uint8_t i = TILE_W;
	uint8_t color;
	TileData *td;	

	while (i) {
		if (IOLCDC & BG_W_DATA) {	
			td = (TileData*)(vram) + *(mapline + TILE_INDEX(*pixel));
		}else{
			td = (TileData*)(vram + TILE_DATA1_SIGNED_BASE) + *((int8_t*)(mapline + TILE_INDEX(*pixel)));
		}
		do{
			color = td->line[TILE_LINE(IOLY)].msb;
			color >>= (7 - TILE_LINE(*pixel));
			color <<= 1;
			color &= 2;
			uint8_t t = td->line[TILE_LINE(IOLY)].lsb;
			t >>= (7 - TILE_LINE(*pixel));
			t &= 1;
			color |= t;
			color = (IOBGP >> (color << 1)) & 3;
			if (!*dst)		// only put if color from sprite is transparent 
				*dst = color;
			dst += 1;
			(*pixel)++;
			i--;
		}while( TILE_LINE(*pixel) != 0 && i != 0);
	}
}
//-----------------------------------------
// read OBJECT Attribute Memory for one line
//-----------------------------------------
void scanOAM(){
uint8_t i, n, tileline = (IOLY + 16) >> 3;	// Y position has a offset of 16pixels
Sprite *poam = (Sprite*)&oam[0];

	memset(scanlinedata, 0, sizeof(scanlinedata));
	
	n = 0;
	for (i = 0; i < MAX_SPRITES; i++){
		if( tileline == (poam->y >> 3) ){
			if(poam->x >= SPRITE_W && poam->x < SCREEN_W + SPRITE_W){			
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
void scanline(){
	uint8_t *bgmapline;
	uint8_t pixel, tileindex;
	uint8_t *sld = scanlinedata;

	// Get tile map base
	bgmapline = (uint8_t*)(vram + ((IOLCDC & BG_MAP) ? TILE_MAP1_BASE : TILE_MAP0_BASE));
	// Add line and scroll offset for getting tile pattern
	bgmapline += (TILE_LINE_INDEX(IOLY) + TILE_LINE_INDEX(IOSCY)) & BG_SIZE_MASK;

	pixel = IOSCX;
	
	for (tileindex = 0; tileindex < SCREEN_H_TILES; tileindex++, sld += TILE_W) {
		putTileData(bgmapline, &pixel, sld);
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
void nextLine(void){
	IOLY++;
	if(IOLY == IOLYC)
		IOSTAT |= LYC_LY_FLAG; 
	else
		IOSTAT &= ~LYC_LY_FLAG;	
	if (IOSTAT & LYC_LY_IE)			//BUG: Changing LYC_LY_IE to  LYC_LY_FLAG corrects scroll..
		IOIF |= STAT_IF;
}
//-----------------------------------------
//
//-----------------------------------------
void video(void){	
	if(!(IOLCDC & LCD_DISPLAY)) return; 	// Lcd controller off	
		
	video_cycles += GET_CYCLE();		
		 	
	switch(IOSTAT & V_MODE_MASK)
	{
		case V_M2: 							// Mode 2 oam access start scanline	
			if(video_cycles > V_M2_CYCLE)
			{							
				video_cycles -= V_M2_CYCLE;
				IOSTAT |= V_M3;				// Next, Mode 3 vram access				
				scanOAM(); 
			}
			break;
			
		case V_M3: 							// Mode 3 vram access
			if(video_cycles > V_M3_CYCLE)
			{
				video_cycles -= V_M3_CYCLE;
				 IOSTAT &= ~(V_MODE_MASK);  // Next, Mode 0 H-blank				 
				 if(IOSTAT & HB_IE) 		// LCD STAT & H-Blank IE
				 	IOIF |= STAT_IF;
			 	scanline();				 		
			}
			break;
			
		case V_M0: 							// Mode 0 H-Blank
			if(video_cycles > V_M0_CYCLE){
				video_cycles -= V_M0_CYCLE;				
				nextLine();			// Finish processing scanline, go to next one
				if(IOLY < SCREEN_H){				
					IOSTAT |= V_M2;     	// Next, Mode 2 searching oam
					if (IOSTAT & OAM_IE)
						IOIF |= STAT_IF;
				}
				else{						
					IOSTAT |= V_M1;     	// Next, Mode 1 V-blank
					IOIF |= V_BLANK_IF;						
					if(IOSTAT & VB_IE)
						IOIF |= STAT_IF;
				}
			}
			break;
			
		case V_M1: 							// Mode 1 V-blank 10 lines
			if(video_cycles > V_LINE_CYCLE)
			{
				video_cycles -= V_LINE_CYCLE;								
				nextLine();				
				if(IOLY < (SCREEN_H + VBLANK_LINES))
					return;
			
				IOSTAT &= ~(V_MODE_MASK); 	// Next, Mode 2 searching oam
				IOSTAT |= V_M2;
				
				IOLY = 0;
				frame = ON;
				LCD_Window(0, 0, SCREEN_W, SCREEN_H);
			}
			break;		
	}
}
