

#include <stdio.h>
#include <common.h>
#include "cartridge.h"
#include "debug.h"

unsigned char bankSelect;

#if defined(__ESP03__)
#include <spiffs.h>
unsigned char *ROM0 = (unsigned char*)0;
unsigned char *ROMBANK = (unsigned char*)0;
//unsigned char ROM0[ROM_SIZE];
//unsigned char ROMBANK[ROM_SIZE];

int loadRombank(void)
{
}

int loadRom(char *fn){
}

#elif defined(__BB__)
#include <pff/pff.h>

unsigned char *ROM0 = (unsigned char*)ROM0_START;
unsigned char *ROMBANK = (unsigned char*)ROMBANK_START;
//unsigned char cartridgeRam[0x2000];


FATFS drive0;
char* f_error(FRESULT res)
{
	switch(res)
	{
		case FR_OK: return "ok"  ;   
			
		case FR_DISK_ERR: 
			return "disk error";
		case FR_NOT_READY:
			return "disk not ready";
		case FR_NO_FILE:  
			return "no file";
		case FR_NO_PATH:  
			return "invalid path";
		case FR_NOT_OPENED: 
			return "cant open file";
		case FR_NOT_ENABLED:
			return "not enable";
		case FR_NO_FILESYSTEM:
			return "file system";
	}
	return "";
}
//-----------------------------------------------------------
//
//-----------------------------------------------------------
void fsInit(void)
{
	DBG_Info(f_error(pf_mount(&drive0)));
}
//--------------------------------------------------
//
//--------------------------------------------------
int loadRombank(void)
{
WORD n;	
	//drawNumber(232,0,bankSelect,10);
	pf_lseek(bankSelect << 14);
	pf_read(ROMBANK, ROM_SIZE, &n);
	//DISPLAY_printf("Loaded %u bytes into Rom Bank %u\n",n, bankSelect);
	//drawChar(232,0,' ');
	return n;
}
//--------------------------------------------------
//
//--------------------------------------------------
int loadRom(char *fn)
{
WORD n;
	if(!drive0.fs_type){
		fsInit();
	}
	DBG_Info(f_error(pf_open(fn)));
	DBG_Info("Loading ROM0");
	DBG_Info(f_error(pf_read(ROM0, ROM_SIZE, &n)));
	bankSelect = 1;
	loadRombank();	
	return n;
}
#elif defined(__EMU__)
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

unsigned char ROM0[ROM_SIZE];
unsigned char ROMBANK[ROM_SIZE];
unsigned char cartridgeRam[ROM_SIZE/2];


char* romfile;

#define LOG_TAG "CARTRIDGE"

int loadRom(char *fn)
{
	char cwd[1024];
	FILE *fp;
	romfile = fn;
	int n;
	
	if (getcwd(cwd, sizeof(cwd)))
		fprintf(stdout, "Current working dir: %s\n", cwd);
	else
		perror("getcwd() error");

	fprintf(stdout,"%s: Loading File \"%s\"\n", LOG_TAG, fn);
	fp = fopen(romfile, "rb");

	if (fp == NULL)
	{
		printf("%s: File not found\n", LOG_TAG);
		return 0;
	}

	n = fread(ROM0, 1, ROM_SIZE, fp);
	n += fread(ROMBANK, 1, ROM_SIZE, fp);

	fclose(fp);
	bankSelect = 1;
	printf("%s: Rom file loaded!\n", LOG_TAG);
	return n;
}
//--------------------------------------------------
//
//--------------------------------------------------
int loadRombank(void)
{
FILE *fp;
size_t n;
	fp = fopen(romfile,"rb");	
	fseek(fp,bankSelect << 14,SEEK_SET);
	n = fread(ROMBANK, 1, ROM_SIZE, fp);
	fclose(fp);	
	//printf("Loaded %u bytes into Rom Bank %u\n", n, bankSelect);
	return n;
}
#endif
/***************************************************
// MBC1
***************************************************/
unsigned char cartridgeRead(unsigned short address)
{
	switch(address >> 14)
	{
		case 0:  // 0000-3FFF   Rom bank 0
			return ROM0[address]; 
			
		case 1:  // 4000-7FFF Nota romBank = 0/1 deve de retornar ROM1
			address &= 0x3FFF;			
			return ROMBANK[address]; 
			
		case 2:  // 8000-C000 banking Ram not implemented
		case 3:
			return 0xFF;//ROMBANK[address - 0x4000];
	}
	return 0xFF;
}
//----------------------------------------------------
//
//----------------------------------------------------
void cartridgeWrite(unsigned short address, char data)
{	
	switch(address >> 13)
	{
		case 1: // 2000-3FFF
			if(data != bankSelect)
			{
				bankSelect = data;
				loadRombank();
			}
			break;
			
		default: break;
	}
}
/***************************************************
// MBC2
***************************************************/


