#include "dmgcpu.h"
#include "video.h"
#include "lcd.h"


static uint16_t video_cycles = 0;

const unsigned short pal[]={0xE7DA,0x8E0E,0x334A,0x08C4};
//const unsigned short pal[]={0x08C4,0x334A,0x8E0E,0xE7DA};
Sprite *spriteline[MAX_SPRITES/sizeof(Sprite)]; // max 10 sprites
unsigned char nsprites;

//-----------------------------------------
//read OBJECT Attribute Memory for one line
//-----------------------------------------
void scanOAM(){
uint8_t i, n, tileline = IOLY + (SPRITE_H * 2);	// Y position has a offset of 16
Sprite *poam = (Sprite*)&oam[0];
Sprite **ps = spriteline;

	tileline >>= 3; 							// skip Tile individual lines
	n = 0;
	for (i = 0; i < MAX_SPRITES; i++){
        if( tileline == (poam->y >> 3) ){
            if(poam->x >= SPRITE_W && poam->x < SCREEN_W + SPRITE_W){			
                *ps = poam;
                ps += 1;
				n++;
			}
		}
        poam += 1;
		if (n >= MAX_LINE_SPRITES)
			break;
	}
	*ps = '\0';         						// mark end of array
}

//-----------------------------------------
//
//-----------------------------------------
void scanline()
{
uint8_t *bgtilemap;
uint8_t *windowtilemap;
Tile *bgtiledata;
uint8_t msb,lsb,pixel, pixelindex;
uint8_t color;
uint8_t line;
Sprite **ps;

bgtilemap = (uint8_t*)(vram + ((IOLCDC & BG_MAP) ?  TILE_MAP1_BASE : TILE_MAP0_BASE));
windowtilemap = (uint8_t*)(vram + ((IOLCDC & W_MAP) ? TILE_MAP1_BASE : TILE_MAP0_BASE));

// skip 8 pixel (tile height) from current scanline and add XY scroll
// clip to 32 x 32 tiles
bgtilemap += (((IOLY >> 3) * BG_H_TILES) + ((IOSCY >> 3) * BG_H_TILES) + (IOSCX >> 3)) & BG_SIZE_MASK;
// window is not scrolable add only scanline
//windowtilemap += (((IOLY - IOWY) >> 3) * BG_H_TILES);

line = IOLY & 7;
if(IOLCDC & BG_W_DATA){ // background and window Tile Data Select
	//Tile data stored at 8000-8FFF
	pixel = IOSCX;
	for (pixelindex = 0; pixelindex < SCREEN_W; ) {
		bgtiledata = (Tile*)vram + *bgtilemap++;
		lsb = bgtiledata->line[line].lsb;
		msb = bgtiledata->line[line].msb;
		do{
			color  = (msb & (0x80 >> (pixel & 7))) ? 1 : 0;
			color |= (lsb & (0x80 >> (pixel & 7))) ? 2 : 0;
			LCD_Data(pal[(IOBGP>>(color << 1)) & 3]);
			pixel++; pixelindex++;
		}while((pixel & 7) != 0 && pixelindex < SCREEN_W);
	}
}
else{
	//Tile data stored at 8800-97FF, signed base 0x9000		
	pixel = IOSCX;
	for (pixelindex = 0; pixelindex < SCREEN_W; ) {
		bgtiledata = (Tile*)(vram + TILE_DATA1_SIGNED_BASE) + (int8_t)(*bgtilemap++);				
		lsb = bgtiledata->line[line].lsb;
		msb = bgtiledata->line[line].msb;
		do {
			color = (msb & (0x80 >> (pixel & 7))) ? 1 : 0;
			color |= (lsb & (0x80 >> (pixel & 7))) ? 2 : 0;
			LCD_Data(pal[(IOBGP >> (color << 1)) & 3]);
			pixel++; pixelindex++;
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
				//scanOAM(); 
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
