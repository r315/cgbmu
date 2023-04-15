#include <stddef.h>
#include "at32f4xx.h"
#include "dma_at32f4xx.h"
#include "gpio_at32f4xx.h"
#include "spi.h"
#include "gpio.h"

#define SPI_CTRL1_MDIV_Pos          3
#define SPI_CTRL1_MDIV_DIV2         (0 << SPI_CTRL1_MDIV_Pos)
#define SPI_CTRL1_MDIV_DIV4         (1 << SPI_CTRL1_MDIV_Pos)
#define SPI_CTRL1_MDIV_DIV8         (2 << SPI_CTRL1_MDIV_Pos)
#define SPI_CTRL1_MDIV_DIV16        (3 << SPI_CTRL1_MDIV_Pos)
#define SPI_CTRL1_MDIV_DIV32        (4 << SPI_CTRL1_MDIV_Pos)
#define SPI_CTRL1_MDIV_DIV64        (5 << SPI_CTRL1_MDIV_Pos)
#define SPI_CTRL1_MDIV_DIV128       (6 << SPI_CTRL1_MDIV_Pos)
#define SPI_CTRL1_MDIV_DIV256       (7 << SPI_CTRL1_MDIV_Pos)
#define SPI_CTRL1_MDIV_DIV512       (8 << SPI_CTRL1_MDIV_Pos)
#define SPI_CTRL1_MDIV_DIV1024      (9 << SPI_CTRL1_MDIV_Pos)

#define SPIDEV_SET_FLAG(_D, _F)   _D->flags |= _F
#define SPIDEV_CLR_FLAG(_D, _F)   _D->flags &= ~(_F)
#define SPIDEV_GET_FLAG(_D, _F)   !!(_D->flags & _F)

static spibus_t *spi_eot[2];

/**
 * @brief DMA Interrupt handler
 * */
void SPI_DMA_IRQHandler(spibus_t *spidev){
    SPI_Type *spi = (SPI_Type*)spidev->ctrl;
    DMA_Channel_Type *dma = (DMA_Channel_Type*)spidev->dma.stream;
    
    dma->CHCTRL &= ~(DMA_CHCTRL1_CHEN);
        
    if(spidev->trf_counter > 0x10000UL){
        spidev->trf_counter -= 0x10000UL;
        dma->TCNT = (spidev->trf_counter > 0x10000UL) ? 0xFFFFUL : spidev->trf_counter;
        dma->CHCTRL |= DMA_CHCTRL1_CHEN;
    }else{        
        if(spi->STS & SPI_STS_OVR){
            //dummy read for clearing OVR flag
            spidev->trf_counter = spi->DT;
        }      

        spi->CTRL2 &= ~(SPI_CTRL2_DMATEN);

        spidev->trf_counter = 0;

        if(spidev->eot_cb){
            spidev->eot_cb();
        }

        SPIDEV_CLR_FLAG(spidev, SPI_BUSY | SPI_DMA_NO_MINC);
    }
}

/**
 * @brief Configures baud rate by dividing Fpckl
 * by 2, 4, 8, 16, 32, 64, 128 or 256.
 * 
 * spi peripheral must be enabled afterwards 
 * 
 * */
static void SPI_SetFreq(SPI_Type *spi, uint32_t freq){
    RCC_ClockType clocks;
    uint32_t div;
    uint32_t br = 8;

    RCC_GetClocksFreq(&clocks);
    div = ((spi == SPI1) ? clocks.APB2CLK_Freq : clocks.APB1CLK_Freq) / (1000 * freq);

    if(div > 256){
        div = 256;
    }else if(div < 2){
        div = 2;
    }

    do{--br;}while((uint32_t)(2 << br) > div); 

    spi->CTRL1 &= ~(SPI_CTRL1_SPIEN | SPI_CTRL1_MCLKP);
    spi->CTRL1 |= (br << SPI_CTRL1_MDIV_Pos);
}

static inline void spi1Eot(void){ SPI_DMA_IRQHandler(spi_eot[0]);}
static inline void spi2Eot(void){ SPI_DMA_IRQHandler(spi_eot[1]);}
/**
 * Public API
 * */

/**
 * @brief Init
 * */
