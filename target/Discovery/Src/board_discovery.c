
#include <stdio.h>
#include <errno.h>
//#include "fatfs.h"
#include "board.h"
#include "i2c.h"
#include "pcf8574.h"

#define ENABLE_FS 0

static i2cbus_t i2cbus;

static void SystemClock_Config(void);
void Serial_Init(void);
uint32_t memavail(void);
void dumpBuf(uint8_t *buf, uint32_t off, uint32_t size);

uint32_t GetTick(void) {
	return HAL_GetTick();
}

void DelayMs(uint32_t ms) {
	HAL_Delay(ms);
}

void BOARD_Init(void)
{
    SCB_EnableICache();
    SCB_EnableDCache();

    HAL_Init();

    SystemClock_Config();

    BSP_LED_Init(LED1);
    BSP_LED_Init(LED2);
    
    Serial_Init();

    printf("\e[2J\r");
#if ENABLE_FS
    SD_Init();

    //SD_DumpSector(0);
    fatFs_Init();
#endif

    BSP_PB_Init(BUTTON_USER, BUTTON_MODE_GPIO);

    I2C_Init(&i2cbus);
    io_drv_pcf8574.init(&i2cbus);

    printf("System clock: %luMHz\n",SystemCoreClock/1000000);

    printf("Mem available: %d\n\n", (int)memavail());

    /* Initialize the LCD */
    BSP_LCD_Init();
    
    BSP_LCD_LayerDefaultInit(0, LCD_FB_START_ADDRESS);
}

static void SystemClock_Config(void)
{
    RCC_ClkInitTypeDef RCC_ClkInitStruct;
    RCC_OscInitTypeDef RCC_OscInitStruct;
    HAL_StatusTypeDef ret = HAL_OK;

    /* Enable HSE Oscillator and activate PLL with HSE as source */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 25;
    RCC_OscInitStruct.PLL.PLLN = 400;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 9;
    RCC_OscInitStruct.PLL.PLLR = 7;

    ret = HAL_RCC_OscConfig(&RCC_OscInitStruct);
    if (ret != HAL_OK)
    {
        while (1)
        {
            ;
        }
    }

    /* Activate the OverDrive to reach the 216 MHz Frequency */
    ret = HAL_PWREx_EnableOverDrive();
    if (ret != HAL_OK)
    {
        while (1)
        {
            ;
        }
    }

    /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 clocks dividers */
    RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

    ret = HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7);
    if (ret != HAL_OK)
    {
        while (1)
        {
            ;
        }
    }
}


#if ENABLE_FS
FRESULT scan_files (char* path)
{
    FRESULT res;
    DIR dir;
    UINT i;
    static FILINFO fno;


    res = f_opendir(&dir, path);                       /* Open the directory */
    if (res == FR_OK) {
        for (;;) {
            res = f_readdir(&dir, &fno);                   /* Read a directory item */
            if (res != FR_OK || fno.fname[0] == 0) break;  /* Break on error or end of dir */
            if (fno.fattrib & AM_DIR) {                    /* It is a directory */
                i = strlen(path);
                sprintf(&path[i], "/%s", fno.fname);
                res = scan_files(path);                    /* Enter the directory */
                if (res != FR_OK) break;
                path[i] = 0;
            } else {                                       /* It is a file. */
                printf("%s/%s\n", path, fno.fname);
            }
        }
        f_closedir(&dir);
    }

    return res;
}

void fatFs_Init(void)
{

    if (FATFS_LinkDriver(&SD_Driver, SDPath) != 0)
    {
        printf("FATFS Link Driver fail\n");
        return;
    }

    FRESULT fr = f_mount(&SDFatFS, (TCHAR const *)SDPath, 1);

    if (fr != FR_OK)
    {
        printf("FATFS Fail to mount: %x\n", fr);
        return;
    }

    printf("Mounted File system: %x\n", SDFatFS.fs_type);

    //char buff[256];
    //strcpy(buff, "/");
    //scan_files(buff);

    //FATFS_UnLinkDriver(SDPath);
}

void dumpBuf(uint8_t *buf, uint32_t off, uint32_t size){
    for (uint32_t i = 0; i < size; i++)
    {
        if ((i & 0x0F) == 0)
        {
            putchar('\n');
            printf("%08X: ", (unsigned int)(off + i));
        }
        printf("%02X ", buf[i]);
    }
    putchar('\n');
}

void SD_DumpSector(uint32_t sector)
{
    uint8_t sector_data[BLOCKSIZE];
    uint8_t sdRes;

    //res = BSP_SD_ReadBlocks_DMA((uint32_t*)sector_data, 0, 1);
    sdRes = BSP_SD_ReadBlocks((uint32_t *)sector_data, sector, 1, 1024);
    while (BSP_SD_GetCardState() != SD_TRANSFER_OK)
        ;

    if (sdRes != MSD_OK)
    {
        printf("Fail to read: %x\n", sdRes);
        return;
    }  

    dumpBuf(sector_data, sector, BLOCKSIZE);  
}

void SD_Init(void)
{   
    HAL_SD_CardInfoTypeDef ci;

    switch(BSP_SD_Init()){
        case MSD_OK:
            break;
        case MSD_ERROR_SD_NOT_PRESENT:
            printf("SD card not present\n");
            return;
        default:
            printf("Fail to init card\n");
            return;
    }
    
    BSP_SD_GetCardInfo(&ci);
    printf("\nType: %x\n", (int)ci.CardType);
    printf("Version: %x\n", (int)ci.CardVersion);
    printf("Class: %x\n", (int)ci.Class);
    printf("Relative address: %x\n", (int)ci.RelCardAdd);
    printf("Number of blocks: %x, (%d)\n", (int)ci.BlockNbr, (int)ci.BlockNbr);
    printf("Block Size: %d\n", (int)ci.BlockSize);
    printf("Logical Number of blocks: %x, (%d)\n", (int)ci.LogBlockNbr, (int)ci.LogBlockNbr);
    printf("Logical Block Size: %d\n\n", (int)ci.LogBlockSize);
}

void BOARD_DeInit(void){
    FATFS_UnLinkDriver(SDPath);
}

int access(char *file, int mode)
{
    FRESULT fr;
    FILINFO fno;
    fr = f_stat(file+2, &fno);

    switch (fr)
    {

    case FR_OK:
        printf("Size: %lu\n", fno.fsize);
        printf("Timestamp: %u/%02u/%02u, %02u:%02u\n",
               (fno.fdate >> 9) + 1980, fno.fdate >> 5 & 15, fno.fdate & 31,
               fno.ftime >> 11, fno.ftime >> 5 & 63);
        printf("Attributes: %c%c%c%c%c\n",
               (fno.fattrib & AM_DIR) ? 'D' : '-',
               (fno.fattrib & AM_RDO) ? 'R' : '-',
               (fno.fattrib & AM_HID) ? 'H' : '-',
               (fno.fattrib & AM_SYS) ? 'S' : '-',
               (fno.fattrib & AM_ARC) ? 'A' : '-');
        return 0;

    case FR_NO_FILE:
        printf("It is not exist.\n");
        break;

    default:
        printf("An error occured. (%d)\n", fr);
    }
    return -1;
}
#endif

void __debugbreak(void){
	 asm volatile
    (
        "bkpt #01 \n"
    );
}