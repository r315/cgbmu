#include "board.h"
#include "st7735.h"
#include "spi.h"

//--------------------------------
// Offset may be related with GM configuration
#ifndef TFT_OFFSET_SOURCE
#define TFT_OFFSET_SOURCE	0
#endif

#ifndef TFT_OFFSET_GATE
#define TFT_OFFSET_GATE		0
#endif

#ifdef TFT_BGR_FILTER
// Applys for TFT's with BGR filter or IPS
#define DEFAULT_MADCTL		0x08
#else
#define DEFAULT_MADCTL		0x00
#endif

#define pgm_read_byte(x) 	*((uint8_t*)(x))
#define ST_CMD_DELAY		0x80

static uint16_t _width, _height;
static uint8_t start_x, start_y;
static uint8_t scratch[4];
static spibus_t *spidev;

static void LCD_CasRasSet(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);


#if (TFT_W == 128) && (TFT_H == 160)
#define st7735_init_seq st7735_128x160
// Can't remember where I got this ...
// Init for 7735R, part 1 (red or green tab)
static const uint8_t st7735_128x160[] = {
		17,                       //  17 commands in list:
		//ST7735_SWRESET,   ST_CMD_DELAY,  //  1: Software reset, this requires CS to go high
		//150,                      //     150 ms delay
		ST7735_SLPOUT, ST_CMD_DELAY,  //  2: Out of sleep mode, 0 args, w/delay
		255,                      //     500 ms delay
		ST7735_FRMCTR1, 3      ,  //  3: Frame rate ctrl - normal mode, 3 args:
		0x01, 0x2C, 0x2D,         //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
		ST7735_FRMCTR2, 3      ,  //  4: Frame rate control - idle mode, 3 args:
		0x01, 0x2C, 0x2D,         //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
		ST7735_FRMCTR3, 6      ,  //  5: Frame rate ctrl - partial mode, 6 args:
		0x01, 0x2C, 0x2D,         //     Dot inversion mode
		0x01, 0x2C, 0x2D,         //     Line inversion mode
		ST7735_INVCTR , 1      ,  //  6: Display inversion ctrl, 1 arg, no delay:
		0x07,                     //     No inversion
		ST7735_PWCTR1 , 3      ,  //  7: Power control, 3 args, no delay:
		0xA2,
		0x02,                     //     -4.6V
		0x84,                     //     AUTO mode
		ST7735_PWCTR2 , 1      ,  //  8: Power control, 1 arg, no delay:
		0xC5,                     //     VGH25 = 2.4C VGSEL = -10 VGH = 3 * AVDD
		ST7735_PWCTR3 , 2      ,  //  9: Power control, 2 args, no delay:
		0x0A,                     //     Opamp current small
		0x00,                     //     Boost frequency
		ST7735_PWCTR4 , 2      ,  // 10: Power control, 2 args, no delay:
		0x8A,                     //     BCLK/2, Opamp current small & Medium low
		0x2A,
		ST7735_PWCTR5 , 2      ,  // 11: Power control, 2 args, no delay:
		0x8A, 0xEE,
		ST7735_VMCTR1 , 1      ,  // 12: Power control, 1 arg, no delay:
		0x0E,
		ST7735_COLMOD , 1      ,  // 13: set color mode, 1 arg, no delay:
		0x05,
		ST7735_GMCTRP1, 16     ,  // 14: Magical unicorn dust, 16 args, no delay:
		0x02, 0x1c, 0x07, 0x12,
		0x37, 0x32, 0x29, 0x2d,
		0x29, 0x25, 0x2B, 0x39,
		0x00, 0x01, 0x03, 0x10,
		ST7735_GMCTRN1, 16     ,  // 15: Sparkles and rainbows, 16 args, no delay:
		0x03, 0x1d, 0x07, 0x06,
		0x2E, 0x2C, 0x29, 0x2D,
		0x2E, 0x2E, 0x37, 0x3F,
		0x00, 0x00, 0x02, 0x10,
		ST7735_MADCTL, 1      ,
		DEFAULT_MADCTL,
		ST7735_NORON  ,    ST_CMD_DELAY, // 16: Normal display on, no args, w/delay
		10,                       //      10 ms delay
		ST7735_DISPON ,    ST_CMD_DELAY, // 17: Main screen turn on, no args w/delay
		100						  //      100 ms delay
};
#elif (TFT_W == 80) && (TFT_H == 160)
#define st7735_init_seq st7735_80x160
/**
 * Values for 132x162 GRAM, visible 80x160
 * Looks good on small 80x160 IPS display
 */
