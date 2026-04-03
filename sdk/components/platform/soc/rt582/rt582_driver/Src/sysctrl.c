/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            sysctrl.c
 * \brief           system control driver file
 */
/*
 * Author:     
 */

#include "mcu.h"


/**
* \brief delay us
*/
void delay_us(volatile uint32_t us)
{
    volatile uint32_t  Delay = 0,sys_clk;
	volatile uint32_t clk_mode = get_ahb_system_clk();
	
	sys_clk = (clk_mode+2)<<4;
	
    Delay = (us * sys_clk )/10;

    do
    {
        __NOP();
        __NOP();
        __NOP();
        __NOP();

    } while (Delay--);
}
/**
* \brief delay ms
*/
void delay_ms(volatile uint32_t ms)
{
    while (ms--)
    {
       delay_us(1000);
    }
}

uint32_t pin_get_mode(uint32_t pin_number) {
    uint32_t reg, base, mask_offset;

    if (pin_number >= 32) {
        return 0xF; /* Invalid setting mode */
    }

    base = MAP_BASE + (pin_number >> 3) * 4;
    mask_offset = (pin_number & 0x7) << 2;

    reg = *((volatile unsigned int*)base);
    reg = reg >> mask_offset;
    reg = reg & 0x7;

    return reg;
}

void pin_set_mode(uint32_t pin_number, uint32_t mode) {
    uint32_t reg, base, mask_offset, mask;

    if ((pin_number >= 32) || (mode > 7)) {
        return; /* Invalid setting mode */
    }

    base = MAP_BASE + (pin_number >> 3) * 4;
    mask_offset = (pin_number & 0x7) << 2;
    mask = 0xF << mask_offset;

    /* pin mux setting is share resource */
    reg = *((volatile unsigned int*)base);
    reg = reg & ~mask;
    reg = reg | (mode << mask_offset);

    *((volatile unsigned int*)base) = reg;

    return;
}

void enable_perclk(uint32_t clock) {
    if ((clock < UART0_CLK) || (clock > I2S_CLK)) {
        return; /* Invalid setting mode */
    }

    SYSCTRL->sys_clk_ctrl |= (1 << clock);
}

void disable_perclk(uint32_t clock) {
    if ((clock < UART0_CLK) || (clock > I2S_CLK)) {
        return; /* Invalid setting mode */
    }

    SYSCTRL->sys_clk_ctrl &= ~(1 << clock);
}

void pin_set_pullopt(uint32_t pin_number, uint32_t mode) {
    uint32_t reg, base, mask_offset, mask;

    if ((pin_number >= 32) || (mode > 7)) {
        return; /* Invalid setting mode */
    }

    base = PULLOPT_BASE + (pin_number >> 3) * 4;
    mask_offset = (pin_number & 0x7) << 2;
    mask = 0xF << mask_offset;

    /* pin mux setting is share resource */
    reg = *((volatile unsigned int*)base);
    reg = reg & ~mask;
    reg = reg | (mode << mask_offset);

    *((volatile unsigned int*)base) = reg;

    return;
}

void pin_set_drvopt(uint32_t pin_number, uint32_t mode) {
    uint32_t reg, base, mask_offset, mask;

    if ((pin_number >= 32) || (mode > 3)) {
        return; /* Invalid setting mode */
    }

    base = DRV_BASE + (pin_number >> 4) * 4;
    mask_offset = (pin_number & 0xF) << 1;
    mask = 0x3 << mask_offset;

    /* pin mux setting is share resource */
    reg = *((volatile unsigned int*)base);
    reg = reg & ~mask;
    reg = reg | (mode << mask_offset);

    *((volatile unsigned int*)base) = reg;

    return;
}

void enable_pin_opendrain(uint32_t pin_number) {
    uint32_t base, mask, reg;

    if (pin_number >= 32) {
        return; /* Invalid setting mode */
    }

    base = OD_BASE;
    mask = 1 << pin_number;

    /* pin mux setting is share resource */
    reg = *((volatile unsigned int*)base);
    reg = reg | mask;
    *((volatile unsigned int*)base) = reg;

    return;
}

