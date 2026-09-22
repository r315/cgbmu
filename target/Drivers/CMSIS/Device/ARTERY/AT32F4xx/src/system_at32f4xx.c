#include "at32f4xx.h"

#define VECT_TAB_OFFSET 0x0

#ifndef USE_STDPERIPH_DRIVER
#define PLL_CFGEN_ENABLE                ((uint32_t)0x80000000)
#define PLL_CFGEN_MASK                  ((uint32_t)0x80000000)
#define RCC_CFG_PLLMULT_LB_MASK         ((uint32_t)0x003C0000)
#define PLL_FREF_MASK                   ((uint32_t)0x07000000)
#define RCC_GET_PLLMULT(MULT)           ((((MULT & RCC_CFG_PLLMULT_LB_MASK) >> RCC_CFG_PLLMULT_LB_POS) | \
                                        ((MULT & RCC_CFG_PLLMULT_HB_MASK) >> (RCC_CFG_PLLMULT_HB_POS - RCC_CFG_PLLMULT_HB_OFFSET))) +\
                                        ((((MULT & RCC_CFG_PLLMULT_HB_MASK)==0) && \
                                        ((MULT & RCC_CFG_PLLMULT_LB_MASK)!=RCC_CFG_PLLMULT_LB_MASK) )? 2 : 1 ))

#define RCC_APB1PERIPH_PWR              ((uint32_t)0x10000000)
#define RCC_AUTO_STEP_EN                ((uint32_t)0x00000030)
#define PLL_MS_POS                      4
#define PLL_NS_POS                      8
#define PLL_FR_POS                      0
#define PLL_MS_MASK                     ((uint32_t)0x000000F0)
#define PLL_NS_MASK                     ((uint32_t)0x0001FF00)
#define PLL_FR_MASK                     ((uint32_t)0x00000007)
#endif

uint32_t SystemCoreClock;
static uint32_t fpll;
static const uint8_t AHBPscTable[16] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 6, 7, 8, 9};

/**
 * @brief
 * @param
 * @return
 */
