#include "dmgcpu.h"
#include "video.h"
#include "lcd.h"


static uint16_t video_cycles = 0;

const unsigned short lcd_pal[]={0xE7DA,0x8E0E,0x334A,0x08C4};
//const unsigned short pal[]={0x08C4,0x334A,0x8E0E,0xE7DA};
Sprite *spriteline[MAX_SPRITES/sizeof(Sprite)];
uint8_t bgdataline[40];
uint8_t spritedataline[160];
unsigned char nsprites;

//-----------------------------------------
//read OBJECT Attribute Memory for one line
//-----------------------------------------
void putSpriteData(Sprite *sp, uint8_t *dst){
uint8_t i = 8, color;
uint8_t pal = (sp->flags & SPRITE_FLAG_PAL) ? IOOBP1 : IOOBP0;
//Each Tile has 16bytes and one line os pixels neads 2 bytes
uint8_t *p  = (uint8_t*)(vram + (sp->pattern * sizeof(Tile)));	//get tiledata base
 p += ((sp->flags & SPRITE_FLAG_YFLIP) ? (7 - (IOLY & TILE_LINE_MASK)) : (IOLY & TILE_LINE_MASK)) << 1; // add line offset, 2byte per line
uint8_t lsb = *p++;
uint8_t msb = *p;

	if(sp->flags & SPRITE_FLAG_XFLIP){
		while(i--){
			color = (msb >> (7 - i)) & 1;
			color <<=1;
			color |= (lsb >> (7 - i)) & 1;
			if(color)
				*dst = (pal>>(color<<1)) & 3;
			dst += 1;		
		}
	}else{
		while(i--){
			color = (msb >> i) & 1;
			color <<=1;
			color |= (lsb >> i) & 1;
			if(color)
				*dst = (pal>>(color<<1)) & 3;
			dst += 1;		
		}
	}	
}
//-----------------------------------------
void scanOAM(){
uint8_t i, n, tileline = (IOLY + 16) >> 3;	// Y position has a offset of 16pixels
Sprite *poam = (Sprite*)&oam[0];
Sprite **ps = spriteline;

	memset(spritedataline, 0, sizeof(spritedataline));
	
	n = 0;
	for (i = 0; i < MAX_SPRITES; i++){
		if( tileline == (poam->y >> 3) ){
			if(poam->x >= SPRITE_W && poam->x < SCREEN_W + SPRITE_W){			
				putSpriteData(poam, spritedataline + poam->x - 8);
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
uint8_t *psd = spritedataline;
uint8_t pixel;


for (pixel= 0; pixel < SCREEN_W; pixel++, psd++ ) {	
	LCD_Data(lcd_pal[*psd]);
}

}
//-----------------------------------------
//
//-----------------------------------------
void _scanline()
{
uint8_t *bgtilemap;
uint8_t *windowtilemap;
Tile *bgtiledata;
uint8_t msb,lsb,pixel, pixelindex;
uint8_t color;
uint8_t line, sdata;
Sprite **ps;

bgtilemap = (uint8_t*)(vram + ((IOLCDC & BG_MAP) ?  TILE_MAP1_BASE : TILE_MAP0_BASE));
windowtilemap = (uint8_t*)(vram + ((IOLCDC & W_MAP) ? TILE_MAP1_BASE : TILE_MAP0_BASE));

// skip 8 pixel (tile height) from current scanline and add XY scroll
// clip to 32 x 32 tiles
bgtilemap += (((IOLY >> 3) * BG_H_TILES) + ((IOSCY >> 3) * BG_H_TILES) + (IOSCX >> 3)) & BG_SIZE_MASK;
// window is not scrolable add only scanline
//windowtilemap += (((IOLY - IOWY) >> 3) * BG_H_TILES);

line = IOLY & 7;
pixel = IOSCX;
if(IOLCDC & BG_W_DATA){ // background and window Tile Data Select
	//Tile data stored at 8000-8FFF	
	for (pixelindex = 0; pixelindex < SCREEN_W; ) {
		bgtiledata = (Tile*)vram + *bgtilemap++;
		lsb = bgtiledata->line[line].lsb;
		msb = bgtiledata->line[line].msb;
		sdata = spritedataline[pixelindex>>2];
		do{
			color  = (msb & (0x80 >> (pixel & 7))) ? 1 : 0;
			color |= (lsb & (0x80 >> (pixel & 7))) ? 2 : 0;
			color |= sdata & 3;
			LCD_Data(lcd_pal[(IOBGP>>(color << 1)) & 3]);
			pixel++; pixelindex++;
			sdata >>= 2; 
		}while((pixel & 7) != 0 && pixelindex < SCREEN_W);
	}
}
else{
	//Tile data stored at 8800-97FF, signed base 0x9000		
	for (pixelindex = 0; pixelindex < SCREEN_W; ) {
		bgtiledata = (Tile*)(vram + TILE_DATA1_SIGNED_BASE) + (int8_t)(*bgtilemap++);				
		lsb = bgtiledata->line[line].lsb;
		msb = bgtiledata->line[line].msb;
		sdata = spritedataline[pixelindex>>2];
		do {
			color = (msb & (0x80 >> (pixel & 7))) ? 1 : 0;
			color |= (lsb & (0x80 >> (pixel & 7))) ? 2 : 0;
			color |= sdata & 3;
			LCD_Data(lcd_pal[(IOBGP >> (color << 1)) & 3]);
			pixel++; pixelindex++;
			sdata >>= 2;
		} while ((pixel & 7) != 0 && pixelindex < SCREEN_W);
	}
	
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
