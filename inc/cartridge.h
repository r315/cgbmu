#ifndef _cartridge_h_
#define _cartridge_h_

#define ROM_SIZE 0x4000 //16k
#define RAM_SIZE 0x2000

#if defined(__EMU__)
	extern unsigned char ROM0[ROM_SIZE];
	extern unsigned char ROMBANK[ROM_SIZE];
#elif defined(__BB__)
	#define ROM0_START      0x2007C000
	#define ROMBANK_START   0x20080000
	extern unsigned char *ROM0;
	extern unsigned char *ROMBANK;
#elif defined(__ESP03__)
	extern unsigned char *ROM0;
	extern unsigned char *ROMBANK;
#endif

void fsInit(void);

int loadRom(char *fn);
unsigned char cartridgeRead(unsigned short address);
void cartridgeWrite(unsigned short address,char data);

#endif