uint32_t SystemCoreClockUpdate(void)
{
    uint32_t tmp = 0, pllmult = 0, pllrefclk = 0, tempcfg = 0;

#if defined(AT32F415xx) || defined(AT32F421xx)
    uint32_t pllsrcfreq = 0, pllns = 0, pllms = 0, pllfr = 0;
#endif
#if defined(AT32F403Axx) || defined(AT32F407xx)
    uint32_t prediv = 0;
#endif
    /* Get SYSCLK source -------------------------------------------------------*/
    tmp = RCC->CFG & RCC_CFG_SYSCLKSTS;

    switch (tmp)
    {
    case RCC_CFG_SYSCLKSTS_HSI: /* HSI used as system clock */
        SystemCoreClock = HSI_VALUE;
        break;

    case RCC_CFG_SYSCLKSTS_HSE: /* HSE used as system clock */
        SystemCoreClock = HSE_VALUE;
        break;

    case RCC_CFG_SYSCLKSTS_PLL: /* PLL used as system clock */
        /* Get PLL clock source and multiplication factor ----------------------*/
        tempcfg = RCC->CFG;
        pllrefclk = tempcfg & RCC_CFG_PLLRC;
        pllmult = RCC_GET_PLLMULT(tempcfg);

#if defined(AT32F415xx) || defined(AT32F421xx)
        if (BIT_READ(RCC->PLL, PLL_CFGEN_MASK) == PLL_CFGEN_ENABLE)
        {
            /* PLL is using flexible configuration */
            pllns = BIT_READ(RCC->PLL, PLL_NS_MASK) >> PLL_NS_POS; // multiplier
            pllms = BIT_READ(RCC->PLL, PLL_MS_MASK) >> PLL_MS_POS; // pre-division
            pllfr = (1 << BIT_READ(RCC->PLL, PLL_FR_MASK));        // post-division

            if (pllrefclk == 0x00)
            {
                /* HSI oscillator clock divided by 2 selected as PLL clock entry */
                pllsrcfreq = (HSI_VALUE >> 1);
            }
            else
            {
                /* HSE selected as PLL clock entry */
                if ((RCC->CFG & RCC_CFG_PLLHSEPSC) != (uint32_t)RESET)
                {
                    pllsrcfreq = (HSE_VALUE >> 1);
                }
                else
                {
                    pllsrcfreq = HSE_VALUE;
                }
            }

            SystemCoreClock = (pllsrcfreq * pllns) / (pllms * pllfr);
        }
        else
#endif
        {
            /* Get PLL clock source and multiplication factor ----------------------*/
            pllmult = BIT_READ(RCC->CFG, RCC_CFG_PLLMULT);
            pllmult = RCC_GET_PLLMULT(pllmult);

            if (pllrefclk == 0x00)
            {
                /* HSI oscillator clock divided by 2 selected as PLL clock entry */
                SystemCoreClock = (HSI_VALUE >> 1) * pllmult;
            }
            else
            {
                /* HSE selected as PLL clock entry */
                if ((RCC->CFG & RCC_CFG_PLLHSEPSC) != (uint32_t)RESET)
                {
#if defined(AT32F403Axx) || defined(AT32F407xx)
                    prediv = (RCC->MISC2 & RCC_HSE_DIV_MASK);
                    prediv = prediv >> RCC_HSE_DIV_POS;
                    /* HSE oscillator clock divided by prediv */
                    SystemCoreClock = (HSE_VALUE / (prediv + 2)) * pllmult;
#else
                    /* HSE oscillator clock divided by 2 */
                    SystemCoreClock = (HSE_VALUE >> 1) * pllmult;
#endif
                }
                else
                {
                    SystemCoreClock = HSE_VALUE * pllmult;
                }
            }
        }

        break;

    default:
        SystemCoreClock = HSI_VALUE;
        break;
    }

    /* Compute HCLK clock frequency ----------------*/
    /* Get HCLK prescaler */
    tmp = AHBPscTable[((RCC->CFG & RCC_CFG_AHBPSC) >> 4)];
    /* HCLK clock frequency */
    SystemCoreClock >>= tmp;

    return SystemCoreClock;
}

/**
 * @brief Default system clock configuration, 150MHz
 *        from HICK
 * @param
 */
