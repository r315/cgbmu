
#include "spi.h"

//---------------------------------
//
//---------------------------------
void SPI_Init (void) 
{

  // Initialize and enable the SSP Interface module.
  LPC_SC->PCONP       |= (1 << 21);           /* Enable power to SSPI0 block */ 
  LPC_SSP0->CR0        = 0x0000;
  LPC_SSP0->CR1        = 0x0000; 

  // SCK, MISO, MOSI are SSP pins. 
  LPC_PINCON->PINSEL0 &= ~(3UL<<30);          /* P0.15 cleared               */
  LPC_PINCON->PINSEL0 |=  (2UL<<30);          /* P0.15 SCK0                  */
  LPC_PINCON->PINSEL1 &= ~((3<<2) | (3<<4)); /* P0.17, P0.18 cleared        */
  LPC_PINCON->PINSEL1 |=  ((2<<2) | (2<<4)); /* P0.17 MISO0, P0.18 MOSI0    */

  LPC_SC->PCLKSEL1    &= ~(3<<10);            /* PCLKSP0 = CCLK/4 ( 25MHz)   */
  LPC_SC->PCLKSEL1    |=  (1<<10);            /* PCLKSP0 = CCLK   (100MHz)   */

  LPC_SSP0->CPSR       = 250;                 /* 100MHz / 250 = 400kBit      */
                                              /* maximum of 18MHz is possible*/    
  LPC_SSP0->CR0        = 0x0007;              /* 8Bit, CPOL=0, CPHA=0        */
  LPC_SSP0->CR1        = 0x0002;              /* SSP0 enable, master         */

} 
//---------------------------------
//spi_hi_speed
//---------------------------------
void _SPI_HI_SPEED(char on) 
{
  // Set a SPI clock to low/high speed for SD/MMC. 

  if (on) 
    // Max. 12 MBit used for Data Transfer. 
	LPC_SSP0->CPSR = 2;                      // 100MHz / 10 = 10MBit          
  else 
    // Max. 400 kBit used in Card Initialization. 
	LPC_SSP0->CPSR = 250;                     // 100MHz / 250 = 400kBit      
  
}																									 
//---------------------------------*/
//
//---------------------------------
uint8_t SPI(uint8_t outb) 
{

  LPC_SSP0->DR = outb;
  while (LPC_SSP0->SR & BSY);                 /* Wait for transfer to finish */
  return (LPC_SSP0->DR);                      /* Return received value       */
}
