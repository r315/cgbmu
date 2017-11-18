#include "dmgcpu.h"
#include "video.h"
#include "lcd.h"


uint16_t video_cycles = 0;

const unsigned short pal[]={0xE7DA,0x8E0E,0x334A,0x08C4};
//const unsigned short pal[]={0x08C4,0x334A,0x8E0E,0xE7DA};
Sprite *spriteline[MAX_SPRITES/sizeof(Sprite)]; // max 10 sprites
unsigned char nsprites;

//-----------------------------------------
//read OBJECT Attribute Memory for one line
//-----------------------------------------
void scanOAM(){
uint8_t i, line = IOLY + SPRITE_H;
Sprite *poam = (Sprite*)&oam[0];
Sprite **ps = spriteline;
	for (i = 0; i < SCREEN_W; i += sizeof(Sprite)){
        if( line >= poam->y && line < poam->y + 8){
            if(poam->x >= SPRITE_W && poam->x < SCREEN_W + SPRITE_W){			
                *ps = (Sprite*)poam;
                ps += 1;				
			}
		}
        poam += 1;
	}
	*ps = '\0';         //mark end of array
}

//-----------------------------------------
//
//-----------------------------------------
void scanline()
{
unsigned short tileMapAddress;
unsigned short tileDataAddress;
unsigned short windowMapAddress;
//unsigned short spriteDataAddress;
unsigned char msb,lsb,t,pixel,offset;
unsigned char color;


if(IOLCDC & BG_MAP)
	tileMapAddress = 0x1C00 ; //BG Tile Map 1 9C00 - 9FFF
else
	tileMapAddress = 0x1800 ; //BG Tile Map 0 9800 - 9BFF
	
if(IOLCDC & W_MAP)
	windowMapAddress = 0x1C00 ; //Window Tile Map 1 9C00 - 9FFF
else
	windowMapAddress = 0x1800 ; //Window Tile Map 0 9800 - 9BFF


tileMapAddress += (((IOLY & 0xf8)<<2) + ((IOSCY& 0xf8)<<2)) & 0x3FF;
windowMapAddress += (((IOLY - IOWY) & 0xf8)<<2);

offset = IOSCX;

if(IOLCDC & W_BG_DATA) // Tile Data Select
{   //8000-8FFF
	for(pixel = 0; pixel < SCREEN_W; )
	{		
		tileDataAddress = vram[tileMapAddress+(offset>>3)] << 4; // 16 bytes/tile		
		tileDataAddress += (IOLY & 7)<<1;	             // line to draw
	
		lsb = vram[tileDataAddress++];
		msb = vram[tileDataAddress];	
		
		do
		{
			t = (offset & 7);
			color = (msb>>(7-t))<<1;			
			color |= (lsb>>(7-t))&1;
			color &= 3;		
			LCD_Data(pal[(IOBGP>>(color<<1)) &3]);
			pixel++;
			offset++;
		}while((offset&0x07)!=0 && pixel < 160);
	}
}
else
{	//8800-97FF	
	for(pixel = 0; pixel < SCREEN_W; )	
	{
		tileDataAddress = 0x1000;				
		tileDataAddress += (signed char)vram[tileMapAddress + (offset>>3)]<<4;
		tileDataAddress += (IOLY & 7) << 1;	// line to draw
	
		lsb = vram[tileDataAddress++];
		msb = vram[tileDataAddress];				
		
		do
		{
			t = (offset & 7);	
			color = (lsb >> (7-t)) & 1;			
			color |=  (msb >> (7-t)) << 1;			
			color &= 3;				
			LCD_Data(pal[(IOBGP>>(color<<1)) &3]);
			pixel++;
			offset++;
		}while((offset&0x07)!= 0 && pixel<160);				
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
		
	if(IOIE & STAT_IE){
		if(IOSTAT & (OAM_IE | LYC_LY_FLAG))
			IOIF |= STAT_IF;		
	}		
}

//-----------------------------------------
//
//-----------------------------------------
void video(void){	
	if(!(IOLCDC & LCD_DISPLAY)) return; // lcd controller off	
		
	video_cycles += GET_CYCLE();		
		 	
	switch(IOSTAT & V_MODE_MASK)
	{
		case V_M2: // oam access start scanline	
			if(video_cycles > V_M2_CYCLE)
			{							
				video_cycles -= V_M2_CYCLE;
				IOSTAT |= V_M3;		// next mode 3, oam and vram access				
				scanOAM(); 
			}
			break;
			
		case V_M3: // oam and vram access
			if(video_cycles > V_M3_CYCLE)
			{
				video_cycles -= V_M3_CYCLE;
				 IOSTAT &= ~(V_MODE_MASK);     // next mode 0, H-blank				 
				 if((IOIE & STAT_IE) && (IOSTAT & HB_IE)) // LCD STAT & H-Blank IE
				 		IOIF |= STAT_IF;
			 	scanline();				 		
			}
			break;
			
		case V_M0: // H-Blank
			if(video_cycles > V_M0_CYCLE){
				video_cycles -= V_M0_CYCLE;
				IOLY++;
				if(IOLY > (SCREEN_H-1)){				
					IOSTAT |= V_M1;     // next mode 1, v-blank
					if(IOIE & V_BLANK_IE)
						IOIF |= V_BLANK_IF;
						
					if((IOIE & STAT_IE) && (IOSTAT & VB_IE))
						IOIF |= STAT_IF;
				}
				else{					// Finish processing on line, go to next one
					IOSTAT |= V_M2;     // next mode 2, serching oam					
				 	lycIrq();
				}
			}
			break;
			
		case V_M1: // V-blank 10 lines
			if(video_cycles > V_LINE_CYCLE)
			{
				video_cycles -= V_LINE_CYCLE;
				IOLY++;									
				lycIrq();
				
				if(IOLY < (SCREEN_H + VBLANK_LINES))
					return;
			
				IOSTAT &= ~(V_MODE_MASK); // next mode 2
				IOSTAT |= V_M2;
				
				lycIrq();
				IOLY = 0;					
				LCD_Window(0, 0, SCREEN_W, SCREEN_H);
			}
			break;		
	}
}