static const uint8_t st7735_80x160 [] = {
	17,                       //  18 commands in list:
	//ST7735_SWRESET,   ST_CMD_DELAY,  //  1: Software reset, 0 args, w/delay
	//120,                      //     120ms delay
	ST7735_SLPOUT ,   ST_CMD_DELAY,  //  2: Out of sleep mode, 0 args, w/delay
	255,                      //     500 ms delay	
	ST7735_FRMCTR1, 3      ,  //  3: Frame rate ctrl - normal mode, 3 args:
	0x05, 0x3A, 0x3A,         //     Rate = fosc/([05]x2+40) * (LINE+[3A]+[3A]+2)
	ST7735_FRMCTR2, 3      ,  //  4: Frame rate control - idle mode, 3 args:
	0x05, 0x3A, 0x3A,         //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
	ST7735_FRMCTR3, 6      ,  //  5: Frame rate ctrl - partial mode, 6 args:
	0x05, 0x3A, 0x3A,         //     Dot inversion mode
	0x05, 0x3A, 0x3A,         //     Line inversion mode
	ST7735_INVCTR , 1      ,  //  6: Display inversion ctrl, 1 arg, no delay:
	0x07,                     //     No inversion
	ST7735_PWCTR1 , 3      ,  //  7: Power control, 3 args, no delay:
	0xA8,                     //     AVDD = 5V, GVDD = 4.3V
	0x08,                     //     GVCL = -4.3V
	0x84,                     //     AUTO mode
	ST7735_PWCTR2 , 1      ,  //  8: Power control, 1 arg, no delay:
	0xC0,                     //     VGH25 = 2.4V, VGSEL = -7.5V, VGH = 11.9V (2 * AVDD + VGH25 - 0.5)
	ST7735_PWCTR3 , 2      ,  //  9: Power control, 2 args, no delay:
	0x0D,                     //     Opamp current: small, medium low
	0x00,                     //     Boost frequency
	ST7735_PWCTR4 , 2      ,  // 10: Power control, 2 args, no delay:
	0x8D,                     //     BCLK/2, Opamp current small & Medium low
	0x2A,                     //
	ST7735_PWCTR5 , 2      ,  // 11: Power control, 2 args, no delay:
	0x8D,                     //
	0xEE,                     //
	ST7735_VMCTR1 , 1      ,  // 12: Power control, 1 arg, no delay:
	0x1A,                     //
	ST7735_GMCTRP1, 16     ,  // 13: Magical unicorn dust, 16 args, no delay:
	0x03, 0x22, 0x07, 0x0A,
	0x2E, 0x30, 0x25, 0x2A,
	0x28, 0x26, 0x2E, 0x3A,
	0x00, 0x01, 0x03, 0x13,
	ST7735_GMCTRN1, 16     ,  // 14: Sparkles and rainbows, 16 args, no delay:
	0x04, 0x16, 0x06, 0x0D,
	0x2D, 0x26, 0x23, 0x27,
	0x27, 0x25, 0x2D, 0x3B,
	0x00, 0x01, 0x04, 0x13,
	ST7735_COLMOD , 1      ,  // 15: set color mode, 1 arg, no delay:
	0x05,                     //     16-bit/pixel
	ST7735_NORON  ,    ST_CMD_DELAY, // 16: Normal display on, no args, w/delay
	10,                       //     10 ms delay
#ifdef TFT_BGR_FILTER
	ST7735_INVON, 0        ,  // 17: invert display, no args, no delay
#else
	ST7735_INVOFF, 0       ,  // 17: Don't invert display, no args, no delay
#endif
	ST7735_DISPON ,    ST_CMD_DELAY, // 18: Main screen turn on, no args w/delay
	100						  //     100 ms delay
};
#else
#error Define TFT_W and TFT_H
#endif

