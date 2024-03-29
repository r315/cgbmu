

#ifndef _board_h_
#define _board_h_

#ifdef __cplusplus
extern "C" {
#endif
#include "stm32f7xx_hal.h"
#include "stm32f769i_discovery_lcd.h"
#include "stm32f769i_discovery_sdram.h"

#define DBG_PIN_TOGGLE

enum {false = 0, true, OFF = false, ON = true};

#define SCREEN_OFFSET_X     0
#define SCREEN_OFFSET_Y     0

#define DMA2D_CR_M2M (0 << 16)
#define DMA2D_CR_M2M_PFC (1 << 16)
#define DMA2D_CR_M2M_BLEND (2 << 16)
#define DMA2D_CR_R2M (3 << 16)
#define DMA2D_FGPFCCR_SET_ALPHA(a) ((a << 24) | (1 << 16))
#define DMA2D_FGPFCCR_SET_CS(cs) ((cs) << 8)	// CLUT size
#define DMA2D_FGPFCCR_SET_CM(cm) ((cm) << 0)  // Input Color mode
#define DMA2D_OPFCCR_SET_CM(cm) ((cm) << 0)	// Output Color mode
#define DMA2D_NLR_PLNL(pl, nl) (((pl) << 16) | nl)

#define BUTTON_DOWN 	(1<<0)
#define BUTTON_UP	    (1<<1)
#define BUTTON_RIGHT 	(1<<2)
#define BUTTON_LEFT  	(1<<3)
#define BUTTON_CENTER   (1<<4)
#define BUTTON_A    	(1<<5)
#define BUTTON_B    	(1<<6)
#define BUTTON_C    	(1<<7)

#define LIB2D_Char __DISPLAY_Char
static inline void LIB2D_SetFcolor(uint16_t color){
    uint8_t r = color >> 11;
    uint8_t g = color >> 5;
    uint8_t b = color >> 0;
    BSP_LCD_SetTextColor(0xFF000000 | (r << 16) | (g << 8) | b);
}

static inline uint16_t LIB2D_Text(uint16_t x, uint16_t y, const char *text) {
    return BSP_LCD_DisplayStringAt(x * 2, y * 2, (uint8_t*)text, LEFT_MODE);
}

static inline uint16_t __DISPLAY_Char(uint16_t x, uint16_t y, uint8_t c){
    BSP_LCD_DisplayChar(x, y * 2, c);
    return x + 14;
}

void BOARD_Init(void);
void LCD_Window(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
void LCD_Data(uint16_t data);
void Error_Handler(void);
uint8_t vc_getCharNonBlocking(char *c);
void __debugbreak(void);


#define SAI1_FSA_Pin GPIO_PIN_4
#define SAI1_FSA_GPIO_Port GPIOE
#define SAI1_SDB_Pin GPIO_PIN_3
#define SAI1_SDB_GPIO_Port GPIOE
#define QSPI_D2_Pin GPIO_PIN_2
#define QSPI_D2_GPIO_Port GPIOE
#define RMII_TXD1_Pin GPIO_PIN_14
#define RMII_TXD1_GPIO_Port GPIOG
#define FMC_NBL1_Pin GPIO_PIN_1
#define FMC_NBL1_GPIO_Port GPIOE
#define FMC_NBL0_Pin GPIO_PIN_0
#define FMC_NBL0_GPIO_Port GPIOE
#define ARDUINO_SCL_D15_Pin GPIO_PIN_8
#define ARDUINO_SCL_D15_GPIO_Port GPIOB
#define ULPI_D7_Pin GPIO_PIN_5
#define ULPI_D7_GPIO_Port GPIOB
#define uSD_D3_Pin GPIO_PIN_4
#define uSD_D3_GPIO_Port GPIOB
#define uSD_D2_Pin GPIO_PIN_3
#define uSD_D2_GPIO_Port GPIOB
#define uSD_CMD_Pin GPIO_PIN_7
#define uSD_CMD_GPIO_Port GPIOD
#define WIFI_RX_Pin GPIO_PIN_12
#define WIFI_RX_GPIO_Port GPIOC
#define SWCLK_Pin GPIO_PIN_14
#define SWCLK_GPIO_Port GPIOA
#define SWDIO_Pin GPIO_PIN_13
#define SWDIO_GPIO_Port GPIOA
#define SAI1_SCKA_Pin GPIO_PIN_5
#define SAI1_SCKA_GPIO_Port GPIOE
#define SAI1_SDA_Pin GPIO_PIN_6
#define SAI1_SDA_GPIO_Port GPIOE
#define RMII_TXD0_Pin GPIO_PIN_13
#define RMII_TXD0_GPIO_Port GPIOG
#define ARDUINO_SDA_D14_Pin GPIO_PIN_9
#define ARDUINO_SDA_D14_GPIO_Port GPIOB
#define AUDIO_SDA_Pin GPIO_PIN_7
#define AUDIO_SDA_GPIO_Port GPIOB
#define QSPI_NCS_Pin GPIO_PIN_6
#define QSPI_NCS_GPIO_Port GPIOB
#define FMC_SDNCAS_Pin GPIO_PIN_15
#define FMC_SDNCAS_GPIO_Port GPIOG
#define RMII_TX_EN_Pin GPIO_PIN_11
#define RMII_TX_EN_GPIO_Port GPIOG
#define LD1_Pin GPIO_PIN_13
#define LD1_GPIO_Port GPIOJ
#define Audio_INT_Pin GPIO_PIN_12
#define Audio_INT_GPIO_Port GPIOJ
#define uSD_CLK_Pin GPIO_PIN_6
#define uSD_CLK_GPIO_Port GPIOD
#define FMC_D2_Pin GPIO_PIN_0
#define FMC_D2_GPIO_Port GPIOD
#define DFSDM_DATIN5_Pin GPIO_PIN_11
#define DFSDM_DATIN5_GPIO_Port GPIOC
#define QSPI_D1_Pin GPIO_PIN_10
#define QSPI_D1_GPIO_Port GPIOC
#define ARD_D13_SCK_Pin GPIO_PIN_12
#define ARD_D13_SCK_GPIO_Port GPIOA
#define NC4_Pin GPIO_PIN_8
#define NC4_GPIO_Port GPIOI
#define FMC_NBL2_Pin GPIO_PIN_4
#define FMC_NBL2_GPIO_Port GPIOI
#define NC3_Pin GPIO_PIN_7
#define NC3_GPIO_Port GPIOK
#define NC2_Pin GPIO_PIN_6
#define NC2_GPIO_Port GPIOK
#define NC1_Pin GPIO_PIN_5
#define NC1_GPIO_Port GPIOK
#define SPDIF_RX_Pin GPIO_PIN_12
#define SPDIF_RX_GPIO_Port GPIOG
#define uSD_D1_Pin GPIO_PIN_10
#define uSD_D1_GPIO_Port GPIOG
#define WIFI_RST_Pin GPIO_PIN_14
#define WIFI_RST_GPIO_Port GPIOJ
#define RMII_RXER_Pin GPIO_PIN_5
#define RMII_RXER_GPIO_Port GPIOD
#define DFSDM_CKOUT_Pin GPIO_PIN_3
#define DFSDM_CKOUT_GPIO_Port GPIOD
#define FMC_D3_Pin GPIO_PIN_1
#define FMC_D3_GPIO_Port GPIOD
#define D27_Pin GPIO_PIN_3
#define D27_GPIO_Port GPIOI
#define D26_Pin GPIO_PIN_2
#define D26_GPIO_Port GPIOI
#define SPI2_NSS_Pin GPIO_PIN_11
#define SPI2_NSS_GPIO_Port GPIOA
#define FMC_A0_Pin GPIO_PIN_0
#define FMC_A0_GPIO_Port GPIOF
#define FMC_NBL3_Pin GPIO_PIN_5
#define FMC_NBL3_GPIO_Port GPIOI
#define D29_Pin GPIO_PIN_7
#define D29_GPIO_Port GPIOI
#define D31_Pin GPIO_PIN_10
#define D31_GPIO_Port GPIOI
#define D28_Pin GPIO_PIN_6
#define D28_GPIO_Port GPIOI
#define NC8_Pin GPIO_PIN_4
#define NC8_GPIO_Port GPIOK
#define NC7_Pin GPIO_PIN_3
#define NC7_GPIO_Port GPIOK
#define uSD_D0_Pin GPIO_PIN_9
#define uSD_D0_GPIO_Port GPIOG
#define OTG_FS_OverCurrent_Pin GPIO_PIN_4
#define OTG_FS_OverCurrent_GPIO_Port GPIOD
#define WIFI_TX_Pin GPIO_PIN_2
#define WIFI_TX_GPIO_Port GPIOD
#define D23_Pin GPIO_PIN_15
#define D23_GPIO_Port GPIOH
#define D25_Pin GPIO_PIN_1
#define D25_GPIO_Port GPIOI
#define VCP_RX_Pin GPIO_PIN_10
#define VCP_RX_GPIO_Port GPIOA
#define RCC_OSC32_IN_Pin GPIO_PIN_14
#define RCC_OSC32_IN_GPIO_Port GPIOC
#define FMC_A1_Pin GPIO_PIN_1
#define FMC_A1_GPIO_Port GPIOF
#define NC5_Pin GPIO_PIN_12
#define NC5_GPIO_Port GPIOI
#define D30_Pin GPIO_PIN_9
#define D30_GPIO_Port GPIOI
#define D21_Pin GPIO_PIN_13
#define D21_GPIO_Port GPIOH
#define D22_Pin GPIO_PIN_14
#define D22_GPIO_Port GPIOH
#define D24_Pin GPIO_PIN_0
#define D24_GPIO_Port GPIOI
#define VCP_TX_Pin GPIO_PIN_9
#define VCP_TX_GPIO_Port GPIOA
#define RCC_OSC32_OUT_Pin GPIO_PIN_15
#define RCC_OSC32_OUT_GPIO_Port GPIOC
#define ULPI_DIR_Pin GPIO_PIN_11
#define ULPI_DIR_GPIO_Port GPIOI
#define QSPI_D0_Pin GPIO_PIN_9
#define QSPI_D0_GPIO_Port GPIOC
#define OSC_25M_Pin GPIO_PIN_0
#define OSC_25M_GPIO_Port GPIOH
#define FMC_A2_Pin GPIO_PIN_2
#define FMC_A2_GPIO_Port GPIOF
#define uSD_Detect_Pin GPIO_PIN_15
#define uSD_Detect_GPIO_Port GPIOI
#define ARD_D5_PWM_Pin GPIO_PIN_8
#define ARD_D5_PWM_GPIO_Port GPIOC
#define ARD_D0_RX_Pin GPIO_PIN_7
#define ARD_D0_RX_GPIO_Port GPIOC
#define FMC_A3_Pin GPIO_PIN_3
#define FMC_A3_GPIO_Port GPIOF
#define ULPI_NXT_Pin GPIO_PIN_4
#define ULPI_NXT_GPIO_Port GPIOH
#define FMC_SDCLK_Pin GPIO_PIN_8
#define FMC_SDCLK_GPIO_Port GPIOG
#define ARDUINO_TX_D1_Pin GPIO_PIN_6
#define ARDUINO_TX_D1_GPIO_Port GPIOC
#define FMC_A4_Pin GPIO_PIN_4
#define FMC_A4_GPIO_Port GPIOF
#define FMC_SDNME_Pin GPIO_PIN_5
#define FMC_SDNME_GPIO_Port GPIOH
#define FMC_SDNE0_Pin GPIO_PIN_3
#define FMC_SDNE0_GPIO_Port GPIOH
#define SAI1_MCLKA_Pin GPIO_PIN_7
#define SAI1_MCLKA_GPIO_Port GPIOG
#define EXT_SDA_Pin GPIO_PIN_6
#define EXT_SDA_GPIO_Port GPIOG
#define ARD_D6_PWM_Pin GPIO_PIN_7
#define ARD_D6_PWM_GPIO_Port GPIOF
#define ARD_D3_PWM_Pin GPIO_PIN_6
#define ARD_D3_PWM_GPIO_Port GPIOF
#define FMC_A5_Pin GPIO_PIN_5
#define FMC_A5_GPIO_Port GPIOF
#define FMC_SDCKE0_Pin GPIO_PIN_2
#define FMC_SDCKE0_GPIO_Port GPIOH
#define FMC_D1_Pin GPIO_PIN_15
#define FMC_D1_GPIO_Port GPIOD
#define ULPI_D6_Pin GPIO_PIN_13
#define ULPI_D6_GPIO_Port GPIOB
#define FMC_D15_Pin GPIO_PIN_10
#define FMC_D15_GPIO_Port GPIOD
#define ARDUINO_A1_Pin GPIO_PIN_10
#define ARDUINO_A1_GPIO_Port GPIOF
#define ARDUINO_A2_Pin GPIO_PIN_9
#define ARDUINO_A2_GPIO_Port GPIOF
#define ARDUINO_A3_Pin GPIO_PIN_8
#define ARDUINO_A3_GPIO_Port GPIOF
#define DFSDM_DATIN1_Pin GPIO_PIN_3
#define DFSDM_DATIN1_GPIO_Port GPIOC
#define FMC_D0_Pin GPIO_PIN_14
#define FMC_D0_GPIO_Port GPIOD
#define ULPI_D5_Pin GPIO_PIN_12
#define ULPI_D5_GPIO_Port GPIOB
#define FMC_D14_Pin GPIO_PIN_9
#define FMC_D14_GPIO_Port GPIOD
#define FMC_D13_Pin GPIO_PIN_8
#define FMC_D13_GPIO_Port GPIOD
#define ULPI_STP_Pin GPIO_PIN_0
#define ULPI_STP_GPIO_Port GPIOC
#define RMII_MDC_Pin GPIO_PIN_1
#define RMII_MDC_GPIO_Port GPIOC
#define ARD_A2_Pin GPIO_PIN_2
#define ARD_A2_GPIO_Port GPIOC
#define QSPI_CLK_Pin GPIO_PIN_2
#define QSPI_CLK_GPIO_Port GPIOB
#define FMC_A6_Pin GPIO_PIN_12
#define FMC_A6_GPIO_Port GPIOF
#define FMC_A11_Pin GPIO_PIN_1
#define FMC_A11_GPIO_Port GPIOG
#define FMC_A9_Pin GPIO_PIN_15
#define FMC_A9_GPIO_Port GPIOF
#define ARD_D8_Pin GPIO_PIN_4
#define ARD_D8_GPIO_Port GPIOJ
#define AUDIO_SCL_Pin GPIO_PIN_12
#define AUDIO_SCL_GPIO_Port GPIOD
#define QSPI_D3_Pin GPIO_PIN_13
#define QSPI_D3_GPIO_Port GPIOD
#define EXT_SCL_Pin GPIO_PIN_3
#define EXT_SCL_GPIO_Port GPIOG
#define FMC_A12_Pin GPIO_PIN_2
#define FMC_A12_GPIO_Port GPIOG
#define LD2_Pin GPIO_PIN_5
#define LD2_GPIO_Port GPIOJ
#define D20_Pin GPIO_PIN_12
#define D20_GPIO_Port GPIOH
#define RMII_REF_CLK_Pin GPIO_PIN_1
#define RMII_REF_CLK_GPIO_Port GPIOA
#define B_USER_Pin GPIO_PIN_0
#define B_USER_GPIO_Port GPIOA
#define ARD_A1_Pin GPIO_PIN_4
#define ARD_A1_GPIO_Port GPIOA
#define RMII_RXD0_Pin GPIO_PIN_4
#define RMII_RXD0_GPIO_Port GPIOC
#define FMC_A7_Pin GPIO_PIN_13
#define FMC_A7_GPIO_Port GPIOF
#define FMC_A10_Pin GPIO_PIN_0
#define FMC_A10_GPIO_Port GPIOG
#define ARD_D7_Pin GPIO_PIN_3
#define ARD_D7_GPIO_Port GPIOJ
#define FMC_D5_Pin GPIO_PIN_8
#define FMC_D5_GPIO_Port GPIOE
#define SPDIF_TX_Pin GPIO_PIN_11
#define SPDIF_TX_GPIO_Port GPIOD
#define FMC_BA1_Pin GPIO_PIN_5
#define FMC_BA1_GPIO_Port GPIOG
#define FMC_BA0_Pin GPIO_PIN_4
#define FMC_BA0_GPIO_Port GPIOG
#define EXT_RST_Pin GPIO_PIN_7
#define EXT_RST_GPIO_Port GPIOH
#define FMC_D_7_Pin GPIO_PIN_9
#define FMC_D_7_GPIO_Port GPIOH
#define FMC_D19_Pin GPIO_PIN_11
#define FMC_D19_GPIO_Port GPIOH
#define RMII_MDIO_Pin GPIO_PIN_2
#define RMII_MDIO_GPIO_Port GPIOA
#define ARD_A0_Pin GPIO_PIN_6
#define ARD_A0_GPIO_Port GPIOA
#define ULPI_CLK_Pin GPIO_PIN_5
#define ULPI_CLK_GPIO_Port GPIOA
#define RMII_RXD1_Pin GPIO_PIN_5
#define RMII_RXD1_GPIO_Port GPIOC
#define FMC_A8_Pin GPIO_PIN_14
#define FMC_A8_GPIO_Port GPIOF
#define FMC_SDNRAS_Pin GPIO_PIN_11
#define FMC_SDNRAS_GPIO_Port GPIOF
#define FMC_D6_Pin GPIO_PIN_9
#define FMC_D6_GPIO_Port GPIOE
#define FMC_D8_Pin GPIO_PIN_11
#define FMC_D8_GPIO_Port GPIOE
#define FMC_D11_Pin GPIO_PIN_14
#define FMC_D11_GPIO_Port GPIOE
#define ULPI_D3_Pin GPIO_PIN_10
#define ULPI_D3_GPIO_Port GPIOB
#define ARDUINO_PWM_D6_Pin GPIO_PIN_6
#define ARDUINO_PWM_D6_GPIO_Port GPIOH
#define FMC_D16_Pin GPIO_PIN_8
#define FMC_D16_GPIO_Port GPIOH
#define FMC_D18_Pin GPIO_PIN_10
#define FMC_D18_GPIO_Port GPIOH
#define ULPI_D0_Pin GPIO_PIN_3
#define ULPI_D0_GPIO_Port GPIOA
#define RMII_CRS_DV_Pin GPIO_PIN_7
#define RMII_CRS_DV_GPIO_Port GPIOA
#define ULPI_D2_Pin GPIO_PIN_1
#define ULPI_D2_GPIO_Port GPIOB
#define ULPI_D1_Pin GPIO_PIN_0
#define ULPI_D1_GPIO_Port GPIOB
#define ARD_D4_Pin GPIO_PIN_0
#define ARD_D4_GPIO_Port GPIOJ
#define ARD_D2_Pin GPIO_PIN_1
#define ARD_D2_GPIO_Port GPIOJ
#define FMC_D4_Pin GPIO_PIN_7
#define FMC_D4_GPIO_Port GPIOE
#define FMC_D7_Pin GPIO_PIN_10
#define FMC_D7_GPIO_Port GPIOE
#define FMC_D9_Pin GPIO_PIN_12
#define FMC_D9_GPIO_Port GPIOE
#define FMC_D12_Pin GPIO_PIN_15
#define FMC_D12_GPIO_Port GPIOE
#define FMC_D10_Pin GPIO_PIN_13
#define FMC_D10_GPIO_Port GPIOE
#define ULPI_D4_Pin GPIO_PIN_11
#define ULPI_D4_GPIO_Port GPIOB
#define ARDUINO_MISO_D12_Pin GPIO_PIN_14
#define ARDUINO_MISO_D12_GPIO_Port GPIOB
#define ARDUINO_MOSI_PWM_D11_Pin GPIO_PIN_15
#define ARDUINO_MOSI_PWM_D11_GPIO_Port GPIOB

#ifdef __cplusplus
}
#endif

#endif
