#ifndef _cartridge_h_
#define _cartridge_h_

#define ROM0_START      0x2007C000
#define ROMBANK_START   0x20080000
#define ROM_SIZE 0x4000 //16k
#define RAM_SIZE 0x2000

#ifdef __EMU__
	extern unsigned char ROM0[0x4000];
	extern unsigned char ROMBANK[0x4000];
#else
	extern unsigned char *ROM0;
	extern unsigned char *ROMBANK;
#endif

void fsInit(void);

void loadRom(char *fn);
unsigned char cartridgeRead(unsigned short address);
void cartridgeWrite(unsigned short address,char data);

#endif
