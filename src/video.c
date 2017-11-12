

#include "dmgcpu.h"
#include "video.h"
#include "lcd.h"


unsigned int videoTicks;
char frameReady;
const unsigned short pal[]={0xE7DA,0x8E0E,0x334A,0x08C4};
//const unsigned short pal[]={0x08C4,0x334A,0x8E0E,0xE7DA};
unsigned char spriteTable[40]; // masx 10 sprites
unsigned char nsprites;
//-----------------------------------------
//
//-----------------------------------------
void scanOAM()
{
	unsigned char i;
	nsprites = 0;

	for (i = 0; i < 160; i += 4)
	{
		if (oam[i] && (IOLY >= (oam[i] - 16)) && (IOLY < (oam[i] - 8)))
		{
			if ((oam[i + 1] > 0) && (oam[i + 1] < 168))
			{
				spriteTable[nsprites++] = oam[i] - 16;    // Y position
				spriteTable[nsprites++] = oam[i + 1] - 8;	// X position
				spriteTable[nsprites++] = oam[i + 2];     // Pattern number
				spriteTable[nsprites++] = oam[i + 3];		// Flags				
			}
		}
	}
	nsprites >>= 2;
}
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
		
	if(IOIE & STAT_IE)      
	{
		if(IOSTAT & OAM_IE)
			IOIF |= STAT_IF;
			
		//if(IOSTAT & LYC_LY_IE)
			if(IOSTAT & LYC_LY_FLAG)
				IOIF |= STAT_IF;	
	}		
}
//-----------------------------------------
//
//-----------------------------------------
char video(void)
{	
	if(!(IOLCDC & LCD_DISPLAY)) return 0; // lcd controller off	
		
	videoTicks += (cycles>>2);		
		 	
	switch(IOSTAT & 3)	
	{
		case 2: // oam access start scanline	
			if(videoTicks > 83)
			{							
				IOSTAT |= 3;	// next mode 3, oam and vram access				
				videoTicks -= 83;
				scanOAM(); 
			}
			break;
			
		case 3: // oam and vram access
			if(videoTicks > 175)
			{
				 IOSTAT &= 0xFC;     // next mode 0, H-blank				 
				 if((IOIE & STAT_IE) && (IOSTAT & HB_IE)) // LCD STAT & H-Blank IE
				 		IOIF |= STAT_IF;
				videoTicks -= 175;				 		
			 	scanline();				 		
			}
			break;
			
		case 0: // H-Blank
			if(videoTicks > 207)
			{
				IOLY++;
				videoTicks -= 207;
				if(IOLY > (SCREEN_H-1))
				{				
					IOSTAT |= 1;     //next mode 1, v-blank
					if(IOIE & V_BLANK_IE)
						IOIF |= V_BLANK_IF;
						
					if((IOIE & STAT_IE) && (IOSTAT & VB_IE))
						IOIF |= STAT_IF;
				}
				else
				{	
					IOSTAT |= 2;     //next mode 2, serching oam					
				 	lycIrq();							
				}			
			}
			break;
			
		case 1: // V-blank 10 lines
			if(videoTicks > 456)
			{
				IOLY++;									
				videoTicks -= 456;
				lycIrq();
				
				if(IOLY < 154)								 
					return frameReady;
			
				IOSTAT &= 0xFC; // next mode 2
				IOSTAT |= 2;
				
				lycIrq();
				IOLY = 0;					
					
				frameReady = 1;
			}
			break;			
		
	}
	return frameReady;
}
