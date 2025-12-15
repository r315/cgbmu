#ifndef _dma_h_
#define _dma_h_

#include <stdint.h>

typedef struct dmactrl{
   void *per;
   void *stream;
   void *src;
   void *dst;
   uint32_t len;            // Number of elements on transfer, rename to count?
   struct {
      uint32_t ssize : 5;
      uint32_t dsize : 5;
      uint32_t dir : 2;
      uint32_t single : 1;  // Single transfer
   };
   void (*eot)(void);
}dmatype_t;

enum {
   DMA_DIR_P2P,
   DMA_DIR_P2M,
   DMA_DIR_M2P,
   DMA_DIR_M2M
};

enum {
    DMA_DATA_SIZE_8 = 0,
    DMA_DATA_SIZE_16,
    DMA_DATA_SIZE_32,
};

/**
 * @brief Request dma controller/channel
 * \param ctrl    : pointer to hold controller and channel
 * \param req     : Request peripheral
 * */
uint32_t DMA_Config(dmatype_t *dma, uint32_t req);

void DMA_Start(dmatype_t *dma);

void DMA_Cancel(dmatype_t *dma);

uint32_t DMA_GetTransfers(dmatype_t *dma);
void DMA_SetSrc(dmatype_t *dma, void *src);
void DMA_SetDst(dmatype_t *dma, void *src);

#endif /* _dma_h_ */