void disable_pin_opendrain(uint32_t pin_number) {
    uint32_t base, mask, reg;

    if (pin_number >= 32) {
        return; /* Invalid setting mode */
    }

    base = OD_BASE;
    mask = ~(1 << pin_number);

    /* pin mux setting is share resource */
    reg = *((volatile unsigned int*)base);
    reg = reg & mask;
    *((volatile unsigned int*)base) = reg;

    return;
}

sys_clk_sel_t pll_unlock_check(void)
{
    sys_clk_sel_t sys_clk_mode;

    sys_clk_mode = get_ahb_system_clk();                                                                /*get mcu system clock mode.*/

    if ((sys_clk_mode == SYS_CLK_48MHZ) || (sys_clk_mode == SYS_CLK_64MHZ))   /*PLL 48Mhz/64Mhz unlock check*/
    {

        if (  ((PLL_VIBIT_STATUS() == PLL_LOCK_VIBIT_0) && (PLL_BANK_VCO_STATUS() == PLL_LOCK_BANK_VCO_4)) ||
                ((PLL_VIBIT_STATUS() == PLL_LOCK_VIBIT_3) && (PLL_BANK_VCO_STATUS() == PLL_LOCK_BANK_VCO_7))
           )
        {
            enter_critical_section();

            change_ahb_system_clk(SYS_32MHZ_CLK);

            leave_critical_section();

            sys_clk_mode = get_ahb_system_clk();
        }
        else  if (((PLL_VIBIT_STATUS() == PLL_LOCK_VIBIT_0)) || ((PLL_VIBIT_STATUS() == PLL_LOCK_VIBIT_3)))
        {
            enter_critical_section();

            change_ahb_system_clk(sys_clk_mode);

            leave_critical_section();

            sys_clk_mode = get_ahb_system_clk();
        }
        else
        {
            /*PLL Stable*/
        }
    }

    return sys_clk_mode;
}

uint32_t pll_status_check(void)
{

    if (  ((PLL_VIBIT_STATUS() == PLL_LOCK_VIBIT_0) && (PLL_BANK_VCO_STATUS() == PLL_LOCK_BANK_VCO_4)) ||
            ((PLL_VIBIT_STATUS() == PLL_LOCK_VIBIT_3) && (PLL_BANK_VCO_STATUS() == PLL_LOCK_BANK_VCO_7))
       )
    {
        return STATUS_INVALID_REQUEST;
    }
    else  if (((PLL_VIBIT_STATUS() == PLL_LOCK_VIBIT_0) && (PLL_BANK_VCO_STATUS() != PLL_LOCK_BANK_VCO_4)) ||
              ((PLL_VIBIT_STATUS() == PLL_LOCK_VIBIT_3) && (PLL_BANK_VCO_STATUS() != PLL_LOCK_BANK_VCO_7))
             )
    {
        return STATUS_EBUSY;
    }
    else  if (PLL_LOCK_STATUS() == PLL_LOCK_DOWN)
    {
        return STATUS_SUCCESS;
    }

    return STATUS_INVALID_REQUEST;
}

