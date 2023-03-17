#ifndef _dma_at32f4xx_h_
#define _dma_at32f4xx_h_

#define DMA_NUMBER_POS     4
#define DMA_CHANNEL_POS    0

#define DMA_NUMBER_MASK    (15 << DMA_NUMBER_POS)
#define DMA_CHANNEL_MASK   (15 << DMA_CHANNEL_POS)

#define DMA1_NUM_CHANNELS   6
#define DMA_NUM_CHANNELS    (DMA1_NUM_CHANNELS)

/**
 * DMA Requests
 * */
#define DMA1_REQ_ADC1                   ((0 << DMA_NUMBER_POS) | (0 << DMA_CHANNEL_POS))
#define DMA1_REQ_TIM2_CH3               ((0 << DMA_NUMBER_POS) | (0 << DMA_CHANNEL_POS))
#define DMA1_REQ_TIM4_CH1               ((0 << DMA_NUMBER_POS) | (0 << DMA_CHANNEL_POS))

#define DMA1_REQ_SPI1_RX                ((0 << DMA_NUMBER_POS) | (1 << DMA_CHANNEL_POS))
#define DMA1_REQ_USART3_TX              ((0 << DMA_NUMBER_POS) | (1 << DMA_CHANNEL_POS))
#define DMA1_REQ_TIM1_CH1               ((0 << DMA_NUMBER_POS) | (1 << DMA_CHANNEL_POS))
#define DMA1_REQ_TIM2_UP                ((0 << DMA_NUMBER_POS) | (1 << DMA_CHANNEL_POS))
#define DMA1_REQ_TIM3_CH3               ((0 << DMA_NUMBER_POS) | (1 << DMA_CHANNEL_POS))

#define DMA1_REQ_SPI1_TX                ((0 << DMA_NUMBER_POS) | (2 << DMA_CHANNEL_POS))
#define DMA1_REQ_USART3_RX              ((0 << DMA_NUMBER_POS) | (2 << DMA_CHANNEL_POS))
#define DMA1_REQ_TIM3_CH4               ((0 << DMA_NUMBER_POS) | (2 << DMA_CHANNEL_POS))
#define DMA1_REQ_TIM3_UP                DMA1_REQ_TIM3_CH4

#define DMA1_REQ_SPI2_RX                ((0 << DMA_NUMBER_POS) | (3 << DMA_CHANNEL_POS))
#define DMA1_REQ_I2S2_RX                DMA1_REQ_SPI2_RX
#define DMA1_REQ_USART1_TX              ((0 << DMA_NUMBER_POS) | (3 << DMA_CHANNEL_POS))
#define DMA1_REQ_I2C2_TX                ((0 << DMA_NUMBER_POS) | (3 << DMA_CHANNEL_POS))
#define DMA1_REQ_TIM1_CH4               ((0 << DMA_NUMBER_POS) | (3 << DMA_CHANNEL_POS))
#define DMA1_REQ_TIM1_TRIG              DMA1_REQ_TIM1_CH4
#define DMA1_REQ_TIM1_COM               DMA1_REQ_TIM1_CH4
#define DMA1_REQ_TIM4_CH2               ((0 << DMA_NUMBER_POS) | (3 << DMA_CHANNEL_POS))

#define DMA1_REQ_SPI2_TX                ((0 << DMA_NUMBER_POS) | (4 << DMA_CHANNEL_POS))
#define DMA1_REQ_I2S2_TX                DMA1_REQ_SPI2_TX
#define DMA1_REQ_USART1_RX              ((0 << DMA_NUMBER_POS) | (4 << DMA_CHANNEL_POS))
#define DMA1_REQ_I2C2_RX                ((0 << DMA_NUMBER_POS) | (4 << DMA_CHANNEL_POS))
#define DMA1_REQ_TIM1_UP                ((0 << DMA_NUMBER_POS) | (4 << DMA_CHANNEL_POS))
#define DMA1_REQ_TIM2_CH1               ((0 << DMA_NUMBER_POS) | (4 << DMA_CHANNEL_POS))
#define DMA1_REQ_TIM4_CH3               ((0 << DMA_NUMBER_POS) | (4 << DMA_CHANNEL_POS))

#define DMA1_REQ_USART2_RX              ((0 << DMA_NUMBER_POS) | (5 << DMA_CHANNEL_POS))
#define DMA1_REQ_I2C1_TX                ((0 << DMA_NUMBER_POS) | (5 << DMA_CHANNEL_POS))
#define DMA1_REQ_TIM1_CH3               ((0 << DMA_NUMBER_POS) | (5 << DMA_CHANNEL_POS))
#define DMA1_REQ_TIM3_CH1               ((0 << DMA_NUMBER_POS) | (5 << DMA_CHANNEL_POS))
#define DMA1_REQ_TRIG                   DMA1_REQ_TIM3_CH1

#define DMA1_REQ_USART2_TX              ((0 << DMA_NUMBER_POS) | (6 << DMA_CHANNEL_POS))
#define DMA1_REQ_I2C1_RX                ((0 << DMA_NUMBER_POS) | (6 << DMA_CHANNEL_POS))
#define DMA1_REQ_TIM2_CH2               ((0 << DMA_NUMBER_POS) | (6 << DMA_CHANNEL_POS))
#define DMA1_REQ_TIM2_CH4               ((0 << DMA_NUMBER_POS) | (6 << DMA_CHANNEL_POS))
#define DMA1_REQ_TIM4_UP                DMA1_REQ_TIM3_CH1

#define DMA_CCR_PSIZE_8                 0
#define DMA_CCR_PSIZE_16                1
#define DMA_CCR_PSIZE_32                2
#define DMA_CCR_MSIZE_8                 0
#define DMA_CCR_MSIZE_16                1
#define DMA_CCR_MSIZE_32                2

#define DMA_CCR_PSIZE_CFG(x)            (x << 8)
#define DMA_CCR_MSIZE_CFG(x)            (x << 10)

#endif