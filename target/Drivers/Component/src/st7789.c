#include "board.h"
#include "st7789.h"
#include "spi.h"

#define ST_CMD_DELAY 		0x80

static uint16_t _width, _height;
static uint8_t scratch[4];
static spibus_t *spidev;

#if TFT_W == 240
const uint8_t st7789_240x240[] = {
    8,
    ST7789_SLPOUT, ST_CMD_DELAY, 120,
    ST7789_COLMOD, 1, COLOR_MODE_65K | COLOR_MODE_16BIT,
    ST7789_MADCTL, 1, ST7789_MADCTL_RGB,
    ST7789_CASET, 4, 0,0,0, TFT_W,
    ST7789_RASET, 4, 0,0,0, TFT_H,
    ST7789_INVON, ST_CMD_DELAY, 10,      // Inversion ON
    ST7789_NORON, ST_CMD_DELAY, 10,      // Normal display on, no args, w/delay
    ST7789_DISPON, ST_CMD_DELAY, 100,    // Main screen turn on, no args, w/delay
};
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

	numCommands = *addr++;          // Get total number os commands
	while(numCommands--) {                     
		LCD_Command(*addr++);       // Send command
		numArgs  = *addr++;         // Get number of args
		ms       = numArgs;         // Get argument
		numArgs &= ~ST_CMD_DELAY;   // Clear delay flag           
		SPI_Transfer(spidev, (uint8_t*)addr, numArgs); // Send arguments
		addr += numArgs;            // Move to next command

		if(ms & ST_CMD_DELAY) {     // If argument was delay, do it
			ms = *addr++;
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
    LCD_Command(ST7789_CASET);
    scratch[0] = x1 >> 8;
    scratch[1] = x1;
    scratch[2] = x2 >> 8;
    scratch[3] = x2;
    SPI_Transfer(spidev, scratch, 4);	

    LCD_Command(ST7789_RASET);
    scratch[0] = y1 >> 8;
    scratch[1] = y1;
    scratch[2] = y2 >> 8;
    scratch[3] = y2;
    SPI_Transfer(spidev, scratch, 4);

    LCD_Command(ST7789_RAMWR);
}

/**
 * @brief Setup draw area
 *
 * \param x : X position
 * \param y : Y position
 * \param w : Width
 * \param h : Height
 */
void LCD_Window(uint16_t x, uint16_t y, uint16_t w, uint16_t h){
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
        scratch[0] = color >> 8;
        scratch[1] = color;
        while(count--){            
            SPI_Transfer(spidev, scratch, 2);
        }
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
 * @brief 
 * 
 * @param x 
 * @param y 
 * @param c 
 */
void LCD_Pixel(uint16_t x, uint16_t y, uint16_t color){
    
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

    // SW Reset requires CS pin release
    LCD_CS0;
    LCD_Command(ST7789_SWRESET);
    LCD_CS1;
    DelayMs(5);

    LCD_CS0;
    LCD_InitSequence(st7789_240x240);
    LCD_CS1;

    _width  = TFT_W;
    _height = TFT_H;
}

/**
 * @brief
 * */
void LCD_SetOrientation(uint8_t m) {

    switch (m) {
    case LCD_PORTRAIT:
        m = (ST7789_MADCTL_MX | ST7789_MADCTL_MY);
        _width  = TFT_W;
        _height = TFT_H;
        break;
    case LCD_LANDSCAPE:
        m = (ST7789_MADCTL_MV | ST7789_MADCTL_MY);
        _width  = TFT_H;
        _height = TFT_W;
        break;
    case LCD_REVERSE_PORTRAIT:
        m = (ST7789_MADCTL_MY);
        _width  = TFT_W;
        _height = TFT_H;
        break;
    case LCD_REVERSE_LANDSCAPE:
        m = (ST7789_MADCTL_MV | ST7789_MADCTL_MX);
        _width  = TFT_H;
        _height = TFT_W;
        break;

    default:
        return;
    }


    SPI_WaitEOT(spidev);

    LCD_CS0;
    LCD_Command(ST7789_MADCTL);
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
    LCD_Command(ST7789_VSCSAD);
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