uint32_t change_ahb_system_clk(sys_clk_sel_t sys_clk_mode)
{
    volatile uint32_t i, j = 0, bank_vco = 0;
    volatile uint32_t clk_status = STATUS_SUCCESS;

    if (sys_clk_mode > SYS_CLK_64MHZ)    /*Invalid parameter*/
    {
        return STATUS_INVALID_PARAM;
    }

    SYSCTRL->sys_clk_ctrl = (SYSCTRL->sys_clk_ctrl & ~HCLK_SEL_MASK) | HCLK_SEL_32M;            //set pll to 32Mhz
    SYSCTRL->sys_clk_ctrl = (SYSCTRL->sys_clk_ctrl & ~BASEBAND_PLL_MASK);                       //baseband pll disable

    if (sys_clk_mode == SYS_CLK_32MHZ)    /*clock mode is 32Mhz*/
    {
    }
    else if (sys_clk_mode == SYS_CLK_48MHZ)        /*Set PLL to 48MHz*/
    {
        PMU->pmu_bbpll.bit.bbpll_ini_bank = 0;
        PMU->pmu_bbpll.bit.bbpll_manubank = 0;
        SYSCTRL->sys_clk_ctrl = (SYSCTRL->sys_clk_ctrl & ~BASEBAND_PLL_FREQ_MASK) | BASEBAND_PLL_48M;


#if (CHIP_VERSION == RT58X_MPB)
        PMU->pmu_bbpll.bit.bbpll_bank1_man =  0; /*change BBPLL setting 2022/03/25 for MPB */
#endif

        for (j = 0; j < PLL_CHECK_COUNT; j++)
        {

            SYSCTRL->sys_clk_ctrl = (SYSCTRL->sys_clk_ctrl | BASEBAND_PLL_ENABLE);
            /*
            * need delay 200 us.
            * If code is not in cache, it will take more time for preload data.
            */
            for (i = 0; i < PLL_WAIT_PERIOD; i++)
            {
                __NOP();
            }

            if (PLL_LOCK_STATUS() == PLL_LOCK_DOWN)            //PLL lock down
            {
                SYSCTRL->sys_clk_ctrl = (SYSCTRL->sys_clk_ctrl & ~HCLK_SEL_MASK) | HCLK_SEL_BASEBAND_PLL;      /*config BASEBAND_PLL_ENABLE*/

                break;
            }
            else if ((PLL_VIBIT_STATUS() == PLL_LOCK_VIBIT_3) && (PLL_BANK_VCO_STATUS() < PLL_LOCK_BANK_VCO_7))
            {
                bank_vco = PLL_BANK_VCO_STATUS();
                PMU->pmu_bbpll.bit.bbpll_ini_bank = (bank_vco + 1); //next bank
                PMU->pmu_bbpll.bit.bbpll_manubank = 1;
            }
            else
            {
                SYSCTRL->sys_clk_ctrl = (SYSCTRL->sys_clk_ctrl & ~BASEBAND_PLL_MASK);                          /*baseband pll disable*/

                //delay > 32us
                for (i = 0; i < PLL_DELAY_PERIOD; i++)
                {
                    __NOP();
                }

                clk_status = pll_status_check();

                if (clk_status == STATUS_INVALID_REQUEST)
                {
                    break;
                }
            }
        }
    }
    else if (sys_clk_mode == SYS_CLK_64MHZ)        /*Set PLL to 64MHz*/
    {
        PMU->pmu_bbpll.bit.bbpll_ini_bank = 0;
        PMU->pmu_bbpll.bit.bbpll_manubank = 0;
        SYSCTRL->sys_clk_ctrl = (SYSCTRL->sys_clk_ctrl & ~BASEBAND_PLL_FREQ_MASK) | BASEBAND_PLL_64M;

#if (CHIP_VERSION == RT58X_MPB)
        PMU->pmu_bbpll.bit.bbpll_bank1_man =  1; /*change BBPLL setting 2022/03/25 for MPB */
#endif

        for (j = 0; j < PLL_CHECK_COUNT; j++)
        {
            SYSCTRL->sys_clk_ctrl = (SYSCTRL->sys_clk_ctrl | BASEBAND_PLL_ENABLE);      /*baseband pll enable*/

            /*
            * need delay 200 us.
            * If code is not in cache, it will take more time for preload data.
            */
            for (i = 0; i < PLL_WAIT_PERIOD; i++)
            {
                __NOP();
            }

            if (PLL_LOCK_STATUS() == PLL_LOCK_DOWN)            //PLL lock down
            {
                SYSCTRL->sys_clk_ctrl = (SYSCTRL->sys_clk_ctrl & ~HCLK_SEL_MASK) | HCLK_SEL_BASEBAND_PLL;        /*config BASEBAND_PLL_ENABLE*/

                break;
            }
            else if ((PLL_VIBIT_STATUS() == PLL_LOCK_VIBIT_3) && (PLL_BANK_VCO_STATUS() < PLL_LOCK_BANK_VCO_7))
            {
                bank_vco = PLL_BANK_VCO_STATUS();
                PMU->pmu_bbpll.bit.bbpll_ini_bank = (bank_vco + 1); //next bank
                PMU->pmu_bbpll.bit.bbpll_manubank = 1;
            }
            else
            {
                SYSCTRL->sys_clk_ctrl = (SYSCTRL->sys_clk_ctrl & ~BASEBAND_PLL_MASK);                            //baseband pll disable

                //delay > 32us
                for (i = 0; i < PLL_DELAY_PERIOD; i++)
                {
                    __NOP();
                }

                clk_status = pll_status_check();

                if (clk_status == STATUS_INVALID_REQUEST)
                {
                    break;
                }
            }
        }
    }

    //Add pll status check lock success, config to pll clock
    if ((j == PLL_CHECK_COUNT) && (clk_status == STATUS_SUCCESS))
    {
        SYSCTRL->sys_clk_ctrl = (SYSCTRL->sys_clk_ctrl | BASEBAND_PLL_ENABLE);                           /*baseband pll enable*/

        for (i = 0; i < PLL_WAIT_PERIOD; i++)
        {
            __NOP();
        }

        SYSCTRL->sys_clk_ctrl = (SYSCTRL->sys_clk_ctrl & ~HCLK_SEL_MASK) | HCLK_SEL_BASEBAND_PLL;        /*config BASEBAND_PLL_ENABLE*/
    }

    flash_timing_init();

    systemfrequencyupdate();

    //Delay_Init();

    return clk_status;
}



