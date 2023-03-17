#include "at32f4xx.h"
#include "gpio_at32f4xx.h"

#if !defined (AT32F403xx)
#define GPIOE_BASE 0
#endif
static const uint32_t ports[] = {GPIOA_BASE, GPIOB_BASE, GPIOC_BASE, GPIOD_BASE, GPIOE_BASE, GPIOF_BASE};

/**
 * @brief 
 * 
 * @param name 
 * @param mode 
 */
void GPIO_Config(uint32_t name, uint32_t mode) {
    GPIO_Type *port = (GPIO_Type*)ports[GPIO_GET_PORT(name)];
    uint8_t pin = GPIO_GET_PIN(name);
    
    mode &= 0x0f;

    if(mode & GPIO_IN_PD){
        if(mode & GPIO_IN_PU){
            port->BSRE = (1 << pin);
            mode &= ~GPIO_IN_FLOAT; // clear reserved bit
        }else{
            port->BRE = (1 << pin);
        }
    }

    if(pin <  8){ 
        port->CTRLL = (port->CTRLL & ~(15 << (pin << 2))) | (mode << (pin << 2));
    }else{ 
        port->CTRLH = (port->CTRLH & ~(15 << ((pin - 8) << 2))) | (mode << ((pin - 8) << 2)); 
    }
}

uint32_t GPIO_Read(uint32_t name){
    GPIO_Type *port = (GPIO_Type*)ports[GPIO_GET_PORT(name)];
    uint8_t pin = GPIO_GET_PIN(name);

    return !!(port->IPTDT & (1 << pin));
}

void GPIO_Write(uint32_t name, uint32_t state){
    GPIO_Type *port = (GPIO_Type*)ports[GPIO_GET_PORT(name)];
    uint8_t pin = GPIO_GET_PIN(name);

    uint32_t mask = (1 << pin);

    if(state & 1){ 
        port->BSRE = mask;
    }else{ 
        port->BRE = mask;
    }  
}

void GPIO_Toggle(uint32_t name){
    GPIO_Type *port = (GPIO_Type*)ports[GPIO_GET_PORT(name)];
    uint8_t pin = GPIO_GET_PIN(name);

    port->OPTDT ^= (1 << pin);
}