void SPI_Init(spibus_t *spidev){
    SPI_Type *spi;

    switch(spidev->bus){
        case SPI_BUS0:
            RCC_APB2PeriphClockCmd(RCC_APB2PERIPH_SPI1, ENABLE);
            RCC_APB2PeriphResetCmd(RCC_APB2PERIPH_SPI1, ENABLE);
            RCC_APB2PeriphResetCmd(RCC_APB2PERIPH_SPI1, DISABLE);
            spi = SPI1;
            spi_eot[0] = spidev;
            break;

        case SPI_BUS1:
            RCC_APB1PeriphClockCmd(RCC_APB1PERIPH_SPI2, ENABLE);
            RCC_APB1PeriphResetCmd(RCC_APB1PERIPH_SPI2, ENABLE);
            RCC_APB1PeriphResetCmd(RCC_APB1PERIPH_SPI2, DISABLE);
            spi = SPI2;
            spi_eot[1] = spidev;
            break;

        default : return;
    }
    
    RCC_AHBPeriphClockCmd(RCC_AHBPERIPH_DMA1, ENABLE);

    spi->CTRL1 = SPI_CTRL1_MSTEN;
    
    SPI_SetFreq(spi, spidev->freq);

    if((spidev->flags & SPI_HW_CS) != 0){
        spi->CTRL2 |=  SPI_CTRL2_NSSOE;
    }            

    spi->CTRL1 |= SPI_CTRL1_SPIEN;
    
    spidev->trf_counter = 0;

    spidev->dma.dst = (void*)&spi->DT;
    spidev->dma.dsize = DMA_CCR_PSIZE_16;
    spidev->dma.src = NULL;
    spidev->dma.ssize = DMA_CCR_MSIZE_16;
    spidev->dma.dir = DMA_DIR_M2P;
    spidev->dma.eot = (spi == SPI1) ? spi1Eot : spi2Eot;
    DMA_Config(&spidev->dma, DMA1_REQ_SPI1_TX);

    // Configure default pins, 
    // remapped or sw cs pin must be configure manually
    if(spi == SPI1){
        GPIO_Config(PA_5, GPIO_SPI_SCK);
        GPIO_Config(PA_6, GPIO_SPI_MISO);
        GPIO_Config(PA_7, GPIO_SPI_MOSI);
        if((spidev->flags & SPI_HW_CS) != 0){
            GPIO_Config(PA_4, GPIO_SPI_CS);
        }
    }else{
        GPIO_Config(PB_13, GPIO_SPI_SCK);
        GPIO_Config(PB_14, GPIO_SPI_MISO);
        GPIO_Config(PB_15, GPIO_SPI_MOSI);
        if((spidev->flags & SPI_HW_CS) != 0){
            GPIO_Config(PA_12, GPIO_SPI_CS);
        }
    }

    spidev->ctrl = spi;
    spidev->flags |= SPI_ENABLED;
}

/**
 * @brief Make single data exchange on spi bus
 *
 * \param spidev : Pointer to spi device to be used
 * \param data  : Data to be transmitted
 *
 * \return Received data
 * */
uint16_t SPI_Xchg(spibus_t *spidev, uint8_t *data){
    SPI_Type *spi = (SPI_Type*)spidev->ctrl;

    if(spidev->flags & SPI_16BIT){
        spi->CTRL1 |= SPI_CTRL1_DFF16;
        *((__IO uint16_t *)&spi->DT) = *(uint16_t*)data;
    }else{
        spi->CTRL1 &= ~(SPI_CTRL1_DFF16);
        *((__IO uint8_t *)&spi->DT) = *data;
    }

    while((spi->STS & SPI_STS_TE) == 0);
    while((spi->STS & SPI_STS_BSY) != 0);

    return spi->DT;
}

/**
 * @brief Write data to SPI, blocking
 * 
 * \param src   : Pointer to source data
 * \param count : total number of bytes to transfer
 * */
void SPI_Transfer(spibus_t *spidev, uint8_t *src, uint32_t count){
    SPI_Type *spi = (SPI_Type*)spidev->ctrl;

    if(spidev->flags & SPI_16BIT){
        spi->CTRL1 |= SPI_CTRL1_DFF16;
        while(count--){
            *((__IO uint16_t *)&spi->DT) = *(uint16_t*)src++;
            while((spi->STS & SPI_STS_TE) == 0);
            while((spi->STS & SPI_STS_BSY) != 0);
        }
    }else{
        spi->CTRL1 &= ~(SPI_CTRL1_DFF16);
        while(count--){
            *((__IO uint8_t *)&spi->DT) = *src++;
            while((spi->STS & SPI_STS_TE) == 0);
            while((spi->STS & SPI_STS_BSY) != 0);
        }
    } 
}

/**
 * @brief Write data to SPI using DMA controller
 * 
 * \param data  : Pointer to data
 * \param count : total number of transfers
 * */
void SPI_TransferDMA(spibus_t *spidev, uint8_t *src, uint32_t count){
    static uint16_t sdata;
    SPI_Type *spi = (SPI_Type*)spidev->ctrl;
    DMA_Channel_Type *dma = (DMA_Channel_Type*)spidev->dma.stream;

    if(spidev->flags & SPI_16BIT){
        //spi->CTRL1 &= ~SPI_CTRL1_SPIEN;
        spi->CTRL1 |= SPI_CTRL1_DFF16; // | SPI_CTRL1_SPIEN;
    }else{
        spi->CTRL1 &= ~(SPI_CTRL1_DFF16);
    }

    if(spidev->flags & SPI_DMA_NO_MINC){
        sdata = *(uint8_t*)src;
        src = (uint8_t*)&sdata;
        dma->CHCTRL &= ~(DMA_CHCTRL1_MINC);
    }else{
        dma->CHCTRL |= DMA_CHCTRL1_MINC;
    }

    spi->CTRL2 |= SPI_CTRL2_DMATEN;

    spidev->trf_counter = count;

    dma->CMBA = (uint32_t)src;
    dma->TCNT = (spidev->trf_counter > 0x10000) ? 0xFFFF : spidev->trf_counter;
    
    SPIDEV_SET_FLAG(spidev, SPI_BUSY);
    
    dma->CHCTRL |= DMA_CHCTRL1_CHEN;
}

/**
 * @brief Wait for end of DMA transfer
 * */
void SPI_WaitEOT(spibus_t *spidev){    
    #if 1
    SPI_Type *spi = (SPI_Type*)spidev->ctrl;
    while(spi->STS & SPI_STS_BSY){
    #else
    while(SPIDEV_GET_FLAG(spidev, SPI_BUSY)){
    #endif
        //LED_TOGGLE;
    }
}