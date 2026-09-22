#include <stddef.h>
#include "drvlcd.h"
#include "st7789.h"
#include "spi.h"
#include "gpio.h"

#define DRV_CMD_DELAY 		0x80
#define DRV_CMD_PARM_NONE 	0
#define DRV_CMD_PARM1 		1
#define DRV_CMD_PARM2 		2
#define DRV_CMD_PARM3 		3
#define DRV_CMD_PARM4 		4
#define DEFAULT_MADCTL      ST7789_MADCTL_RGB

#define LCD_CS1     GPIO_Write(drvlcd->cs, GPIO_PIN_HIGH)
#define LCD_CS0     GPIO_Write(drvlcd->cs, GPIO_PIN_LOW)
#define LCD_CD1     GPIO_Write(drvlcd->cd, GPIO_PIN_HIGH)
#define LCD_CD0     GPIO_Write(drvlcd->cd, GPIO_PIN_LOW)
#define LCD_BKL1    GPIO_Write(drvlcd->bkl, GPIO_PIN_HIGH)
#define LCD_BKL0    GPIO_Write(drvlcd->bkl, GPIO_PIN_LOW)
#define LCD_RST1    GPIO_Write(drvlcd->rst, GPIO_PIN_HIGH)
#define LCD_RST0    GPIO_Write(drvlcd->rst, GPIO_PIN_LOW)

static uint16_t _width, _height;
static uint8_t scratch[5];
static drvlcdspi_t *drvlcd;
static spibus_t *spidev;

const drvlcd_t st7789_drv =
{
    LCD_Init,
    LCD_FillRect,
    LCD_WriteArea,
    LCD_Pixel,
    LCD_Scroll,
    LCD_SetOrientation,
    LCD_Window,
    LCD_Data,
    LCD_Bkl,
    LCD_GetWidth,
    LCD_GetHeight,
    LCD_GetSize,
    LCD_DataEnd
};

extern void DelayMs(uint32_t ms);
/**
 * @brief Initialization sequence
 *
 * [0} Number of commands on sequence
 *
 * [0] number of parameters, bit 7 apply delay after command
 * [1] command
 * [2] parameter 1
 * [n] parameter n / delay
 *
 * repeat from index 1
 */
static const uint8_t st7789_sequence[] = {
    6,                                  // Number of commands in sequence
    DRV_CMD_DELAY, ST7789_SLPOUT, 120,
    DRV_CMD_PARM1, ST7789_COLMOD, (COLOR_MODE_65K | COLOR_MODE_16BIT),
    DRV_CMD_PARM1, ST7789_MADCTL, DEFAULT_MADCTL,
    DRV_CMD_DELAY, ST7789_INVON, 10,
    DRV_CMD_DELAY, ST7789_NORON, 10,
    DRV_CMD_DELAY, ST7789_DISPON, 10,
};


/**
 * @brief Writes command to display
 */