sys_clk_sel_t get_ahb_system_clk(void) {
    sys_clk_sel_t clk_mode = SYS_32MHZ_CLK;

    if ((SYSCTRL->sys_clk_ctrl & HCLK_SEL_MASK) == HCLK_SEL_32M) {
        clk_mode = SYS_32MHZ_CLK;
    } else if ((SYSCTRL->sys_clk_ctrl & HCLK_SEL_MASK)
               == HCLK_SEL_BASEBAND_PLL) {
        if ((SYSCTRL->sys_clk_ctrl & BASEBAND_PLL_MASK)
            != BASEBAND_PLL_ENABLE) {
            ASSERT();
        }
        if ((SYSCTRL->sys_clk_ctrl & BASEBAND_PLL_FREQ_MASK)
            == BASEBAND_PLL_48M) {
            clk_mode = SYS_48MHZ_CLK;
        } else if ((SYSCTRL->sys_clk_ctrl & BASEBAND_PLL_FREQ_MASK)
                   == BASEBAND_PLL_64M) {
            clk_mode = SYS_64MHZ_CLK;
        }
    } else {
        ASSERT();
    }

    return clk_mode;
}



void set_slow_clock_source(uint32_t mode) {
    /*Slow clock selection.*/
    uint32_t temp;

    if (mode > 3) {
        return; /*Invalid mode*/
    }

    temp = SYSCTRL->sys_clk_ctrl & ~(SLOW_CLOCK_SEL_MASK);
    temp |= (mode << SLOW_CLOCK_SEL_SHIFT);
    SYSCTRL->sys_clk_ctrl = temp;
}

void set_ext32k_pin(uint32_t pin_number) {

    if (pin_number > 7) /*GPIO0~GPIO7 can input EXT_32K*/
    {
        return; /*Invalid setting mode.*/
    }

    pin_set_mode(pin_number, MODE_EXT32K);

    return;
}
