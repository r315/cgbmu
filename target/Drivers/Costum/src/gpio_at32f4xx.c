#include <stdlib.h>
#include "at32f4xx.h"
#include "gpio_at32f4xx.h"

#if !defined (AT32F403xx)
#define GPIOE_BASE 0
#endif

static const uint32_t ports[] = {GPIOA_BASE, GPIOB_BASE, GPIOC_BASE, GPIOD_BASE, GPIOE_BASE, GPIOF_BASE};

static GPIO_Type *getPort(pinName_e name)
{
    uint8_t pidx = GPIO_NAME_TO_PORT(name);
    if(pidx > sizeof(ports)/sizeof(ports[0])){
        return NULL;
    }

    return (GPIO_Type*)ports[pidx];
}

/**
 * @brief
 *
 * @param name
 * @param mode
 */
void GPIO_Config(pinName_e name, uint32_t cfg) {
    GPIO_Type *port;
    uint8_t pin = GPIO_NAME_TO_PIN(name);

    if ((port = getPort(name)) == NULL) {
        return;
    }

    cfg = GPIO_CFG_MASK(cfg);

    if(cfg == GPI_PD){
        port->BRE = (1 << pin);  // Activate pull-down
    }else if(cfg == GPI_PU){
        port->BSRE = (1 << pin); // Activate pull-up
        cfg &= ~GPIO_IOF_FLT;    // clear reserved bit
    }

    if(pin <  8){
        port->CTRLL = (port->CTRLL & ~(15 << (pin << 2))) | (cfg << (pin << 2));
    }else{
        port->CTRLH = (port->CTRLH & ~(15 << ((pin - 8) << 2))) | (cfg << ((pin - 8) << 2));
    }
}

/**
 * @brief
 *
 * @param name
 * @return uint32_t
 */
uint32_t GPIO_Read(pinName_e name){
    GPIO_Type *port;
    uint8_t pin = GPIO_NAME_TO_PIN(name);

    if ((port = getPort(name)) == NULL) {
        return 0xFFFFFFFF;
    }

    return !!(port->IPTDT & (1 << pin));
}

/**
 * @brief
 *
 * @param name
 * @param state
 */
void GPIO_Write(pinName_e name, uint32_t state){
    GPIO_Type *port;
    uint8_t pin = GPIO_NAME_TO_PIN(name);

    if ((port = getPort(name)) == NULL) {
        return;
    }

    port->BSRE = (state != 0)? (1 << pin) : (0x10000 << pin);
}

/**
 * @brief
 *
 * @param name
 */
void GPIO_Toggle(pinName_e name){
    GPIO_Type *port;
    uint8_t pin = GPIO_NAME_TO_PIN(name);

    if ((port = getPort(name)) == NULL) {
        return;
    }

    port->OPTDT ^= (1 << pin);
}