static void LCD_Command(uint8_t *params, uint8_t len){
    LCD_CD0;
    SPI_Transfer(spidev, params++, 1);
    LCD_CD1;
    if(len)
        SPI_Transfer(spidev, params, len);
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
static void LCD_InitSequence(const uint8_t *seq) {
    uint8_t  numCommands, numParm;
	uint16_t ms;

	numCommands = *seq++;                           // Get number of commands in sequence

	while(numCommands--) {
		numParm = seq[0] & ~DRV_CMD_DELAY;          // Get number of parameters by masking delay bit
		LCD_Command((uint8_t*)seq + 1, numParm);    // Send command and its parameters
        numParm += 2;                               // skip len and command byte
		if(seq[0] & DRV_CMD_DELAY) {                // If delay bit is set, do delay
			ms = seq[numParm];
			if(ms == 255)
                ms = 500;
			DelayMs(ms);
            numParm++;                              // Delay value counts as one argument
		}
        seq += numParm;                             // Move to next command
	}
}

/**
 * @brief Write a block of data
 *
 */
void LCD_WriteData(const uint16_t *data, uint32_t count){

    if(spidev->cfg & SPI_CFG_DMA){
        spidev->cfg |= SPI_CFG_TRF_16BIT;
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
 * This is called from an interrupt, wait for
 * EOT causes deadlock
 * */
void LCD_DataEnd(void){
    spidev->cfg &= ~(SPI_CFG_TRF_16BIT | SPI_CFG_TRF_CONST);
    //SPI_WaitEOT(spibus);
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
static void LCD_CasRasSet(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    scratch[0] = ST7789_CASET;
    scratch[1] = x1 >> 8;
    scratch[2] = x1;
    scratch[3] = x2 >> 8;
    scratch[4] = x2;
    LCD_Command(scratch, DRV_CMD_PARM4);

    scratch[0] = ST7789_RASET;
    scratch[1] = y1 >> 8;
    scratch[2] = y1;
    scratch[3] = y2 >> 8;
    scratch[4] = y2;
    LCD_Command(scratch, DRV_CMD_PARM4);

    scratch[0] = ST7789_RAMWR;
    LCD_Command(scratch, DRV_CMD_PARM_NONE);
}

/**
 * @brief Setup draw area
 *
 * \param x : X position
 * \param y : Y position
 * \param w : Width
 * \param h : Height
 */
void LCD_Window(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
    SPI_WaitEOT(spidev);
    LCD_CS0;
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

    LCD_Window(x, y, w, h);

    if(spidev->cfg & SPI_CFG_DMA){
        spidev->cfg |= SPI_CFG_TRF_16BIT | SPI_CFG_TRF_CONST;
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
void LCD_WriteArea(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t *data){
    uint32_t count = w * h;

    if(!count){
        return;
    }

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
uint8_t LCD_Init(void *driver)
{
    if(driver == NULL){
        return 0;
    }

    drvlcd = (drvlcdspi_t*)driver;
    spidev = drvlcd->spidev;

    if(spidev->cfg & SPI_CFG_DMA){
        SPI_SetEOT(spidev, LCD_DataEnd);
    }

    LCD_CD1;
    LCD_CS1;
    LCD_BKL0;
    LCD_RST0;
    DelayMs(2);
    LCD_RST1;
    DelayMs(5);

    // SW Reset requires CS pin release
    scratch[0] = ST7789_SWRESET;
    LCD_CS0;
    LCD_Command(scratch, DRV_CMD_PARM_NONE);
    LCD_CS1;

    DelayMs(5);

    LCD_CS0;
    LCD_InitSequence(st7789_sequence);
    LCD_CS1;

    _width  = drvlcd->w;
    _height = drvlcd->h;

    return 1;
}

/**
 * @brief command LCMCTRL can be used to
 * facilitate orientation change
 *
 * */
void LCD_SetOrientation(drvlcdorientation_t m) {

    switch (m) {
    case LCD_PORTRAIT:
        scratch[1] = DEFAULT_MADCTL;
        _width  = drvlcd->w;
        _height = drvlcd->h;
        break;
    case LCD_LANDSCAPE:
        scratch[1] = DEFAULT_MADCTL | (ST7789_MADCTL_MV | ST7789_MADCTL_MX);
        _width  = drvlcd->h;
        _height = drvlcd->w;
        break;
    case LCD_REVERSE_PORTRAIT:
        scratch[1] = DEFAULT_MADCTL | (ST7789_MADCTL_MY | ST7789_MADCTL_MX);
        _width  = drvlcd->w;
        _height = drvlcd->h;
        break;
    case LCD_REVERSE_LANDSCAPE:
        scratch[1] = DEFAULT_MADCTL | (ST7789_MADCTL_MV | ST7789_MADCTL_MY);
        _width  = drvlcd->h;
        _height = drvlcd->w;
        break;

    default:
        return;
    }

    SPI_WaitEOT(spidev);

    scratch[0] = ST7789_MADCTL;

    LCD_CS0;
    LCD_Command(scratch, DRV_CMD_PARM1);
    LCD_CS1;
}

/**
 * @brief Screen scroll
 *
 */
void LCD_Scroll(uint16_t sc){

    SPI_WaitEOT(spidev);

    scratch[0] = ST7789_VSCSAD;
    scratch[1] = sc;

    LCD_CS0;
    LCD_Command(scratch, DRV_CMD_PARM1);
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

/**
 * @brief Driver specific functionality
 * TODO: create command specification
 *
 * | FUN | PARMS ...
 * @param ptr
 * @return
 */
uint32_t LCD_DirectCommand(void *ptr)
{
    uint8_t *msg = ptr;

    typedef struct {
        uint8_t x,y,w,h; // This may cause problems since vres is 320pixel
        uint8_t *data;
    }pixel18bit_t;

    SPI_WaitEOT(spidev);

    switch (msg[0])
    {
        case 0x40: // direct command to controller
            // FUN ! LEN | CMD | PARAM ...
            LCD_CS0;
            LCD_Command(msg + 2, msg[1]);
            LCD_CS1;
            break;

        case 0x50: // Config 18bit data
            scratch[0] = ST7789_COLMOD;
            scratch[1] = COLOR_MODE_262K | COLOR_MODE_18BIT;

            LCD_CS0;
            LCD_Command(scratch, DRV_CMD_PARM1);
            LCD_CS1;
            break;

        case 0x51: // Config 16bit data
            scratch[0] = ST7789_COLMOD;
            scratch[1] = COLOR_MODE_65K | COLOR_MODE_16BIT;

            LCD_CS0;
            LCD_Command(scratch, DRV_CMD_PARM1);
            LCD_CS1;
            break;

        case 0x52: // transfer 18bit data
        {
            pixel18bit_t *buffer = (pixel18bit_t*)(msg + 4); // for this command, skip 4 bytes to keep struct aligned
            uint8_t *data = buffer->data;
            uint32_t count = buffer->w * buffer->h * 3;

            LCD_CS0;
            LCD_CasRasSet(buffer->x, buffer->y, buffer->x + (buffer->w - 1), buffer->y + (buffer->h - 1));

            if(spidev->cfg & SPI_CFG_DMA){
                SPI_TransferDMA(spidev, (uint8_t*)data, count);
            }else{
                while(count--)
                    SPI_Transfer(spidev, data++, 1);
                LCD_CS1;
            }
            break;
        }
    }

    return 0;
}