/**
 * @brief Writes command to display
 */
static void LCD_Command(uint8_t data){
	LCD_CD0; 
	SPI_Transfer(spidev, &data, 1);
	LCD_CD1;
}

/**
 * @brief Write 16bit data with big-endian (msb first)
 */
void LCD_Data(uint16_t data){
	scratch[0] = data >> 8;
	scratch[1] = data;
	SPI_Transfer(spidev, scratch, 2);
}

/**
 * @brief Companion code to the above tables.  Reads and issues
 * a series of LCD commands stored as byte array.
 * */
static void LCD_InitSequence(const uint8_t *addr) {
	uint8_t  numCommands, numArgs;
	uint16_t ms;

	numCommands = pgm_read_byte(addr++);       // Number of commands to follow
	while(numCommands--) {                     // For each command...
		LCD_Command(pgm_read_byte(addr++));    //   Read, issue command
		numArgs  = pgm_read_byte(addr++);      //   Number of args to follow
		ms       = numArgs & ST_CMD_DELAY;            //   If hibit set, delay follows args
		numArgs &= ~ST_CMD_DELAY;                     //   Mask out delay bit
		while(numArgs--) {                     //   For each argument...
			SPI_Transfer(spidev, (uint8_t*)addr++, 1);  	   //   Read, issue argument
		}

		if(ms) {
			ms = pgm_read_byte(addr++);        // Read post-command delay time (ms)
			if(ms == 255) ms = 500;
			DelayMs(ms);
		}
	}
}

/**
 * @brief Write a block of data
 * 
 */
static void LCD_WriteData(uint16_t *data, uint32_t count){
	if(spidev->dma.per != NULL){
		spidev->flags |= SPI_16BIT;
		SPI_TransferDMA(spidev, (uint8_t*)data, count);
		//LCD_CS1; // SET by DMA handler
	}else{
		while(count--)
			LCD_Data(*data++);
		LCD_CS1;
	}	
}

/**
 * @brief End of transfer handler for DMA use
 * */
static void LCD_EOTHandler(void){
	spidev->flags &= ~(SPI_DMA_NO_MINC | SPI_16BIT);
	SPI_WaitEOT(spidev);
	LCD_CS1;
}

/**
 * @brief Define area to be writtem
 * 
 * \param x1 :  Start x
 * \param y1 :  Start y
 * \param x2 :  End x
 * \param y2 :  End y
 * \param color :
 */
static void LCD_CasRasSet(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2){
	LCD_Command(ST7735_CASET);
	scratch[0] = x1 >> 8;
	scratch[1] = x1;
	scratch[2] = x2 >> 8;
	scratch[3] = x2;
	SPI_Transfer(spidev, scratch, 4);	

	LCD_Command(ST7735_RASET);
	scratch[0] = y1 >> 8;
	scratch[1] = y1;
	scratch[2] = y2 >> 8;
	scratch[3] = y2;
	SPI_Transfer(spidev, scratch, 4);

	LCD_Command(ST7735_RAMWR);
}

/**
 * @brief Setup draw area, LCD_CS must be managed by caller
 *
 * \param x : X position
 * \param y : Y position
 * \param w : Width
 * \param h : Height
 */
void LCD_Window(uint16_t x, uint16_t y, uint16_t w, uint16_t h){
	x += start_x;
	y += start_y;

	LCD_CasRasSet(x, y, x + (w - 1), y + (h - 1));
}

/**
 * @brief Fill's area with same color
 * 
 * \param x :
 * \param y :
 * \param w :
 * \param h :
 * \param color :
 */
