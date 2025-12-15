#include <stdint.h>
#include <stddef.h>
#include "at32f4xx.h"
#include "dma_at32f4xx.h"
#include "dma.h"

#ifndef USE_STDPERIPH_DRIVER
#define RCC_AHBPERIPH_DMA1              ((uint32_t)0x00000001)
#define RCC_AHBPERIPH_DMA2              ((uint32_t)0x00000002)
#endif

static dmatype_t *hdma[DMA_NUM_CHANNELS];

/**
 * @brief Request dma controller/channel
 * \param ctrl    : pointer to hold controller and channel
 * \param request : Request in format
 *                   [9:8] -> DMA number
 *                   [7:4] -> Channel number
 *                   [3:0] -> Request source
 * */
uint32_t DMA_Config(dmatype_t *dma, uint32_t request){
    DMA_Channel_Type *stream;
    uint8_t ch_num;
    IRQn_Type irqn;

    ch_num = (request & DMA_CHANNEL_MASK) >> DMA_CHANNEL_POS;

    if(hdma[ch_num] && hdma[ch_num] != dma){
        return 0; // request is already in use
    }


    if((request & DMA_NUMBER_MASK) == 0){
        RCC->AHBEN |= RCC_AHBPERIPH_DMA1;
        dma->per = DMA1;
        irqn = DMA1_Channel1_IRQn + ch_num;
        stream = (DMA_Channel_Type*)((uint32_t)DMA1_Channel1 + (ch_num * 0x14));
    }else{
        RCC->AHBEN |= RCC_AHBPERIPH_DMA2;
        dma->per = DMA2;
        irqn = DMA2_Channel1_IRQn + ((ch_num == 4) ? 3 : ch_num);
        stream = (DMA_Channel_Type*)((uint32_t)DMA2_Channel1 + (ch_num * 0x14));
    }

    uint32_t config = 0;

    switch(dma->dir){
        case DMA_DIR_P2P:
            config |=   DMA_CCR_MSIZE_CFG(dma->ssize) | DMA_CCR_PSIZE_CFG(dma->dsize);
            stream->CPBA = (uint32_t)dma->src;
            stream->CMBA = (uint32_t)dma->dst;
            break;

        case DMA_DIR_P2M:
            config |=   DMA_CCR_PSIZE_CFG(dma->ssize) |
                        DMA_CCR_MSIZE_CFG(dma->dsize) |
                        DMA_CHCTRL1_MINC;                   // Increment destination
            stream->CPBA = (uint32_t)dma->src;
            stream->CMBA = (uint32_t)dma->dst;
            break;

        case DMA_DIR_M2P:
            config |=   DMA_CCR_PSIZE_CFG(dma->dsize) |
                        DMA_CCR_MSIZE_CFG(dma->ssize) |
                        DMA_CHCTRL1_MINC | DMA_CHCTRL1_DIR; // Increment source
            stream->CMBA = (uint32_t)dma->src;
            stream->CPBA = (uint32_t)dma->dst;
            break;

        case DMA_DIR_M2M:
            config |=   DMA_CCR_PSIZE_CFG(dma->dsize) |
                        DMA_CCR_MSIZE_CFG(dma->ssize) |
                        DMA_CHCTRL1_MEMTOMEM | DMA_CHCTRL1_MINC | DMA_CHCTRL1_PINC;
            stream->CPBA = (uint32_t)dma->src;
            stream->CMBA = (uint32_t)dma->dst;
            break;
    }

    config |= DMA_CHCTRL1_TCIE;
    NVIC_EnableIRQ(irqn);

    stream->CHCTRL = config;
    dma->stream = stream;

    hdma[ch_num] = dma;

    return 1;
}

void DMA_Start(dmatype_t *dma)
{
    DMA_Channel_Type *stream = dma->stream;
    uint32_t cfg = stream->CHCTRL & ~(DMA_CHCTRL1_MINC | DMA_CHCTRL1_CIRM);

    stream->TCNT = dma->len;

    if(!dma->single){
        cfg |= DMA_CHCTRL1_CIRM;
    }

    if(dma->dir != DMA_DIR_P2P){
        cfg |= DMA_CHCTRL1_MINC;
    }

    stream->CHCTRL = cfg | DMA_CHCTRL1_CHEN;
}

void DMA_Cancel(dmatype_t *dma)
{
    DMA_Channel_Type *stream = dma->stream;

    stream->CHCTRL &= ~DMA_CHCTRL1_CHEN;
}

uint32_t DMA_GetTransfers(dmatype_t *dma)
{
    DMA_Channel_Type *stream = dma->stream;
    return dma->len - stream->TCNT;
}

void DMA_SetSrc(dmatype_t *dma, void *src)
{
    DMA_Channel_Type *stream = dma->stream;

    switch(dma->dir){
        case DMA_DIR_P2P:
            stream->CPBA = (uint32_t)src;
            break;

        case DMA_DIR_P2M:
            stream->CPBA = (uint32_t)src;
            break;

        case DMA_DIR_M2P:
            stream->CMBA = (uint32_t)src;
            break;

        case DMA_DIR_M2M:
            stream->CPBA = (uint32_t)src;
            break;
    }
}