void SystemInit(void)
{
#if defined(__FPU_USED) && (__FPU_USED == 1U)
    SCB->CPACR |= ((3UL << 10 * 2) | (3UL << 11 * 2)); /* set CP10 and CP11 Full Access */
#endif
    RCC->APB1EN |= RCC_APB1PERIPH_PWR;
    /* Enable low power mode, 0x40007050[bit2] */
    *(volatile uint8_t *)(0x40007050) |= (uint8_t)(0x1 << 2);
    RCC->APB1EN &= ~RCC_APB1PERIPH_PWR;

    /* Reset the RCC clock configuration to the default reset state(for debug purpose) */

    /* Set HSIEN to enable internal 8MHz oscillator */
    BIT_SET(RCC->CTRL, RCC_CTRL_HSIEN);

    /* Reset SW, AHBPSC, APB1PSC, APB2PSC, ADCPSC and CLKOUT bits */
    BIT_CLEAR(RCC->CFG, RCC_CFG_SYSCLKSEL | RCC_CFG_AHBPSC |
                        RCC_CFG_APB1PSC | RCC_CFG_APB2PSC |
                        RCC_CFG_ADCPSC | RCC_CFG_CLKOUT);

    /* Reset HSEEN, HSECFDEN and PLLEN bits */
    BIT_CLEAR(RCC->CTRL, RCC_CTRL_HSEEN | RCC_CTRL_HSECFDEN |
                         RCC_CTRL_PLLEN);

    /* Reset HSEBYPS bit */
    BIT_CLEAR(RCC->CTRL, RCC_CTRL_HSEBYPS);

    /* Reset PLLRC, PLLHSEPSC, PLLMUL, USBPSC and PLLRANGE bits */
    BIT_CLEAR(RCC->CFG, RCC_CFG_PLLRC | RCC_CFG_PLLHSEPSC |
                        RCC_CFG_PLLMULT | RCC_CFG_USBPSC | RCC_CFG_PLLRANGE);

    /* Reset USB768B, CLKOUT[3], HSICAL_KEY[7:0] */
    BIT_CLEAR(RCC->MISC, 0x010100FF);

    /* Disable all interrupts and clear pending bits  */
    RCC->CLKINT = RCC_CLKINT_LSISTBLFC | RCC_CLKINT_LSESTBLFC |
                  RCC_CLKINT_HSISTBLFC | RCC_CLKINT_HSESTBLFC |
                  RCC_CLKINT_PLLSTBLFC | RCC_CLKINT_HSECFDFC;

/* -------- Configure the System clock frequency, HCLK, PCLK2 and PCLK1 prescalers --------*/
    // Hang if HSI is not stable
    while ((RCC->CTRL & RCC_CTRL_HSISTBL) == 0);

    #if defined(AT32F415xx)
    /* Enable Prefetch Buffer */
    FLASH->ACR |= FLASH_ACR_PRFTBE;

    /* Flash 4 wait cycles */
    FLASH->ACR &= (uint32_t)((uint32_t)~FLASH_ACR_LATENCY);
    FLASH->ACR |= (uint32_t)FLASH_ACR_LATENCY_4;
    #endif

    /* HCLK = SYSCLK */
    RCC->CFG |= (uint32_t)RCC_CFG_AHBPSC_DIV1;
    /* PCLK2 = HCLK/2 */
    RCC->CFG &= 0xFFFFC7FF;
    RCC->CFG |= (uint32_t)RCC_CFG_APB2PSC_DIV2;
    /* PCLK1 = HCLK/2 */
    RCC->CFG &= 0xFFFFF8FF;
    RCC->CFG |= (uint32_t)RCC_CFG_APB1PSC_DIV2;
    /* PLL source = HSI/2 */
    RCC->CFG &= RCC_CFG_PLLCFG_MASK;
    RCC->CFG |= (uint32_t)(RCC_CFG_PLLRC_HSI_DIV2);
    /* PLL configuration: PLLCLK = ((HSI/2) * 150) / (1 * 4) = 150 MHz */
    uint32_t pll_ns = 150;
    uint32_t pll_ms = 1;
    uint32_t pll_fr = 2;
    uint32_t pll_reg = RCC->PLL;
    /* Clear any configuration */
    pll_reg &= ~(PLL_FR_MASK | PLL_MS_MASK | PLL_NS_MASK | PLL_FREF_MASK | PLL_CFGEN_MASK);
    /* set new configuration */
    pll_reg |= ((pll_ns << PLL_NS_POS) | (pll_ms << PLL_MS_POS) | pll_fr);
    /* Use flexible pll configuration */
    pll_reg |= PLL_CFGEN_ENABLE;
    /* Apply configuration */
    RCC->PLL = pll_reg;
    /* Enable PLL */
    RCC->CTRL |= RCC_CTRL_PLLEN;
    /* Wait till PLL is ready */
    while ((RCC->CTRL & RCC_CTRL_PLLSTBL) == 0)
    {
    }

    fpll = 150000000UL;

    /* Select PLL as system clock source */
    SystemConfigClockSrc(RCC_CFG_SYSCLKSEL_PLL);

    SystemCoreClockUpdate();

#ifdef VECT_TAB_SRAM
    SCB->VTOR = SRAM_BASE | VECT_TAB_OFFSET; /* Vector Table Relocation in Internal SRAM. */
#else
    SCB->VTOR = FLASH_BASE | VECT_TAB_OFFSET; /* Vector Table Relocation in Internal FLASH. */
#endif
}

/**
 * @brief           PLLCLK = fosc / ms * ns / fr
 * @param fin       pll source frequency in Hz
 * @param fout      desired pll output frequency in Hz
 * @return          Configured frequency in Hz
 */
