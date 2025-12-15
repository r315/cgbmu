#include <stddef.h>
#include "at32f4xx.h"
#include "dma_at32f4xx.h"
#include "gpio_at32f4xx.h"
#include "spi.h"
#include "gpio.h"
#include "clock.h"

#ifndef USE_STDPERIPH_DRIVER
#define RCC_APB2PERIPH_SPI1         ((uint32_t)0x00001000)
#define RCC_APB1PERIPH_SPI2         ((uint32_t)0x00004000)
#endif

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

typedef struct{
    SPI_Type *spi;
    dmatype_t dma_tx;
    void (*eot)(void);          // User end of transfer call back
    uint32_t trf_counter;       // Transfer counter, used when data so be transferred is greater than 65535
    uint16_t data_word;
}hspi_t;

static hspi_t hspia = {
    .spi = SPI1,
    .trf_counter = 0,
    .eot = NULL
}, hspib = {
    .spi = SPI2,
    .trf_counter = 0,
    .eot = NULL
};


/**
 * @brief DMA Interrupt handler
 * */
static void spi_eot(hspi_t *hspi)
{
    SPI_Type *spi = hspi->spi;
    DMA_Channel_Type *dma = hspi->dma_tx.per;

    if(hspi->trf_counter > 0x10000UL){
        hspi->trf_counter -= 0x10000UL;
        dma->TCNT = (hspi->trf_counter > 0x10000UL) ? 0xFFFFUL : hspi->trf_counter;
        dma->CHCTRL |= DMA_CHCTRL1_CHEN;
    }else{
        if(spi->STS & SPI_STS_OVR){
            //dummy read for clearing OVR flag
            hspi->trf_counter = spi->DT;
        }

        spi->CTRL2 &= ~(SPI_CTRL2_DMATEN);

        if(hspi->eot){
            while(spi->STS & SPI_STS_BSY);
            hspi->eot();
        }

        hspi->trf_counter = 0;
    }
}

/**
 * @brief Configures baud rate by dividing Fpckl
 * by 2, 4, 8, 16, 32, 64, 128 or 256.
 *
 * spi peripheral must be enabled afterwards
 *
 * */
static void SPI_SetFreq(SPI_Type *spi, uint32_t freq)
{
    uint32_t div;
    uint32_t br = 8;

#ifdef USE_STDPERIPH_DRIVER
    RCC_ClockType clocks;
    RCC_GetClocksFreq(&clocks);
    div = ((spi == SPI1) ? clocks.APB2CLK_Freq : clocks.APB1CLK_Freq) / (1000 * freq);
#else
    sysclock_t clocks;
    CLOCK_GetAll(&clocks);
    div = ((spi == SPI1) ? clocks.pclk2 : clocks.pclk1) / (1000 * freq);
#endif

    if(div > 256){
        div = 256;
    }else if(div < 2){
        div = 2;
    }

    do{--br;}while((uint32_t)(2 << br) > div);

    spi->CTRL1 &= ~(SPI_CTRL1_SPIEN | SPI_CTRL1_MCLKP);
    spi->CTRL1 |= (br << SPI_CTRL1_MDIV_Pos);
}

static inline void spi1Eot(void){spi_eot(&hspia);}
static inline void spi2Eot(void){spi_eot(&hspib);}
/**
 * Public API
 * */

/**
 * @brief Init
 * */
