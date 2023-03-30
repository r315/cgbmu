#include "stm32f7xx_hal.h"
#include "stm32f769i_discovery.h"
#include "i2c.h"


static I2C_HandleTypeDef hexti2c;

/**
 * @brief 
 * 
 * @param i2c 
 */
void I2C_Error(i2cbus_t *i2c)
{
    /* De-initialize the I2C communication bus */
    HAL_I2C_DeInit(&hexti2c);
    I2C_Init(i2c);
}

/**
 * @brief  Reads multiple data.
 * @param  i2c_handler : I2C handler
 * @param  Addr: I2C address
 * @param  Reg: Reg address
 * @param  MemAddress: memory address
 * @param  Buffer: Pointer to data buffer
 * @param  Length: Length of the data
 * @retval HAL status
 */
uint32_t I2C_Read(i2cbus_t *i2c, uint8_t *Buffer, uint32_t Length)
{
    if(i2c == NULL)
        return 0;    

    if (HAL_I2C_Master_Receive(&hexti2c, i2c->addr, Buffer, Length, 1000) != HAL_OK)
    {
        I2C_Error(i2c);
        return 0;
    }

    return Length;
}
/**
 * @brief  Writes a value in a register of the device through BUS in using DMA mode.
 * @param  i2c_handler : I2C handler
 * @param  Addr: Device address on BUS Bus.
 * @param  Reg: The target register address to write
 * @param  MemAddress: memory address
 * @param  Buffer: The target register value to be written
 * @param  Length: buffer size to be written
 * @retval HAL status
 */
uint32_t I2C_Write(i2cbus_t *i2c, uint8_t *Buffer, uint32_t Length)
{
    if(i2c == NULL)
        return 0;

    if (HAL_I2C_Master_Transmit(&hexti2c, i2c->addr, Buffer, Length, 1000) != HAL_OK)
    {
        I2C_Error(i2c);
        return 0;
    }
    
    return Length;
}

/**
 * @brief  Initializes I2C MSP.
 * @param  i2c_handler : I2C handler
 * @retval None
 */
static void I2C_MspInit(void)
{
    GPIO_InitTypeDef gpio_init_structure;
    /*** Configure the GPIOs ***/
    /* Enable GPIO clock */
    DISCOVERY_EXT_I2Cx_SCL_SDA_GPIO_CLK_ENABLE();

    /* Configure I2C Tx as alternate function */
    gpio_init_structure.Pin = DISCOVERY_EXT_I2Cx_SCL_PIN;
    gpio_init_structure.Mode = GPIO_MODE_AF_OD;
    gpio_init_structure.Pull = GPIO_NOPULL;
    gpio_init_structure.Speed = GPIO_SPEED_FAST;
    gpio_init_structure.Alternate = DISCOVERY_EXT_I2Cx_SCL_SDA_AF;
    HAL_GPIO_Init(DISCOVERY_EXT_I2Cx_SCL_SDA_GPIO_PORT, &gpio_init_structure);

    /* Configure I2C Rx as alternate function */
    gpio_init_structure.Pin = DISCOVERY_EXT_I2Cx_SDA_PIN;
    HAL_GPIO_Init(DISCOVERY_EXT_I2Cx_SCL_SDA_GPIO_PORT, &gpio_init_structure);

    /*** Configure the I2C peripheral ***/
    /* Enable I2C clock */
    DISCOVERY_EXT_I2Cx_CLK_ENABLE();

    /* Force the I2C peripheral clock reset */
    DISCOVERY_EXT_I2Cx_FORCE_RESET();

    /* Release the I2C peripheral clock reset */
    DISCOVERY_EXT_I2Cx_RELEASE_RESET();

    /* Enable and set I2C1 Interrupt to a lower priority */
    HAL_NVIC_SetPriority(DISCOVERY_EXT_I2Cx_EV_IRQn, 0x0F, 0);
    HAL_NVIC_EnableIRQ(DISCOVERY_EXT_I2Cx_EV_IRQn);

    /* Enable and set I2C1 Interrupt to a lower priority */
    HAL_NVIC_SetPriority(DISCOVERY_EXT_I2Cx_ER_IRQn, 0x0F, 0);
    HAL_NVIC_EnableIRQ(DISCOVERY_EXT_I2Cx_ER_IRQn);
}

/**
 * @brief  Initializes I2C HAL.
 * @param  i2c_handler : I2C handler
 * @retval None
 */
void I2C_Init(i2cbus_t *i2c)
{
    i2c->ctrl = &hexti2c;

    if (HAL_I2C_GetState(&hexti2c) == HAL_I2C_STATE_RESET)
    {
        hexti2c.Instance = DISCOVERY_EXT_I2Cx;
        hexti2c.Init.Timing = DISCOVERY_I2Cx_TIMING;
        hexti2c.Init.OwnAddress1 = 0;
        hexti2c.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
        hexti2c.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
        hexti2c.Init.OwnAddress2 = 0;
        hexti2c.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
        hexti2c.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

        /* Init the I2C */
        I2C_MspInit();
        HAL_I2C_Init(&hexti2c);
    }
}