static uint32_t configurePll(uint32_t fin, uint32_t fout)
{
    uint32_t pll_cfg, div, fref;

    fin /= 1000;
    div = fout / fin;
    fout /= 1000;

    if((div % 1000) == 0){
        // Integer multiplication.
        // This seams to fail above 204MHz with 12MHz HSE
        pll_cfg = RCC->CFG & ~(RCC_CFG_PLLHSEPSC | RCC_CFG_PLLMULT);
        div /= 1000;

        if(div > 128){
            // Not possible
            return 0;
        }

        if(div > 64){
            // divide it by 2
            pll_cfg |= RCC_CFG_PLLHSEPSC;
            div >>= 1;
            fin <<= 1;
        }

        if(div < 2){
            // Invalid div
            return 0;
        }

        if(fin < 5000){
            fref = 0;
        }else if(fin < 6250){
            fref = 1;
        }else if(fin < 8330){
            fref = 2;
        }else if(fin < 12500){
            fref = 3;
        }else if(fin < 20830){
            fref = 4;
        }else {
            fref = 5;
        }

        fout = fin * div;

        // Adjust PLLMULT, if div > 16 -1 : -2
        div -= (div > 16) ? 1 : 2;

        pll_cfg |= (div & 0x0F) << 18; // set bits 21:18
        pll_cfg |= (div & 0x30) << 25; // set bits 30:29

        RCC->CFG = pll_cfg;

        RCC->PLL = (RCC->PLL & ~RCC_PLL_PLLFREF) | (fref << 24);

        return fout * 1000;
    }

    // Use flexible configuration
    // Seams able to work at 250MHz with 12MHz HSE

    for (uint32_t ms = 1; ms <= 64; ms++)
    {
        fref = fin / ms;

        // 2 MHz <= FIN/MS <= 16 MHz
        if (fref < 2000U || fref > 16000U)
            continue;
        // pdiv = [1,2,4,8,16,32]
        for (uint8_t pdiv = 1; pdiv < 64; pdiv <<= 1)
        {
            uint32_t numerator   = (uint32_t)fout * ms * pdiv;

            // Require exact integer multiplier
            if ((numerator % fin) != 0)
                continue;

            uint32_t ns = (uint32_t)(numerator / fin);

            if (ns == 0)
                continue;

            uint32_t vco = ((uint32_t)fin * ns) / ms;

            // 500 MHz <= VCO <= 1000 MHz
            if (vco < 500000ULL || vco > 1000000ULL)
                continue;

            vco = vco * 1000 / pdiv;

            uint8_t fr = 0;

            while(pdiv != (1 << fr)){
                fr++;
            }

            pll_cfg = RCC_PLL_PLLCFGEN | (ns << PLL_NS_POS) | (ms << PLL_MS_POS) | (fr << PLL_FR_POS);

            RCC->PLL = pll_cfg;

            return vco;
        }
    }

    return 0;
}

/**
 * @brief   Configure PLL to generate an given output frequency
 *          Note: System clock source cannot be PLL when calling this
 *          function
 *
 * @param pll_src   PLL source clock
 *                  RCC_CFG_PLLRC_HSE
 *                  RCC_CFG_PLLRC_HSI_DIV2
 *
 * @param fosc      Input frequency
 * @param pllclk    Desired output clock
 *
 * @return          Real pllclk, 0 if fail
 */
