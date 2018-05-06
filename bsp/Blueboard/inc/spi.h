#ifndef _spi_h_
#define _spi_h_

#include <LPC17xx.h> 

/* SSPxSR - bit definitions. */
#define TFE     0x01
#define TNF     0x02
#define RNE     0x04
#define RFF     0x08
#define BSY     0x10 

//---------------------------------
//
//---------------------------------
void SPI_Init (void);
//void SPI_HI_SPEED(BOOL on);
uint8_t SPI(uint8_t outb); 

#endif