uint32_t SPI_Init(spibus_t *spibus)
{
    hspi_t *hspi;

    switch(spibus->bus){
        case SPI_BUS0:
        case SPI_BUS2:
            RCC->APB2EN |= RCC_APB2PERIPH_SPI1;
            hspi = &hspia;
            break;

        case SPI_BUS1:
        case SPI_BUS3:
            RCC->APB1EN |= RCC_APB1PERIPH_SPI2;
            hspi = &hspib;
            break;

        default :
            return SPI_ERR_PARM;
    }

    hspi->spi->CTRL1 = SPI_CTRL1_MSTEN;

    SPI_SetFreq(hspi->spi, spibus->freq);

    if(spibus->cfg & SPI_CFG_CS){
        hspi->spi->CTRL2 |=  SPI_CTRL2_NSSOE;
    }else{
        // in master mode if not using HW CS, CS pin must keeped high
        hspi->spi->CTRL1 |= SPI_CTRL1_ISS | SPI_CTRL1_SWNSSEN;
    }

    hspi->spi->CTRL1 |= SPI_CTRL1_SPIEN;

    if(spibus->cfg & SPI_CFG_DMA){
        hspi->dma_tx.dst = (void*)&hspi->spi->DT;
        hspi->dma_tx.dsize = DMA_CCR_PSIZE_16;
        hspi->dma_tx.src = NULL;
        hspi->dma_tx.ssize = DMA_CCR_MSIZE_16;
        hspi->dma_tx.dir = DMA_DIR_M2P;

        if(hspi->spi == SPI1){
            hspi->dma_tx.eot = spi1Eot;
            DMA_Config(&hspi->dma_tx, DMA1_REQ_SPI1_TX);
        }else{
            hspi->dma_tx.eot = spi2Eot;
            DMA_Config(&hspi->dma_tx, DMA1_REQ_SPI2_TX);
        }
    }

    /**
     * This configures all pins used by SPI.
     * A pin that is used for other function than SPI,
     * must be reconfigured after calling SPI_Init()
     */
    switch(spibus->bus){
        case SPI_BUS0: // SPI1 default pins
            GPIO_Config(PA_5, GPIO_SPI1_SCK);
            GPIO_Config(PA_6, GPIO_SPI1_MISO);
            GPIO_Config(PA_7, GPIO_SPI1_MOSI);
            if((spibus->cfg & SPI_CFG_CS) != 0){
                GPIO_Config(PA_4, GPIO_SPI1_CS);
            }
            break;

        case SPI_BUS2: // SPI1 remapped
            // To use PB3 as SPI2 sclk, SPI2 has to be remapped
            RCC->APB2EN |= RCC_APB2EN_AFIOEN;
            AFIO->MAP = (AFIO->MAP & ~(7 << 24)) | AFIO_MAP_SWJTAG_CONF_JTAGDISABLE;
            AFIO->MAP5 = (AFIO->MAP5 & ~(0xFF << 16)) | AFIO_MAP5_SPI1_GRMP;

            GPIO_Config(PB_3, GPIO_SPI1_SCK);
            GPIO_Config(PB_4, GPIO_SPI1_MISO);
            GPIO_Config(PB_5, GPIO_SPI1_MOSI);
            if((spibus->cfg & SPI_CFG_CS) != 0){
                GPIO_Config(PA_15, GPIO_SPI1_CS);
            }
            break;

        case SPI_BUS1: // SPI2 default pins
            GPIO_Config(PB_13, GPIO_SPI2_SCK);
            GPIO_Config(PB_14, GPIO_SPI2_MISO);
            GPIO_Config(PB_15, GPIO_SPI2_MOSI);
            if((spibus->cfg & SPI_CFG_CS) != 0){
                GPIO_Config(PB_12, GPIO_SPI2_CS);
            }
            break;

        case SPI_BUS3: // SPI2 Remapped
            // To use PB3 as SPI2 sclk, SPI2 has to be remapped
            RCC->APB2EN |= RCC_APB2EN_AFIOEN;
            AFIO->MAP = (AFIO->MAP & ~(7 << 24)) | AFIO_MAP_SWJTAG_CONF_JTAGDISABLE;
            AFIO->MAP5 = (AFIO->MAP5 & ~(0xFF << 16)) | AFIO_MAP5_SPI2_GRMP;

            GPIO_Config(PB_3, GPIO_SPI2_SCK);
            GPIO_Config(PB_4, GPIO_SPI2_MISO);
            GPIO_Config(PB_5, GPIO_SPI2_MOSI);
            if((spibus->cfg & SPI_CFG_CS) != 0){
                GPIO_Config(PA_15, GPIO_SPI2_CS);
            }
            break;
    }

    hspi->trf_counter = 0;
    spibus->handle = hspi;

    return SPI_OK;
}

