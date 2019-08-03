#include "c_types.h"
#include "esp8266_auxrom.h"
#include "esp8266_rom.h"
#include "eagle_soc.h"
#include "ets_sys.h"
#include "nosdk8266.h"

#include <hspi.h>
#include <lcd.h>
#include <display.h>

#include <cgbmu.h>
#include <cartridge.h>

#define HSPI_DIVIDER 1

#define FLASH_BASE 0x40200000
#define DISK_START (FLASH_BASE + 0x80000)



#define GPIO_OUT 	     ( 0x60000300 )
#define GPIO_OUT_W1TS    ( 0x60000304 )
#define GPIO_OUT_W1TC    ( 0x60000308 )
#define GPIO_ENABLE      ( 0x6000030C )
#define GPIO_ENABLE_W1TS ( 0x60000310 )
#define GPIO_ENABLE_W1TC ( 0x60000314 )
#define GPIO_IN          ( 0x60000318 )

#define GPIO_PIN0          0x60000328
#define GPIO_PIN2          0x60000330

//-----------------------------------------------------------
//
//-----------------------------------------------------------
void fsInit(void)
{
	
}
//-----------------------------------------------------------
//
//-----------------------------------------------------------
uint8_t readJoyPad(void){
	//GPIO_AS_INPUT(0);

	return GPIO_INPUT_GET(0) == 0? (1<<3) : 0;
}
//-----------------------------------------------------------
//
//-----------------------------------------------------------
int loadRom(char *fn){
	DISPLAY_printf("Loading %s\n", fn);
	ROM0 = (unsigned char*)DISK_START;
	loadRombank(1);
	return ROM_SIZE;
}
//-----------------------------------------------------------
//
//-----------------------------------------------------------
int loadRombank(uint8_t bank){	
	bank = (bank != 0) ? bank : 1;
	ROMBANK = (unsigned char*)(DISK_START + (bank << 14));
	bankselect = bank;		// save current
	return ROM_SIZE;
}



#define LINESIZE 0x8
#define ICACHE_FLASH_ATTR //__attribute__((section(".irom0.text")))

LOCAL void ICACHE_FLASH_ATTR hexDumpLine(void *mem, char print_ascii, int len){
    for(int i=0; i<len/4; i++){
		DISPLAY_printf("%04X",*(unsigned int*)(mem));
		mem += 4;
	}
}


LOCAL void ICACHE_FLASH_ATTR hexDump(char *mem, int len){
    DISPLAY_printf("%p:\n",mem);
	for(int i=0; i<len ;i+=LINESIZE){
		DISPLAY_printf("%02X: ",i);
		hexDumpLine(mem, 1, LINESIZE);
		DISPLAY_putc('\n');
		mem += LINESIZE;
	}
}


void printcache(void){
	DISPLAY_printf("%p\n", *(void **)(FLASH_BASE));
}

LOCAL void ICACHE_FLASH_ATTR dumpRegister(int reg){
	char *name = "UNAMED";
	switch (reg){
		case PERIPHS_IO_MUX_GPIO0_U: name = "IO_MUX_GPIO0_U"; break;
		case GPIO_OUT: name = "GPIO_OUT   "; break;
		case GPIO_ENABLE: name = "GPIO_ENABLE"; break;
		case GPIO_IN: name = "GPIO_IN    "; break;
		case GPIO_PIN0: name = "GPIO_PIN0  "; break;
		case GPIO_PIN2: name = "GPIO_PIN2  "; break;
		default: break;
	}

	DISPLAY_printf("%s = %p\n",name,*((uint32_t*)reg));
}
//-----------------------------------------------------------
//
//-----------------------------------------------------------
extern Display _display;
extern Font testFont;
int main()
{
	uint32_t i = 0;
	nosdk8266_init();
	char *level;

	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U,FUNC_GPIO2);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO5_U,FUNC_GPIO5);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U,FUNC_GPIO0);

	//WRITE_PERI_REG(GPIO_OUT_W1TC,_BV(2) | _BV(0));
	//WRITE_PERI_REG(GPIO_ENABLE_W1TC,_BV(2) | _BV(0));
	//WRITE_PERI_REG(GPIO_ENABLE,0);
	//PIN_DIR_OUTPUT = _BV(2) | _BV(0); //Enable GPIO2 light off.
	//GPIO_AS_INPUT(0);

	

	printf("\033[2J");
	printf("\n\r");

/*
	printf( "Trying a Flash test write.\n" );
	Cache_Read_Disable();
	SPIUnlock();
	SPIEraseBlock(0x40000>>16);
	uint32_t stuff = 0xAABBCCDD;
	SPIWrite(0x40004, &stuff, 4);
	stuff = 0;
	SPIRead(0x40004, &stuff, 4);
	Cache_Read_Enable(0,0,1);
	printf( "Checking to see if we can read from cache: %p / %p (both should be 0xaabbccdd)\n", *(void **)(0x40200000 + 0x40004), stuff );
*/

    printf( "Starting Display %p\n", &LCD_Init);

    HSPI_Init(HSPI_DIVIDER, HSPI_MODE_TX);

	LCD_Init();
	LCD_Clear(RED);
	LCD_Bkl(ON);

	//DISPLAY_SetFont(defaultBoldFont);
	//DISPLAY_SetFont(pixelDustFont);
	//DISPLAY_SetFont(corrierFont);
//DISPLAY_Init(OFF);
	//LCD_Rotation(LCD_REVERSE_LANDSCAPE);
	LCD_Rect(0,0,120,120,BLUE);	

	float val = 3.141592;
	//DISPLAY_printf("Float Test\nPI: %f\n", val);	

	//Cache_Read_Enable(0,0,1);
	//printcache();
	

	//DBG_Mem(0,64);

	//hw_timer_init();

	//hw_timer_set_func(count);
	//hw_timer_arm(100,1);

//	if(loadRom("mario.gb"))
//		cgbmu(1);
	DISPLAY_SetFont(corrierFont);
	DISPLAY_putc(' ');
#define LED 2

	GPIO_AS_OUTPUT(LED);
	GPIO_AS_INPUT(0);

	//dumpRegister(PERIPHS_IO_MUX_GPIO0_U);
	//dumpRegister(GPIO_OUT);
	//dumpRegister(GPIO_ENABLE);
	//dumpRegister(GPIO_IN);
	//dumpRegister(GPIO_PIN0);
	//dumpRegister(GPIO_PIN2);

	
	while(1){
	//	level = readJoyPad() != 0 ? "0\n" : "1\n";
	//	DISPLAY_printf(level);
	//dumpRegister(GPIO_OUT);
	//dumpRegister(GPIO_ENABLE);
	//dumpRegister(GPIO_IN);
	//DISPLAY_printf("Line %u\n", _display.cy/_display.font.h);
		//GPIO_SET(LED);
		//DelayMs(50);
		//GPIO_CLR(LED);
		DelayMs(100);
	}

	DISPLAY_printf("done\n");


	while(1);
return 0;
}

	