void DMA_SetDst(dmatype_t *dma, void *dst)
{
    DMA_Channel_Type *stream = dma->stream;

    switch(dma->dir){
        case DMA_DIR_P2P:
            stream->CMBA = (uint32_t)dst;
            break;

        case DMA_DIR_P2M:
            stream->CMBA = (uint32_t)dst;
            break;

        case DMA_DIR_M2P:
            stream->CPBA = (uint32_t)dst;
            break;

        case DMA_DIR_M2M:
            stream->CMBA = (uint32_t)dst;
            break;
    }
}

static inline void dma_irq_handler(dmatype_t *dma)
{
    DMA_Channel_Type *stream = dma->stream;
    uint32_t ctrl = stream->CHCTRL;

    if(!(ctrl & DMA_CHCTRL1_CIRM)){
        // Disable stream if circular mode is disabled
        stream->CHCTRL = ctrl & ~DMA_CHCTRL1_CHEN;
    }

    if(dma->eot != NULL){
        dma->eot();
    }
}

void DMA1_Channel1_IRQHandler(void)
{
    if(DMA1->ISTS & DMA_ISTS_GIF1){
        dma_irq_handler(hdma[0]);
        DMA1->ICLR = (DMA_ICLR_CGIF1 | DMA_ICLR_CTCIF1 | DMA_ICLR_CERRIF1);
    }
}

void DMA1_Channel2_IRQHandler(void)
{
    if(DMA1->ISTS & DMA_ISTS_GIF2){
        dma_irq_handler(hdma[1]);
        DMA1->ICLR = (DMA_ICLR_CGIF2 | DMA_ICLR_CTCIF2 | DMA_ICLR_CERRIF2);
    }
}

void DMA1_Channel3_IRQHandler(void)
{
    if(DMA1->ISTS & DMA_ISTS_GIF3){
        dma_irq_handler(hdma[2]);
        DMA1->ICLR = (DMA_ICLR_CGIF3 | DMA_ICLR_CTCIF3 | DMA_ICLR_CERRIF3);
    }
}

void DMA1_Channel4_IRQHandler(void)
{
    if(DMA1->ISTS & DMA_ISTS_GIF4){
        dma_irq_handler(hdma[3]);
        DMA1->ICLR = (DMA_ICLR_CGIF4 | DMA_ICLR_CTCIF4 | DMA_ICLR_CERRIF4);
    }
}

void DMA1_Channel5_IRQHandler(void)
{
    if(DMA1->ISTS & DMA_ISTS_GIF5){
        dma_irq_handler(hdma[4]);
        DMA1->ICLR = (DMA_ICLR_CGIF5 | DMA_ICLR_CTCIF5 | DMA_ICLR_CERRIF5);
    }
}

void DMA1_Channel6_IRQHandler(void)
{
    if(DMA1->ISTS & DMA_ISTS_GIF6){
        dma_irq_handler(hdma[5]);
        DMA1->ICLR = (DMA_ICLR_CGIF6 | DMA_ICLR_CTCIF6 | DMA_ICLR_CERRIF6);
    }
}

void DMA1_Channel7_IRQHandler(void)
{
    if(DMA1->ISTS & DMA_ISTS_GIF7){
        dma_irq_handler(hdma[6]);
        DMA1->ICLR = (DMA_ICLR_CGIF7 | DMA_ICLR_CTCIF7 | DMA_ICLR_CERRIF7);
    }
}

void DMA2_Channel1_IRQHandler(void)
{
    if(DMA2->ISTS & DMA_ISTS_GIF1){
        dma_irq_handler(hdma[8]);
        DMA2->ICLR = (DMA_ICLR_CGIF1 | DMA_ICLR_CTCIF1 | DMA_ICLR_CERRIF1);
    }
}

void DMA2_Channel2_IRQHandler(void)
{
    if(DMA2->ISTS & DMA_ISTS_GIF2){
        dma_irq_handler(hdma[9]);
        DMA2->ICLR = (DMA_ICLR_CGIF2 | DMA_ICLR_CTCIF2 | DMA_ICLR_CERRIF2);
    }
}

void DMA2_Channel3_IRQHandler(void)
{
    if(DMA2->ISTS & DMA_ISTS_GIF3){
        dma_irq_handler(hdma[10]);
        DMA2->ICLR = (DMA_ICLR_CGIF3 | DMA_ICLR_CTCIF3 | DMA_ICLR_CERRIF3);
    }
}

void DMA2_Channel4_5_IRQHandler(void)
{
    if(DMA2->ISTS & DMA_ISTS_GIF4){
        dma_irq_handler(hdma[11]);
        DMA2->ICLR = (DMA_ICLR_CGIF4 | DMA_ICLR_CTCIF4 | DMA_ICLR_CERRIF4);
    }

    if(DMA2->ISTS & DMA_ISTS_GIF5){
        dma_irq_handler(hdma[12]);
        DMA2->ICLR = (DMA_ICLR_CGIF5 | DMA_ICLR_CTCIF5 | DMA_ICLR_CERRIF5);
    }
}