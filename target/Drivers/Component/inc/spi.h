/**
* @file     spi.h
* @brief    Contains the spi API header.
*
* @version   1.0
* @date      5 Nov. 2016
* @author    Hugo Reis
**********************************************************************/

#ifndef _SPI_H_
#define _SPI_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "dma.h"

/*
SPI modes                                 0    1    2    3
               _   _   _   _
CPOL = 0    __| |_| |_| |_| |__           X    X
            __   _   _   _   __
CPOL = 1      |_| |_| |_| |_|                       X    X
          __ ___ ___ ___ ___ ___ ___
CPHA = 0  __X___X___X___X___X___X___      X         X
          ____ ___ ___ ___ ___ ___ ___
CPHA = 1  ____X___X___X___X___X___X___         X         X

*/

#define SPI_BUS0    0
#define SPI_BUS1    1
#define SPI_BUS2    2
#define SPI_BUS3    3

enum spimode_e{
    SPI_MODE0 = 0x00,
    SPI_MODE1 = 0x40,
    SPI_MODE2 = 0x80,
    SPI_MODE3 = 0xC0,
};

enum spicfg_e{
    SPI_CFG_CS        = (1 << 0), /* HW chip select*/
    SPI_CFG_DMA       = (1 << 1),
    SPI_CFG_TRF_CONST = (1 << 2),
    SPI_CFG_TRF_16BIT = (1 << 3)
};

enum spierr_e {
    SPI_OK,
    SPI_ERR,
    SPI_ERR_PARM
};

typedef struct spibus{
    void *handle;           // Handle to internal spi structure
    uint8_t  bus;           // bus number 0,1...
    uint32_t freq;          // Clock frequency in khz
    uint8_t  cfg;           // Configuration, 7:6 Mode, 5:0 flags
}spibus_t;

uint32_t SPI_Init(spibus_t *spibus);
void SPI_Transfer(spibus_t *spibus, uint8_t *src, uint32_t count);
void SPI_TransferDMA(spibus_t *spibus, uint8_t *data, uint32_t count);
void SPI_WaitEOT(spibus_t *spibus);
void SPI_SetEOT(spibus_t *spibus, void(*eot)(void));
uint16_t SPI_Xchg(spibus_t *spibus, uint8_t *data);

#ifdef __cplusplus
}
#endif

#endif /* _SPI_H_ */
