#include "dmgcpu.h"
#include "video.h"
#include "lcd.h"


static uint16_t video_cycles = 0;

const unsigned short lcd_pal[]={0xE7DA,0x8E0E,0x334A,0x08C4};

Sprite *spriteline[MAX_SPRITES/sizeof(Sprite)];
uint8_t bgdataline[40];
uint8_t scanlinedata[160];		// one line of pixels
unsigned char nsprites;


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
void putTileData(TileData *tl, uint8_t *dst) {
	uint8_t i = TILE_W;
	uint8_t color, pixel;

	pixel = 7 - (IOSCX & TILE_LINE_MASK);

	while (i--) {
		color = (tl->line[IOLY & TILE_LINE_MASK].msb >> pixel) & 1;
		color <<= 1;
		color |= (tl->line[IOLY & TILE_LINE_MASK].lsb >> pixel) & 1;
		color = (IOBGP >> (color << 1)) & 3;
		if (!*dst)		// only put if color from sprite is transparent 
			*dst = color;
		pixel--;
		if (pixel == 255) {
			tl++;
			pixel = 7;
		}
		dst += 1;
	}
}
//-----------------------------------------
// read OBJECT Attribute Memory for one line
//-----------------------------------------
void scanOAM(){
uint8_t i, n, tileline = (IOLY + 16) >> 3;	// Y position has a offset of 16pixels
Sprite *poam = (Sprite*)&oam[0];
Sprite **ps = spriteline;

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
	uint8_t *bgtilepattern;
	uint8_t pixel, tileindex;

	// Get tile map base
	bgtilepattern = (uint8_t*)(vram + ((IOLCDC & BG_MAP) ? TILE_MAP1_BASE : TILE_MAP0_BASE));
	// Add line and scroll offset for getting tile pattern
	bgtilepattern += (((IOLY >> 3) * BG_H_TILES) + ((IOSCY >> 3) * BG_H_TILES)) & BG_SIZE_MASK;

	if (IOLCDC & BG_W_DATA) {							 
		for (tileindex = 0; tileindex < SCREEN_H_TILES; tileindex++, bgtilepattern++ ) {
			putTileData((TileData*)(vram) + *bgtilepattern, scanlinedata + (tileindex * TILE_W));
		}
	}
	else {
		for (tileindex = 0; tileindex < SCREEN_H_TILES; tileindex++, bgtilepattern++) {
			putTileData((TileData*)(vram + TILE_DATA1_SIGNED_BASE) + (int8_t)*bgtilepattern, scanlinedata + (tileindex * TILE_W));
		}
	}

	for (pixel= 0; pixel < SCREEN_W; pixel++) {	
		LCD_Data(lcd_pal[scanlinedata[pixel]]);
	}	

}
//-----------------------------------------
//
//-----------------------------------------
void lycIrq(void)
{
	if(IOLY == IOLYC)		
		IOSTAT |= LYC_LY_FLAG; 
	else
		IOSTAT &= ~LYC_LY_FLAG;
		
	if(IOSTAT & (OAM_IE | LYC_LY_FLAG))
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
				IOLY++;
				if(IOLY > (SCREEN_H-1)){				
					IOSTAT |= V_M1;     	// Next, Mode 1 V-blank
					IOIF |= V_BLANK_IF;						
					if(IOSTAT & VB_IE)
						IOIF |= STAT_IF;
				}
				else{						// Finish processing scanline, go to next one
					IOSTAT |= V_M2;     	// Next, Mode 2 searching oam					
				 	lycIrq();
				}
			}
			break;
			
		case V_M1: 							// Mode 1 V-blank 10 lines
			if(video_cycles > V_LINE_CYCLE)
			{
				video_cycles -= V_LINE_CYCLE;
				IOLY++;									
				lycIrq();
				
				if(IOLY < (SCREEN_H + VBLANK_LINES))
					return;
			
				IOSTAT &= ~(V_MODE_MASK); 	// Next, Mode 2 searching oam
				IOSTAT |= V_M2;
				
				lycIrq();
				IOLY = 0;					
				LCD_Window(0, 0, SCREEN_W, SCREEN_H);
			}
			break;		
	}
}
