#ifndef CLOCK_H
#define CLOCK_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

enum clocknr {
    CLOCK_CPU = 0,
    CLOCK_HCLK,
    CLOCK_PCLK1,
    CLOCK_PCLK2
};

typedef struct sysclock {
    uint32_t cpu;       // Cpu clock
    uint32_t hclk;      // HCLK
    uint32_t pclk1;     // Peripheral bus1
    uint32_t pclk2;     // Peripheral bus2
    uint32_t pclk3;     // Peripheral bus3/adc
}sysclock_t;

void CLOCK_GetAll(sysclock_t *clk);
uint32_t CLOCK_Get(uint8_t idx);

#ifdef __cplusplus
}
#endif

#endif // CLOCK_H