uint32_t SystemConfigPll(uint32_t pll_src, uint32_t fosc, uint32_t pllclk)
{
    __IO uint32_t StartUpCounter = 0, HSIStatus = 0;

    // Check if system is currenly using pll
    if(((RCC->CTRL & RCC_CFG_SYSCLKSTS) >> 2) == 2){
        return 0;
    }

    // Disable pll
    RCC->CTRL &= ~RCC_CTRL_PLLEN;

    pll_src &= RCC_CFG_PLLRC_HSE;

    if(pll_src){
        // HEXT
        RCC->CTRL |= ((uint32_t)RCC_CTRL_HSEEN);

        do{
            HSIStatus = RCC->CTRL & RCC_CTRL_HSESTBL;
            StartUpCounter++;
        } while ((HSIStatus == 0) && (StartUpCounter != 0xFFFF));

        if ((RCC->CTRL & RCC_CTRL_HSESTBL) == RESET) {
            // Fail to get stable
            return 0;
        }
    }else{
        // HICK
        RCC->CTRL |= ((uint32_t)RCC_CTRL_HSIEN);

        do{
            HSIStatus = RCC->CTRL & RCC_CTRL_HSISTBL;
            StartUpCounter++;
        } while ((HSIStatus == 0) && (StartUpCounter != 0xFFFF));

        if ((RCC->CTRL & RCC_CTRL_HSISTBL) == RESET) {
            // Fail to get stable
            return 0;
        }
    }

    // Set pll clock src
    RCC->CFG = (RCC->CFG & ~RCC_CFG_PLLRC) | pll_src;

    fpll = configurePll(fosc, pllclk);

    if(fpll){
        // Enable pll
        StartUpCounter = 0;
        RCC->CTRL |= RCC_CTRL_PLLEN;

        do{
            HSIStatus = RCC->CTRL & RCC_CTRL_PLLSTBL;
            StartUpCounter++;
        } while ((HSIStatus == 0) && (StartUpCounter != 0xFFFF));
    }

    return fpll;
}

/**
 * @brief
 * @param src   Switch system clock
 *              RCC_CFG_SYSCLKSEL_PLL
 *              RCC_CFG_SYSCLKSEL_HSE
 *              RCC_CFG_SYSCLKSEL_HSI
 *
 * @return      0 on timeout, 1 otherwise
 */
uint32_t SystemConfigClockSrc(uint8_t src)
{
    __IO uint32_t StartUpCounter = 0;

    src &= 3;

    uint32_t sclksel = (RCC->CFG & RCC_CFG_SYSCLKSTS) >> 2;
    /* Check if we are already running on source */
    if (sclksel == src){
        return src;
    }

    #if defined(AT32F413xx) || defined(AT32F403Axx) || \
        defined(AT32F407xx) || defined(AT32F415xx)
    RCC->MISC2 |= RCC_AUTO_STEP_EN;
    #endif

    #if defined(AT32F415xx)
    if(src == RCC_CFG_SYSCLKSEL_PLL){
        /* Configure flash wait cycles */
        uint32_t fls_acr = FLASH->ACR & ~(FLASH_ACR_LATENCY);

        if(fpll <= 32000000UL){
            fls_acr |= FLASH_ACR_LATENCY_0;
        }else if(fpll <= 64000000UL){
            fls_acr |= FLASH_ACR_LATENCY_1;
        }else if(fpll <= 96000000UL){
            fls_acr |= FLASH_ACR_LATENCY_2;
        }else if(fpll <= 128000000UL){
            fls_acr |= FLASH_ACR_LATENCY_3;
        }else {
            fls_acr |= FLASH_ACR_LATENCY_4;
        }

        FLASH->ACR = fls_acr | FLASH_ACR_PRFTBE;
        /* Configure system clock dividers */
        if(fpll > 75000000UL){
            RCC->CFG &= ~(RCC_CFG_AHBPSC | RCC_CFG_APB2PSC | RCC_CFG_APB1PSC | RCC_CFG_ADCPSC);
            RCC->CFG |= (RCC_CFG_AHBPSC_DIV1 | RCC_CFG_APB2PSC_DIV2 | RCC_CFG_APB1PSC_DIV2 | RCC_CFG_ADCPSC_DIV6);
        }
    }
    #endif

    /* Set system clock source */
    RCC->CFG = (RCC->CFG & ~(RCC_CFG_SYSCLKSEL)) | src;

    do{
        sclksel = (RCC->CFG & RCC_CFG_SYSCLKSTS) >> 2;
        if(sclksel == src){
            break;
        }
    } while ((++StartUpCounter) != 0xFFFF);

    #ifdef AT32F403xx
    WaitHseStbl(PLL_STABLE_DELAY);
    #endif
    #if defined(AT32F413xx) || defined(AT32F403Axx) || \
        defined(AT32F407xx) || defined(AT32F415xx)
    RCC->MISC2 &= ~RCC_AUTO_STEP_EN;
    #endif

    return !!(sclksel == src);
}