/**
* @file		i2c.h
* @brief	Contains the i2c API header.
*     		
* @version	1.0
* @date		6 Apr. 2017
* @author	Hugo Reis
**********************************************************************/

#ifndef _I2C_H_
#define _I2C_H_

#include <stdint.h>

typedef struct {
    void *ctrl;
    uint8_t addr;           // Slave address
    uint8_t bus_num;        // Bus number
}i2cbus_t;


void I2C_Init(i2cbus_t *i2c);
uint32_t I2C_Write(i2cbus_t *i2c, uint8_t *data, uint32_t size);
uint32_t I2C_Read(i2cbus_t *i2c, uint8_t *data, uint32_t size);
#endif