/**
 * @brief Make single data exchange on spi bus
 *
 * \param spibus : Pointer to spi device to be used
 * \param data  : Data to be transmitted
 *
 * \return Received data
 * */
uint16_t SPI_Xchg(spibus_t *spibus, uint8_t *data){
    SPI_Type *spi = ((hspi_t*)spibus->handle)->spi;

    if(spibus->cfg & SPI_CFG_TRF_16BIT){
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
void SPI_Transfer(spibus_t *spibus, uint8_t *src, uint32_t count){
    SPI_Type *spi = ((hspi_t*)spibus->handle)->spi;

    if(spibus->cfg & SPI_CFG_TRF_16BIT){
        spi->CTRL1 |= SPI_CTRL1_DFF16;
        while(count--){
            *((__IO uint16_t *)&spi->DT) = *(uint16_t*)src++;
            while((spi->STS & SPI_STS_TE) == 0);
            while((spi->STS & SPI_STS_BSY) != 0);
        }
    }else{
        spi->CTRL1 &= ~(SPI_CTRL1_DFF16);
        while(count--){
            while((spi->STS & SPI_STS_TE) == 0);
            *((__IO uint8_t *)&spi->DT) = *src++;
            while((spi->STS & SPI_STS_BSY) != 0);
            //*(src++) = *((__IO uint8_t *)&spi->DT);
        }
    }
}

/**
 * @brief Write data to SPI using DMA controller
 *
 * \param data  : Pointer to data
 * \param count : total number of transfers
 * */
void SPI_TransferDMA(spibus_t *spibus, uint8_t *src, uint32_t count)
{
    hspi_t *hspi = (hspi_t*)spibus->handle;
    SPI_Type *spi = hspi->spi;
    DMA_Channel_Type *dma = hspi->dma_tx.stream;

    if(spibus->cfg & SPI_CFG_TRF_16BIT){
        spi->CTRL1 |= SPI_CTRL1_DFF16;
    }else{
        spi->CTRL1 &= ~(SPI_CTRL1_DFF16);
    }

    if(spibus->cfg & SPI_CFG_TRF_CONST){
        hspi->data_word = *(uint16_t*)src;  // get constant value
        src = (uint8_t*)&hspi->data_word;   // set source pointer to constant value variable
        dma->CHCTRL &= ~(DMA_CHCTRL1_MINC);
    }else{
        dma->CHCTRL |= DMA_CHCTRL1_MINC;
    }

    spi->CTRL2 |= SPI_CTRL2_DMATEN;

    hspi->trf_counter = count;

    dma->CMBA = (uint32_t)src;
    dma->TCNT = (hspi->trf_counter > 0x10000) ? 0xFFFF : hspi->trf_counter;

    dma->CHCTRL |= DMA_CHCTRL1_CHEN;
}

/**
 * @brief Wait for end of DMA transfer
 * This shouldn't be called from an interrupt
 * */
void SPI_WaitEOT(spibus_t *spibus){
    hspi_t *hspi = (hspi_t*)spibus->handle;
    #if 0
    while(hspi->spi->STS & SPI_STS_BSY){
    #else
    while(hspi->trf_counter){
    #endif
        //LED_TOGGLE;
    }
}

/**
 * @brief Configure a end of transfer callback
 * @param spibus
 * @param eot
 */
void SPI_SetEOT(spibus_t *spibus, void(*eot)(void))
{
    hspi_t *hspi = spibus->handle;
    hspi->eot = eot;
}
