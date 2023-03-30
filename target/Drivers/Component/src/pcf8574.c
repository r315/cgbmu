
#include <stddef.h>
#include "pcf8574.h"
#include "i2c.h"

static i2cbus_t *i2cbus;

input_drv_t io_drv_pcf8574 = {
    pcf8574_Init,
    pcf8574_Read,
    pcf8574_Write,
};

void pcf8574_Init(void *param){
    if(param == NULL){
        return;
    }

    i2cbus = (i2cbus_t*)param;
    i2cbus->addr = PCF8574_I2C_ADDRESS;

    // I/Os should be high before being used as inputs.
    uint8_t data = 0xFF;    
    if(I2C_Write(i2cbus, &data, 1) == 0){
        i2cbus->addr = 0;
        i2cbus = NULL;
    }
}

uint16_t pcf8574_Read(uint8_t *dst, uint16_t size){
    return I2C_Read(i2cbus, dst, size) == 0;
}

uint16_t pcf8574_Write(uint8_t *data, uint16_t size){
    return I2C_Write(i2cbus, data, size) == 0;
}