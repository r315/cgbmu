#include "c_types.h"
#include "esp8266_auxrom.h"
#include "esp8266_rom.h"
#include "eagle_soc.h"
#include "ets_sys.h"
#include "nosdk8266.h"

#include <hspi.h>
#include <lcd.h>
#include <display.h>

#define HSPI_DIVIDER 1

#define call_delay_us( time ) { asm volatile( "mov.n a2, %0\n_call0 delay4clk" : : "r"(time*13) : "a2" ); }

#define Delay_Ms(x)                 \
	{                          \
         while(x--){               \
	    call_delay_us(500000); \
	    call_delay_us(500000); \
         }                         \
        }


int main()
{
	int i = 0;
	nosdk8266_init();

	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U,FUNC_GPIO2);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO5_U,FUNC_GPIO5);
	PIN_DIR_OUTPUT = _BV(2); //Enable GPIO2 light off.

	//printf( "Trying a Flash test write.\n" );

	//Cache_Read_Disable();
	//SPIUnlock();
	//SPIEraseBlock(0x40000>>16);
	//uint32_t stuff = 0xAABBCCDD;
	//SPIWrite(0x40004, &stuff, 4);
	//stuff = 0;
	//SPIRead(0x40004, &stuff, 4);
	Cache_Read_Enable(0,0,1);
	//printf( "Checking to see if we can read from cache: %p / %p (both should be 0xaabbccdd)\n", *(void **)(0x40200000 + 0x40000 + 4), stuff );

	printf("\033[2J");
	printf("\n\r");


    printf( "Starting Display %p\n", &LCD_Init);

    HSPI_Init(HSPI_DIVIDER, HSPI_MODE_TX);

	LCD_Init();
	LCD_Clear(RED);
	LCD_Bkl(ON);

	LCD_Rect(0,0,120,120,BLUE);	

	GPIO_OUTPUT_SET(0,0);

	float val = 3.141592;
	DISPLAY_printf("Float Test\nPI: %f\n", val);

	while(1)
	{	
		DISPLAY_printf( "Count: %d\n", i);
		call_delay_us( 300000 );
		i++;
	}
}

	
