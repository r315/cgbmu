#ifndef _at32f4xx_h_
#define _at32f4xx_h_


// Mode bits
#define GPIO_MODE_IN       0 // Input
#define GPIO_MODE_LARGE    1 // Large output source
#define GPIO_MODE_NOR      2 // Normal output source
#define GPIO_MODE_MAX      3 // Maximum output source
#define GPIO_MODE_MASK     3
// Configuration bits
#define GPIO_IN_ANALOG      (0 << 2)
#define GPIO_IN_FLOAT       (1 << 2)
#define GPIO_IN_PD          (2 << 2)
#define GPIO_IN_PU          (3 << 2)
#define GPIO_OUT_PP         (0 << 2)
#define GPIO_OUT_OD         (1 << 2)
#define GPIO_MUX_PP         (2 << 2)
#define GPIO_MUX_OD         (3 << 2)
#define GPIO_CFG_MASK       (3 << 2)

#define GPIO_OUTPUT         (GPIO_OUT_PP | GPIO_MODE_NOR)
#define GPIO_OUTPUT_OD      (GPIO_OUT_PP | GPIO_MODE_NOR)
#define GPIO_INPUT          (GPIO_IN_FLOAT | GPIO_MODE_NOR)
#define GPIO_INPUT_PU       (GPIO_IN_PU | GPIO_MODE_NOR)
#define GPIO_INPUT_PD       (GPIO_IN_PD | GPIO_MODE_NOR)

#define GPIO_SPI_SCK        (GPIO_MUX_PP | GPIO_MODE_NOR)
#define GPIO_SPI_MISO       (GPIO_MUX_PP | GPIO_MODE_NOR)
#define GPIO_SPI_MOSI       (GPIO_MUX_PP | GPIO_MODE_NOR)
#define GPIO_SPI_CS         (GPIO_MUX_PP | GPIO_MODE_NOR)

#define GPIO_GET_PORT(name) ((name >> 4) & 15)
#define GPIO_GET_PIN(name) (name & 15)

enum {
    PA_0 = 0,
    PA_1,
    PA_2,
    PA_3,
    PA_4,
    PA_5,
    PA_6,
    PA_7,
    PA_8,
    PA_9,
    PA_10,
    PA_11,
    PA_12,
    PA_13,
    PA_14,
    PA_15,

    PB_0,
    PB_1,
    PB_2,
    PB_3,
    PB_4,
    PB_5,
    PB_6,
    PB_7,
    PB_8,
    PB_9,
    PB_10,
    PB_11,
    PB_12,
    PB_13,
    PB_14,
    PB_15,

    PC_0,
    PC_1,
    PC_2,
    PC_3,
    PC_4,
    PC_5,
    PC_6,
    PC_7,
    PC_8,
    PC_9,
    PC_10,
    PC_11,
    PC_12,
    PC_13,
    PC_14,
    PC_15,

    PD_0,
    PD_1,
    PD_2,
    PD_3,
    PD_4,
    PD_5,
    PD_6,
    PD_7,
    PD_8,
    PD_9,
    PD_10,
    PD_11,
    PD_12,
    PD_13,
    PD_14,
    PD_15,

    PE_0,
    PE_1,
    PE_2,
    PE_3,
    PE_4,
    PE_5,
    PE_6,
    PE_7,
    PE_8,
    PE_9,
    PE_10,
    PE_11,
    PE_12,
    PE_13,
    PE_14,
    PE_15,

    PF_0,
    PF_1,
    PF_2,
    PF_3,
    PF_4,
    PF_5,
    PF_6,
    PF_7,
    PF_8,
    PF_9,
    PF_10,
    PF_11,
    PF_12,
    PF_13,
    PF_14,
    PF_15,
};
#endif