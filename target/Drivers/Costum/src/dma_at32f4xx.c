#include <stdint.h>
#include <stddef.h>
#include "at32f4xx.h"
#include "dma_at32f4xx.h"
#include "dma.h"

typedef void (*eot_t)(void);
static eot_t ch_eot[DMA_NUM_CHANNELS];

/**
 * @brief Request dma controller/channel
 * \param ctrl    : pointer to hold controller and channel
 * \param request : Request in format
 *                   [9:8] -> DMA number
 *                   [7:4] -> Channel number
 *                   [3:0] -> Request source
 * */
void DMA_Config(dmatype_t *dma, uint32_t request){
    DMA_Channel_Type *stream; 
    uint8_t ch_num;

    if((request & DMA_NUMBER_MASK) == 0){
        RCC_AHBPeriphClockCmd(RCC_AHBPERIPH_DMA1, ENABLE);
        ch_num = (request & DMA_CHANNEL_MASK) >> DMA_CHANNEL_POS;
        dma->per = DMA1;
        stream = (DMA_Channel_Type*)((uint32_t)DMA1_Channel1 + (ch_num * 0x14));

        uint32_t config = 0;

        switch(dma->dir){
            case DMA_DIR_P2P:
                config |=   DMA_CCR_MSIZE_CFG(dma->ssize) | DMA_CCR_PSIZE_CFG(dma->dsize);
                break;

            case DMA_DIR_P2M:
                config |=   DMA_CCR_PSIZE_CFG(dma->ssize) | 
                            DMA_CCR_MSIZE_CFG(dma->dsize) |
                            DMA_CHCTRL1_PINC;                   // Increment destination
                break;

            case DMA_DIR_M2P:
                config |=   DMA_CCR_PSIZE_CFG(dma->dsize) | 
                            DMA_CCR_MSIZE_CFG(dma->ssize) | 
                            DMA_CHCTRL1_MINC | DMA_CHCTRL1_DIR; // Increment source
                break;

            case DMA_DIR_M2M:
                config |=   DMA_CCR_PSIZE_CFG(dma->dsize) | 
                            DMA_CCR_MSIZE_CFG(dma->ssize) | 
                            DMA_CHCTRL1_MEMTOMEM | DMA_CHCTRL1_MINC | DMA_CHCTRL1_PINC;
                break;
        }

        if(dma->eot){
            config |= DMA_CHCTRL1_TCIE;
            NVIC_EnableIRQ(DMA1_Channel1_IRQn + ch_num);
        }

        stream->CHCTRL = config;
        stream->CPBA = (uint32_t)dma->dst;
        dma->stream = stream;
        
        ch_eot[ch_num] = dma->eot;
    }else{
        RCC_AHBPeriphClockCmd(RCC_AHBPERIPH_DMA2, ENABLE);
        //TODO:
    }
}

static inline void dma_irq_handler(uint8_t ch_num)
{
    if(ch_eot[ch_num] != NULL){
        ch_eot[ch_num]();
    }
}

void DMA1_Channel3_IRQHandler(void)
{ 
    if(DMA1->ISTS & DMA_ISTS_GIF3){
        dma_irq_handler(2);
        DMA1->ICLR = (DMA_ICLR_CGIF3 | DMA_ICLR_CTCIF3 | DMA_ICLR_CERRIF3);
    }
}