void LCD_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color){
	uint32_t count = w * h;

	if(!count){
        return;
    }

	SPI_WaitEOT(spidev);

	LCD_CS0;
	LCD_Window(x, y, w, h);

	if(spidev->dma.per != NULL){
		spidev->flags |= SPI_DMA_NO_MINC | SPI_16BIT;
		*((uint16_t*)scratch) = color;
		SPI_TransferDMA(spidev, (uint8_t*)scratch, count);
		//LCD_CS1; // SET by DMA handler	
	}else{
		while(count--)
			LCD_Data(color);
		LCD_CS1;
	}
}

/**
 * @brief Write data block to defined area
 *  
 * \param x :
 * \param y :
 * \param w :
 * \param h :
 * \param data : Pointer to data
 */
void LCD_WriteArea(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t *data){
	uint32_t count = w * h;
    
    if(!count){
        return;
    }

	SPI_WaitEOT(spidev);
    LCD_CS0;
    LCD_Window(x, y, w, h);
    LCD_WriteData(data, count);
}

/**
 * @brief Draw a single pixel
 *
 */
void LCD_Pixel(uint16_t x, uint16_t y, uint16_t color){
	x += start_x;
	y += start_y;
	
	SPI_WaitEOT(spidev);

	LCD_CS0;
	LCD_CasRasSet(x, y, x, y);
	LCD_Data(color);
	LCD_CS1;
}


/**
 * @brief Display initialisation
 * */
void LCD_Init(void *spi){
	spidev = (spibus_t*)spi;
	if(spidev->dma.per != NULL){
		spidev->eot_cb = LCD_EOTHandler;
	}

	LCD_CD1;
	LCD_CS1;
	LCD_BKL0;
	LCD_RST0;
	DelayMs(2);
	LCD_RST1;
	DelayMs(5);

	LCD_CS0;
	LCD_Command(ST7735_SWRESET);
	LCD_CS1;

	DelayMs(150);

	LCD_CS0;
	LCD_InitSequence(st7735_init_seq);
	LCD_CS1;

	// Set offset
	start_x = TFT_OFFSET_SOURCE;
	start_y = TFT_OFFSET_GATE;
	_width  = TFT_W;
	_height = TFT_H;
}

/**
 * @brief
 * */
void LCD_SetOrientation(uint8_t m) {

	switch (m) {
    case LCD_PORTRAIT:
        m = (ST7735_MADCTL_MX | ST7735_MADCTL_MY);
        _width  = TFT_W;
        _height = TFT_H;
        break;
    case LCD_LANDSCAPE:
        m = (ST7735_MADCTL_MV | ST7735_MADCTL_MY);
        _width  = TFT_H;
        _height = TFT_W;
        break;
    case LCD_REVERSE_PORTRAIT:
        m = (ST7735_MADCTL_MY);
        _width  = TFT_W;
        _height = TFT_H;
        break;
    case LCD_REVERSE_LANDSCAPE:
        m = (ST7735_MADCTL_MV | ST7735_MADCTL_MX);
        _width  = TFT_H;
        _height = TFT_W;
        break;

    default:
        return;
    }

	SPI_WaitEOT(spidev);

	LCD_CS0;
	LCD_Command(ST7735_MADCTL);
	SPI_Transfer(spidev, &m, 1);
	LCD_CS1;
}

/**
 * @brief Screen scroll
 *
 */
void LCD_Scroll(uint16_t sc){

	SPI_WaitEOT(spidev);

	LCD_CS0;
	LCD_Command(ST7735_VSCSAD);
	LCD_Data(sc);
	LCD_CS1;
}

uint16_t LCD_GetWidth(void){
	return _width;
}

uint16_t LCD_GetHeight(void){
	return _height;
}

uint32_t LCD_GetSize(void){
	return _height*_width;
}

void LCD_Bkl(uint8_t state){
	if(state != 0) {
		LCD_BKL1;
	}else {
		LCD_BKL0;
	}
}
