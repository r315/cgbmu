#ifndef _cartridge_h_
#define _cartridge_h_

#define ROM_SIZE 0x4000
#define RAM_SIZE 0x2000

unsigned char bankselect;
extern unsigned char *ROM0;
extern unsigned char *ROMBANK;

unsigned char cartridgeRead(unsigned short address);
void cartridgeWrite(unsigned short address,char data);

#endif
