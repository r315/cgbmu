#ifndef _cartridge_h_
#define _cartridge_h_

#define ROM_SIZE 0x4000		// Rom bank #0 0x0000 - 0x3FFF
#define RAM_SIZE 0x2000     // Internal

#define CARTRIDGE_SIZE	0x8000
//#define MBC1_SIZE		

#define CARTRIDGE_TYPE_OFFSET	0x147
#define CARTRIDGE_ROM			0
#define CARTRIDGE_MBC1			1
#define CARTRIDGE_MBC1_RAM		2

unsigned char bankselect;
extern unsigned char *ROM0;
extern unsigned char *ROMBANK;

unsigned char cartridgeRead(unsigned short address);
void cartridgeWrite(unsigned short address,char data);

#endif
